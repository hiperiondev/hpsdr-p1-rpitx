#define _GNU_SOURCE /* memrchr */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "mxml.h"
#include "mxml_int.h"

/* #define DEBUG 1 */

/*
 * Flatten the edit list into the XML document. The result is a token
 * stream that represents the combined edits and XML.
 *
 * This non-recursive implementation uses a token carrier model that moves up
 * and down the edit list.
 * An 'empty' token carrier moves upwards (towards the source XML), and when
 * 'occupied' the carrier moves downwards (towards the writefn).
 * At the source XML (top end), the empty carrier is filled with the next
 * scanned OPEN, VALUE or CLOSE token, from the source XML and sent down.
 * At the writefn end, the token is removed from the carrier and emitted
 * through the writefn, and the empty carrier sent up.
 * A token holds its full tag path context and can be inspected at every level.
 * the OPEN, VALUE and CLOSE tokens of the same element have the same context
 * string, namely the dotted tag.
 * Each edit list entry may manipulate the carrier as it passes (in either
 * direction) and may manipulate its own state.
 *
 * The general operation of edit entries is as follows:
 *   DELETE: If a descending token matches the path, remove it.
 *   SET:    After a descending OPEN passes, insert the new VALUE, and
 *           drop any other matching VALUES.
 *   APPEND: If a descending CLOSE token matches the parent, enter state 1,
 *           holding the CLOSE token aside, and send down the new OPEN token in
 *           the carrier.
 *           In the ascending case, when in state 1,2,3 send down the new
 *           VALUE, new CLOSE then finally the original, held CLOSE, each time
 *           advancing the state.
 */

/* An edit entry state union.
 * Tokens pass through each edit entry, and it may updte its state. */
struct editstate {
	enum {
		EDIT_KIND_XML,
		EDIT_KIND_DELETE,
		EDIT_KIND_SET,
		EDIT_KIND_APPEND,
		EDIT_KIND_WRITE
	} kind;
	const struct edit *edit;

	union {
		struct appendstate {
			const char *key;
			unsigned int keylen;
			unsigned int parentlen;
			const char *value;
			int valuelen;
			enum {
				APPEND_IDLE,
				APPEND_SENT_OPEN,
				APPEND_SENT_VALUE,
				APPEND_SENT_CLOSE
			} state;
			struct token *held_token;
			struct token sent_token;
			char tagdata[128];
		} append;
		struct xmlstate {
			struct cursor cursor;
			struct token token;
			char key[KEY_MAX];
			int keylen;
			int init;
		} xml;
		struct writestate {
			int (*fn)(void *context, const struct token *token);
			void *context;
		} write;
		struct setstate {
			int just_opened;
			struct token token;
		} set;
		struct {
			const char *key;
			unsigned int keylen;
		} del;
	};
};

/** Returns the last tag part of a key.
 *  For example, the last tag in "a.b.c" is "c" */
static const char *
last_tag(const char *key)
{
	const char *dot = memrchr(key, '.', strlen(key));
	return dot ? dot + 1 : key;
}

/** Returns the length of the parent part of a key.
 *  For example, key "a.b.c" has parent "a.b" */
static unsigned int
parent_len(const char *key, unsigned int keylen)
{
	const char *dot = memrchr(key, '.', keylen);
	return dot ? dot - key : 0;
}

/**
 * Creates an edit state array from match m->edits.
 * @param n_return storage for the length of the array returned.
 * @returns new array; caller should #free() it eventually.
 */
