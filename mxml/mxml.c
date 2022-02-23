#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "mxml.h"
#include "mxml_int.h"

/* The space needed to hold UINT_MAX in base 10.
 * Used for buffer size calculations.
 * 25/10 approximates 8*ln(2)/ln(10) */
#define UINT_MAX_LEN (sizeof (unsigned int) * 25 / 10 + 1)

/** Tests if the string ends with an ending. */
static int
ends_with(const char *s, const char *end)
{
	int slen = strlen(s);
	int endlen = strlen(end);

	return slen >= endlen &&
		strcmp(s + slen - endlen, end) == 0;
}

/**
 * Copies/calculates unencoded form of XML.
 * @param out (optional) buffer to copy unencoded data to.
 * @returns number of bytes copied into @a out.
 */
static size_t
unencode_xml_into(const struct cursor *init_curs, char *out)
{
	size_t n = 0;
	struct cursor c = *init_curs;

#define OUT(ch) do { if (out) out[n] = (ch); n++; } while (0)

	while (!cursor_is_at_eof(&c)) {
		char ch = *c.pos++;
		if (ch == '&' && !cursor_is_at_eof(&c)) {
			switch (*c.pos) {
			case 'l': OUT('<'); break; /* &lt; */
			case 'g': OUT('>'); break; /* &gt; */
			case 'a': OUT('&'); break; /* &amp; */
			}
			cursor_skip_to_ch(&c, ';');
			if (!cursor_is_at_eof(&c))
				c.pos++;
		} else if (ch == '<') {
			break;
		} else {
			OUT(ch);
		}
	}
	return n;
#undef OUT
}

/**
 * Unencodes XML into a new string.
 * Expands the XML entities, (&lt; &gt; &amp;)
 * @returns NUL-terminated string allocated by #malloc.
 * @retval NULL [ENOMEM] could not allocate memory.
 */
static char *
unencode_xml(const char *content, size_t contentsz)
{
	size_t retsz = 0;
	struct cursor c;
	char *ret;

	c.pos = content;
	c.end = content + contentsz;
	retsz = unencode_xml_into(&c, NULL);
	ret = malloc(retsz + 1);
	if (ret) {
		unencode_xml_into(&c, ret);
		ret[retsz] = '\0';
	}
	return ret;
}


/* Save a little bit of memory with a global empty value string */
static char value_empty[] = "";

static char *
value_strdup(const char *s)
{
	if (!s || !*s)
		return value_empty;
	return strdup(s);
}

static void
value_free(char *value)
{
	if (value != value_empty)
		free(value);
}


struct mxml *
mxml_new(const char *start, unsigned int size)
{
	struct mxml *m = malloc(sizeof *m);

	if (!m)
		return NULL;
	m->start = start;
	m->size = size;
	m->edits = NULL;
#if HAVE_CACHE
	cache_init(m);
#endif
	return m;
}

void
mxml_free(struct mxml *m)
{
	struct edit *e;

	if (!m)
		return;
	while ((e = m->edits)) {
		m->edits = e->next;
		free(e->key);
		value_free(e->value);
		free(e);
	}
	free(m);
}

static const char *
find_expand_key(struct mxml *m, const char *key, size_t *size_return)
{
	char ekey[KEY_MAX];
	int ekeylen;

	ekeylen = expand_key(m, ekey, sizeof ekey, key);
	if (ekeylen < 0)
		return NULL;
	return find_key(m, ekey, ekeylen, size_return);
}

char *
mxml_get(struct mxml *m, const char *key)
{
	const char *content;
	size_t contentsz;

	content = find_expand_key(m, key, &contentsz);
	if (!content) {
		/* Provide a missing list total */
		if (errno == ENOENT && ends_with(key, "[#]"))
			return strdup("0");
		return NULL;
	}
	return unencode_xml(content, contentsz);
}

/**
 * Create a new edit record, inserted at the head
 * of the edit list.
 * @retval NULL [ENOMEM] no memory
 */
