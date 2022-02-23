#define _GNU_SOURCE	/* for memmem() */
#include <string.h>
#include <ctype.h>

#include "mxml_int.h"

/** Cursor is at the end of its range? */
int
cursor_is_at_eof(const struct cursor *c)
{
	return c->pos == c->end;
}

/** Cursor is positioned at the string? */
static int
cursor_is_atn(const struct cursor *c, const char *s, size_t slen)
{
	return c->pos + slen <= c->end &&
	       memcmp(c->pos, s, slen) == 0;
}

/** Cursor is positioned at the string? */
int
cursor_is_at(const struct cursor *c, const char *s)
{
	return cursor_is_atn(c, s, strlen(s));
}

/** Advance cursor past a string only if it is there.
 *  @retval 0 could not advance; cursor left unchanged.
 *  @retval 1 advanced cursor past @a s. */
int
cursor_eatn(struct cursor *c, const char *s, unsigned int slen)
{
	if (c->pos + slen > c->end)
		return 0;
	if (memcmp(c->pos, s, slen) != 0)
		return 0;
	c->pos += slen;
	return 1;
}

/** Advance cursor past a character only if it is there. */
int
cursor_eatch(struct cursor *c, char ch)
{
	if (!cursor_is_at_eof(c) && *c->pos == ch) {
		++c->pos;
		return 1;
	}
	return 0;
}

/** Advance cursor past a string only if it is there.
 *  @retval 0 could not advance; cursor left unchanged.
 *  @retval 1 advanced cursor past @a s. */
static int
cursor_eat(struct cursor *c, const char *s)
{
	int slen = strlen(s);
	if (cursor_is_atn(c, s, slen)) {
		c->pos += slen;
		return 1;
	} else {
		return 0;
	}
}

/** Advance cursor past any whitespace.
 *  @retval 0 iff no whitespace was found. */
int
cursor_eat_white(struct cursor *c)
{
	if (cursor_is_at_eof(c) || !isspace(*c->pos))
		return 0;
	do {
		c->pos++;
	} while (!(cursor_is_at_eof(c) || !isspace(*c->pos)));
	return 1;
}

/** Advance cursor until it is positioned at @a s, or at EOF */
static void
cursor_skip_to(struct cursor *c, const char *s)
{
	const char *found;

	found = memmem(c->pos, c->end - c->pos, s, strlen(s));
	c->pos = found ? found : c->end;
}

/** Advance cursor until it is positioned at @a ch, or at EOF */
void
cursor_skip_to_ch(struct cursor *c, char ch)
{
	const char *found = memchr(c->pos, ch, c->end - c->pos);
	c->pos = found ? found : c->end;
}

/**
 * Skip over all text, XML comments and processing instructions.
 * This leaves the cursor positioned at the next <tag> </tag>
 * or EOF.
 */
void
cursor_skip_content(struct cursor *c)
{
	while (!cursor_is_at_eof(c)) {
		if (*c->pos != '<')
			c->pos++;
		else if (cursor_eat(c, "<!--"))
			cursor_skip_to(c, "-->");	/* TODO handle ---> */
		else if (cursor_eat(c, "<?"))
			cursor_skip_to(c, "?>");	/* TODO handle "..." */
		else
			break;				/* TODO handle CDATA */
	}
}

/** Advances cursor to position it at the next un-matched "</" or EOF */
void
cursor_skip_to_close(struct cursor *c)
{
	unsigned int depth = 0;
	cursor_skip_content(c); /* Leaves cursor at eof or '<' */
	while (!cursor_is_at_eof(c)) {
		/* *c->pos == '<' */
		if (cursor_is_at(c, "</")) {
			if (!depth)
				return;
			depth--;
			cursor_skip_to_ch(c, '>');
		} else {
			/* At <sometag; skip to its '>' */
			depth++;
			cursor_skip_to_ch(c, '>'); /* TODO handle attributes */
		}
		cursor_skip_content(c);
	}
}