static struct editstate *
make_editstates(const struct mxml *m, unsigned int *n_return)
{
	unsigned int n = 0;
	const struct edit *edit;
	struct editstate *states, *es;

	n = 2;
	for (edit = m->edits; edit; edit = edit->next)
		n++;
	states = calloc(n, sizeof *states);
	if (!states)
		goto nomem;

	/* The bottom of the state list is the output writer */
	es = states;
	es->kind = EDIT_KIND_WRITE;
	es++;

	for (edit = m->edits; edit; edit = edit->next, es++) {
		es->edit = edit;
		switch (edit->op) {
		case EDIT_DELETE:
			es->kind = EDIT_KIND_DELETE;
			es->del.key = edit->key;
			es->del.keylen = strlen(edit->key);
			break;
		case EDIT_SET:
			es->kind = EDIT_KIND_SET;
			es->set.token.type = TOK_VALUE;
			strncpy(es->set.token.key, edit->key,
				sizeof es->set.token.key);
			es->set.token.keylen = strlen(edit->key);
			es->set.token.value = edit->value;
			es->set.token.valuelen = -1;
			break;
		case EDIT_APPEND:
			es->kind = EDIT_KIND_APPEND;
			es->append.state = APPEND_IDLE;
			es->append.key = edit->key;
			es->append.keylen = strlen(edit->key);
			es->append.parentlen = parent_len(es->append.key,
			    es->append.keylen);
			es->append.value = edit->value;
			es->append.valuelen = -1;
			if (snprintf(es->append.tagdata,
			    sizeof es->append.tagdata, "</%s>",
			    last_tag(edit->key)) >= sizeof es->append.tagdata)
				goto nomem;
			break;
		}
	}

	/* The top of the state list is the XML token source */
	es->kind = EDIT_KIND_XML;
	es->xml.cursor.pos = m->start;
	es->xml.cursor.end = m->start + m->size;
	es++;

	*n_return = n;
	return states;
nomem:
	free(states);
	errno = ENOMEM;
	return NULL;
}

/**
 * Generates the next token from the XML source.
 * This function advances x->cursor, to fille in x->token.
 * It will generate either an OPEN, CLOSE, VALUE (text) or EOF token.
 * @param x pointer to private state
 * @retval -1 on error (too deeply nested)
 */
static int
xml_tokenize(struct xmlstate *x)
{
	const char *tag;
	int taglen;
	struct cursor * const c = &x->cursor;
	struct token *token = &x->token;

	token->value = c->pos;

	if (cursor_is_at_eof(c))
		goto eof;
	if (*c->pos != '<' || !x->init) {
		token->type = TOK_VALUE;
		cursor_skip_content(c);
		token->valuelen = c->pos - token->value;
		memcpy(token->key, x->key, x->keylen);
		token->keylen = x->keylen;
		x->init = 1;
		return 0;
	}

	c->pos++; /* skip < */
	if (cursor_is_at_eof(c))
		goto eof;
	if (*c->pos == '/') {
		c->pos++;
		token->type = TOK_CLOSE;
	} else
		token->type = TOK_OPEN;

	tag = c->pos;
	while (!cursor_is_at_eof(c) && *c->pos != '>' && !isspace(*c->pos))
		c->pos++;
	taglen = c->pos - tag;
	cursor_skip_to_ch(c, '>'); /* TODO attributes */
	cursor_eatch(c, '>');
	token->valuelen = c->pos - token->value;

	if (token->type == TOK_OPEN) {
		/* Append .tag to x->key */
		if (x->keylen + 1 + taglen > sizeof x->key) {
			errno = ENOMEM;
			return -1;
		}
#ifdef DEBUG
		fprintf(stderr, "[append dot]");
#endif
		if (x->keylen)
			x->key[x->keylen++] = '.';
		memcpy(&x->key[x->keylen], tag, taglen);
		x->keylen += taglen;
	}

	memcpy(token->key, x->key, x->keylen);
	token->keylen = x->keylen;

	if (token->type == TOK_CLOSE) {
		/* Remove .tag from end of key */
		const char *dot = memrchr(x->key, '.', x->keylen);
#ifdef DEBUG
		fprintf(stderr, "[strip dot]");
#endif
		if (dot) x->keylen = dot - x->key;
		else x->keylen = 0;
	}

	return 0;
eof:
	token->type = TOK_EOF;
	token->keylen = 0;
	token->value = NULL;
	token->valuelen = 0;
	return 0;
}

#ifdef DEBUG
  /* Colours make debug easier to read */
# define C_KEY  "\033[33m"
# define C_STR  "\033[34m"
# define C_OUT  "\033[32m"
# define C_DIM  "\033[2m"
# define C_END  "\033[m"
#endif

#ifdef DEBUG
/** Prints a string using C escapes, e.g. "\n" */
static void
fputesc(FILE *f, const char *s, int len)
{
	while (len--) {
		char ch = *s++;
		if (ch == '\n') fputs("\\n", f);
		else if (ch == '\r') fputs("\\r", f);
		else if (ch < ' ')
			fprintf(f, "\\o%03o", ch);
		else {
			if (ch == '\\' || ch == '\"')
				fputc('\\', f);
			fputc(ch, f);
		}
	}
}
#endif /* DEBUG */