static struct edit *
edit_new(struct mxml *m, enum edit_op op, const char *ekey, int ekeylen,
	const char *value)
{
	struct edit *e = calloc(1, sizeof *e);
	if (!e)
		return NULL;
	e->key = strndup(ekey, ekeylen);
	if (!e->key) {
		free(e);
		return NULL;
	}
	e->value = value_strdup(value);
	if (!e->value) {
		free(e->key);
		free(e);
		return NULL;
	}
	e->op = op;
	e->next = m->edits;
	m->edits = e;
	return e;
}

int
mxml_delete(struct mxml *m, const char *key)
{
	char ekey[KEY_MAX];
	int ekeylen;
	const char *content;
	size_t contentsz;
	struct edit *edit;

	ekeylen = expand_key(m, ekey, sizeof ekey, key);
	if (ekeylen < 0)
		return 0;

	/* Can't delete [#] */
	if (ends_with(key, "[#]")) {
		errno = EPERM;
		return -1;
	}

	content = find_key(m, ekey, ekeylen, &contentsz);
	if (!content)
		return errno == ENOENT ? 0 : -1;

	edit = edit_new(m, EDIT_DELETE, ekey, ekeylen, NULL);
	if (!edit)
		return -1;

	/* Deleting [$] updates .total */
	if (ends_with(key, "[$]")) {
		/*
		 * The ekey will be "tags.tagNNN".
		 * We'll capture the NNN, decrement it,
		 * then convert ekey to "tags.total".
		 */
		char totalbuf[UINT_MAX_LEN + 1];
		char *total;
		unsigned int i = ekeylen;
		unsigned j = sizeof totalbuf;
		int carry = 1;

		/* In right-to-left loops, indicies
		 * are pre-decremented before read/write.
		 * Subtract the carry from the number, so that
		 * "100" becomes "099".
		 */
		totalbuf[--j] = '\0';
		while (i) {
			char ch = ekey[--i];
			if (!isdigit(ch))
				break;
			if (carry) {
				if (ch == '0') ch = '9';
				else ch--, carry = 0;
			}
			totalbuf[--j] = ch;
		}
		if (carry) /* something went bad; assume 0 */
			j = sizeof totalbuf - 1;
		/* "09" become "9"; and "0" becomes "" */
		if (totalbuf[j] == '0')
			j++;
		/* Convert "" to NULL (delete) */
		total = totalbuf[j] ? &totalbuf[j] : NULL;

		/* Back up to after the '.' in "tags.tagNNN" */
		while (i && ekey[i - 1] != '.')
			i--;
		/* Replace the ending to make "tags.total" */
		if (i + 5 > sizeof ekey) {
			errno = ENOMEM;
			return -1;
		}
		memcpy(ekey + i, "total", 5);
		ekeylen = i + 5;

		content = find_key(m, ekey, ekeylen, &contentsz);
		if (content && !total) {
			/* Instead of setting .total to 0,
			 * we delete it. */
			edit = edit_new(m, EDIT_DELETE,
					ekey, ekeylen, NULL);
			if (!edit)
				return -1;
		} else if (content && total) {
			edit = edit_new(m, EDIT_SET,
					ekey, ekeylen, total);
			if (!edit)
				return -1;
		} else if (!content && total) {
			/* XXX this should never happen */
			edit = edit_new(m, EDIT_APPEND,
					ekey, ekeylen, total);
			if (!edit)
				return -1;
		}
	}

	return 0;
}

int
mxml_update(struct mxml *m, const char *key, const char *value)
{
	char ekey[KEY_MAX];
	int ekeylen;
	const char *content;
	size_t contentsz;
	struct edit *edit;

	ekeylen = expand_key(m, ekey, sizeof ekey, key);
	if (ekeylen < 0)
		return 0;

	/* Can't set [#] */
	if (ends_with(key, "[#]")) {
		errno = EPERM;
		return -1;
	}

	content = find_key(m, ekey, ekeylen, &contentsz);
	if (!content)
		return -1;	/* Must exist to be set */
	edit = edit_new(m, EDIT_SET, ekey, ekeylen, value);
	if (!edit)
		return -1;
	return 0;
}

int
mxml_exists(struct mxml *m, const char *key)
{
	size_t contentsz;

	return find_expand_key(m, key, &contentsz) != NULL;
}

int
mxml_append(struct mxml *m, const char *key, const char *value)
{
	char ekey[KEY_MAX];
	int ekeylen;
	int sublen;
	const char *dot;
	const char *brackplus;
	const char *content;
	size_t contentsz;
	struct edit *edit;

	ekeylen = expand_key(m, ekey, sizeof ekey, key);
	if (ekeylen < 0)
		return 0;

	/* Can't append if it already exists */
	content = find_key(m, ekey, ekeylen, &contentsz);
	if (content) {
		errno = EEXIST;
		return -1;
	}

	/* Append all missing parents first. */
	sublen = 0;
	while ((dot = memchr(ekey + sublen, '.', ekeylen - sublen))) {
		sublen = dot - ekey;
		content = find_key(m, ekey, sublen, &contentsz);
		if (!content) {
			edit = edit_new(m, EDIT_APPEND, ekey, sublen, NULL);
			if (!edit)
				return -1;
		}
		sublen++;
	}

	/* Update tags.total when the key contains "tag[+]" */
	if ((brackplus = strstr(key, "[+]"))) {
		char *tkey;		/* "tag[#]" */
		char etkey[KEY_MAX];	/* "tags.total" */
		int etkeylen;
		int bracklen = brackplus - key + 3;
		const char *tcontent;
		size_t tcontentsz;
		unsigned int total;
		char newtotal[UINT_MAX_LEN + 1];

		/* Make a copy of "tag[+]" */
		tkey = strndup(key, bracklen);
		if (!tkey)
			return -1;
		/* Replace the trailing [+] with [#] */
		tkey[bracklen - 2] = '#';

		/* Expand tag[#] into tags.total */
		etkeylen = expand_key(m, etkey, sizeof etkey, tkey);
		if (etkeylen < 0)
			return 0;

		/* Find the existing total, if any */
		tcontent = find_key(m, etkey, etkeylen, &tcontentsz);
		if (!tcontent || parse_uint(tcontent, tcontentsz, &total) < 0)
			total = 0;

		/* Add one, and make an edit element to update it */
		snprintf(newtotal, sizeof newtotal, "%u", total + 1);
		edit = edit_new(m, tcontent ? EDIT_SET : EDIT_APPEND,
		    etkey, etkeylen, newtotal);

		free(tkey);
	}

	edit = edit_new(m, EDIT_APPEND, ekey, ekeylen, value);
	if (!edit)
		return -1;
	return 0;
}

int
mxml_set(struct mxml *m, const char *key, const char *value)
{
	int ret;

	if (!value) {
		ret = mxml_delete(m, key);
	} else {
		ret = mxml_append(m, key, value);
		if (ret == -1 && errno == EEXIST)
			ret = mxml_update(m, key, value);
	}
	return ret;
}


char *
mxml_expand_key(struct mxml *m, const char *key)
{
	char outbuf[KEY_MAX];
	unsigned int outlen = 0;
	const char *brack;
	const char *first;
	int n;

	first = key;
	while ((brack = strstr(first, "[$]"))) {
		const char *content;
		size_t contentsz;
		unsigned int total;
		const char *endbrack = brack + 3;
		int tkeylen = endbrack - key;
		char *tkey = strndup(key, tkeylen);

		if (!tkey)
			return NULL;
		tkey[tkeylen - 2] = '#';
		content = find_expand_key(m, tkey, &contentsz);
		if (!content || parse_uint(content, contentsz, &total) < 0)
			total = 0;
		free(tkey);

		n = snprintf(&outbuf[outlen], sizeof outbuf - outlen,
			"%.*s[%u]", (int)(brack - first), first, total);
		outlen += n;
		if (outlen >= sizeof outbuf) {
			errno = ENOMEM;
			return NULL;
		}
		first = endbrack;
	}
	n = snprintf(&outbuf[outlen], sizeof outbuf - outlen, "%s", first);
	outlen += n;
	if (outlen >= sizeof outbuf) {
		errno = ENOMEM;
		return NULL;
	}

	return strndup(outbuf, outlen);
}