/**
 * Handle the set state receiving the token carrier.
 * We pass through the OPEN, and inject our own VALUE immediately next.
 * Then we drop all the other VALUEs passed to us.
 */
static int
process_set(struct setstate *set, struct token **carrier)
{
	struct token *token = *carrier;

	if (token &&
	    set->token.keylen == token->keylen &&
	    memcmp(token->key, set->token.key, token->keylen) == 0)
	{
		if (token->type == TOK_OPEN)
			set->just_opened = 1;
		else if (token->type == TOK_VALUE)
			*carrier = NULL; /* drop */
	}
	else if (!token && set->just_opened) {
		/* The carrier has returned after our OPEN went by */
		set->just_opened = 0;
		*carrier = &set->token;
	}

	return 0;
}

/**
 * Handle the append state receiving the token carrier.
 * APPEND is the most complicated process; it has to
 * hold back and insert tokens. This function can advance
 * the app->state and replace the token in the carrier.
 */
static int
process_append(struct appendstate *app, struct token **carrier)
{
	struct token *token = *carrier;

	/* Is this the parental CLOSE token that triggers us? */
	if (token && token->type == TOK_CLOSE &&
	    app->parentlen == token->keylen &&
	    memcmp(app->key, token->key, token->keylen) == 0)
	{
		/* It matches. We hold the </parent> CLOSE token
		 * in our private state; then we send our own
		 * <tag> OPEN token. We'll also set our state
		 * so that the next time we get an empty carrier,
		 * we can continue on below. */
		struct token *newtok = &app->sent_token;
		app->held_token = token;
		app->tagdata[1] = '<';
		newtok->type = TOK_OPEN;
		memcpy(app->sent_token.key, app->key,
		    app->sent_token.keylen = app->keylen);
		newtok->value = &app->tagdata[1];
		newtok->valuelen = strlen(newtok->value);
		*carrier = newtok;
		app->state = APPEND_SENT_OPEN;
		return 0;
	}

	if (token)
		return 0;

	switch (app->state) {
	default:
	case APPEND_IDLE:
		/* Haven't been triggered yet */
		return 0;
	case APPEND_SENT_OPEN:
		if (app->valuelen) {
			*carrier = token = &app->sent_token;
			token->type = TOK_VALUE;
			token->value = app->value;
			token->valuelen = app->valuelen;
			app->state = APPEND_SENT_VALUE;
			return 0;
		} /* else fallthrough */
	case APPEND_SENT_VALUE:
		/* send our </tag> token */
		*carrier = token = &app->sent_token;
		token->type = TOK_CLOSE;
		app->tagdata[0] = '<';
		app->tagdata[1] = '/';
		token->value = app->tagdata;
		token->valuelen = strlen(token->value);
		app->state = APPEND_SENT_CLOSE;
		return 0;
	case APPEND_SENT_CLOSE:
		/* send the held </parent> token */
		*carrier = app->held_token;
		app->state = APPEND_IDLE;
		return 0;
	};
}

static int
process_token(struct editstate *s, struct token **carrier)
{
	struct token *token = *carrier;

	switch (s->kind) {
	case EDIT_KIND_XML:
		/* assert(!token); */
		if (xml_tokenize(&s->xml) == -1)
			return -1;
		*carrier = &s->xml.token;
		return 0;
	case EDIT_KIND_DELETE:
		if (token && s->del.keylen <= token->keylen &&
		    (s->del.keylen == token->keylen ||
		     token->key[s->del.keylen] == '.') &&
		     memcmp(token->key, s->del.key, s->del.keylen) == 0)
			*carrier = NULL;
		return 0;
	case EDIT_KIND_SET:
		return process_set(&s->set, carrier);
	case EDIT_KIND_APPEND:
		return process_append(&s->append, carrier);
	case EDIT_KIND_WRITE:
		if (!token)
			return 0; /* special initial case */
		if (token->type == TOK_EOF)
			return 0; /* special final case: pass EOF */
		*carrier = NULL;
		return s->write.fn(s->write.context, token);
	}
	errno = EIO;
	return -1;
}


/**
 * Flatten the edit list and XML source document into a token stream.
 */
int
flatten_edits(const struct mxml *m,
	      int (*fn)(void *context, const struct token *token),
	      void *context)
{
	int ret = 0;
	struct editstate *states, *curstate;
	unsigned int nstates = 0;
	struct token *token;	/* token carrier */

	/* Compute the edit filter chain */
	states = make_editstates(m, &nstates);
	if (!states)
		return -1;

#ifdef DEBUG
	fprintf(stderr, "\nmxml_write %u states", nstates);
#endif
	/* assert(nstates > 0 && states[0].kind == EDIT_KIND_WRITE); */
	states[0].write.fn = fn;
	states[0].write.context = context;

	/* Main loop that drives tokens up and down the chain */
	curstate = states;
	token = NULL;
	while (curstate) {
		int n;
#ifdef DEBUG
		static int previd = 0;
		int id = (curstate - states);
		fprintf(stderr, "\n%*s" C_DIM "<", id + previd, "");
		if (token) {
			switch (token->type) {
			case TOK_EMPTY: fprintf(stderr, "EMPTY"); break;
			case TOK_EOF: fprintf(stderr, "EOF"); break;
			case TOK_OPEN: fprintf(stderr, "OPEN" C_END " " C_KEY "%.*s" C_END,
				token->keylen, token->key); break;
			case TOK_VALUE: fprintf(stderr, "VALUE" C_END " " C_KEY "%.*s" C_END,
				token->keylen, token->key);
				break;
			case TOK_CLOSE: fprintf(stderr, "CLOSE" C_END " " C_KEY "%.*s" C_END,
				token->keylen, token->key); break;
			default: fprintf(stderr, "???");
			}
			if (token->value) {
				fprintf(stderr, " %s" C_END "\"" C_STR,
				    token->valuelen == -1 ? "(user) " : "");
				fputesc(stderr, token->value, token->valuelen == -1
				    ? strlen(token->value) : token->valuelen);
				fprintf(stderr, C_END "\"");
			}
		}
		previd = id;
		fprintf(stderr, C_DIM ">" C_END "\n%*s{", id + previd, "");
		switch (curstate->kind) {
		case EDIT_KIND_XML:
			fprintf(stderr, "XML"); break;
		case EDIT_KIND_DELETE:
			fprintf(stderr, "DELETE " C_KEY "%.*s" C_END,
			    curstate->del.keylen,
			    curstate->del.key);
			break;
		case EDIT_KIND_SET:
			fprintf(stderr, "SET " C_KEY "%.*s" C_END "=\"" C_STR "%s" C_END "\"",
			    curstate->set.token.keylen,
			    curstate->set.token.key,
			    curstate->set.token.value);
			break;
		case EDIT_KIND_APPEND:
			fprintf(stderr, "APPEND " C_KEY "%.*s" C_END "|" C_KEY "%.*s" C_END "=\"" C_STR "%s" C_END "\" %s",
			    curstate->append.parentlen,
			    curstate->append.key,
			    curstate->append.keylen - curstate->append.parentlen,
			    curstate->append.key + curstate->append.parentlen,
			    curstate->edit->value,
			    curstate->append.state == APPEND_IDLE ? "IDLE" :
			    curstate->append.state == APPEND_SENT_OPEN ? "SENT_OPEN" :
			    curstate->append.state == APPEND_SENT_VALUE ? "SENT_VALUE" :
			    curstate->append.state == APPEND_SENT_CLOSE ? "SENT_CLOSE" :
			    "?");
			break;
		case EDIT_KIND_WRITE:
			fprintf(stderr, "WRITE"); break;
		default:
			fprintf(stderr, "???");
		}
		fprintf(stderr, "}");
#endif

		if (curstate == states && token && token->type == TOK_EOF)
			break;

		/* Process the token carrier with the current edit entry,
		 * which may involve output, which we accumulate in ret. */
		n = process_token(curstate, &token);
		if (n == -1)
			return -1;
		ret += n;

		/* If the carrier is empty, it floats up to the XML source;
		 * If it is full it floats down to the writer end. */
		if (!token)
			curstate++;
		else if (curstate != states)
			curstate--;
		else
			curstate = NULL; /* Fell off the bottom */
	}
	free(states);
#ifdef DEBUG
	fprintf(stderr, " EOF: return %d\n", ret);
#endif
	return ret;
}
