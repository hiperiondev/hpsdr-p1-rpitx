#define _GNU_SOURCE /* memrchr */
#include <string.h>
#include <errno.h>

#include "mxml_int.h"

/** Tests if key @a child is equal to, or a descendent of @a parent.
 *  That is, is parent equal to, or a prefix of child? */
static int
is_key_self_or_child(const char *child, int childlen,
		     const char *parent, int parentlen)
{
	if (childlen < parentlen)
		return 0;
	if (childlen == parentlen)
		return strncmp(child, parent, childlen) == 0;
	if (child[parentlen] != '.')
		return 0;
	return strncmp(child, parent, parentlen) == 0;
}

/** Finds element key and returns its inner content span.
 *  @param reqkey an expanded key, eg "foo.bars.bar3.baz" (no "[...]")
 *  @param sz_return storage for returning the length of the span in bytes
 *  @returns pointer to beginning of the content inside "<baz>...</baz>"
 *  @retval NULL [ENOENT] if the element for the key was not found.
 **/
static const char *
find_key_noedit(struct mxml *m, const char *reqkey, int reqkeylen,
		size_t *sz_return)
{
	const char *dot;
	const char *parent_text;
	size_t parent_sz;
	struct cursor c;
	const char *tag;
	unsigned int taglen;
	const char *ret;

#if HAVE_CACHE
	/* Search the cache for an exact match */
	ret = cache_get(m, reqkey, reqkeylen, sz_return);
	if (ret) {
		errno = ENOENT;
		return ret;
	}
#endif

	/* Find the XML content for reqkey's parent element */
	dot = memrchr(reqkey, '.', reqkeylen);
	if (dot) {
		parent_text = find_key_noedit(m, reqkey, dot - reqkey,
					      &parent_sz);
		if (!parent_text)
			return NULL;	/* Could not find parent */
		tag = dot + 1;
		taglen = reqkeylen - (tag - reqkey);
	} else {
		parent_text = m->start;
		parent_sz = m->size;
		tag = reqkey;
		taglen = reqkeylen;
	}
	c.pos = parent_text;
	c.end = parent_text + parent_sz;

	/* Hunt forward for the first opening <tag> or closing </parent> */
	cursor_skip_content(&c); /* Leaves cursor at eof or '<' */
	while (!cursor_is_at_eof(&c) && !cursor_is_at(&c, "</")) {
		/* We found a direct child of the parent */
		cursor_eatch(&c, '<');
		if (cursor_eatn(&c, tag, taglen) && /* It starts with "<tag" */
		    (cursor_is_at(&c, ">") || cursor_eat_white(&c)))
		{
			/* We matched "<tag>" or "<tag " */
			cursor_skip_to_ch(&c, '>'); /* TODO attributes */
			cursor_eatch(&c, '>');
			ret = c.pos; /* Content starts after '>' */
			cursor_skip_to_close(&c); /* cursor is now at </tag> */
			*sz_return = c.pos - ret;
#if HAVE_CACHE
			cache_set(m, reqkey, reqkeylen, ret, *sz_return);
#endif
			return ret;
		}
		/* We found "<othertag" */
		cursor_skip_to_ch(&c, '>'); /* TODO assumes no attributes */
		cursor_skip_to_close(&c); /* Skip to matching unmatched </ */
		/* XXX assumes well-formed xml, ie we are at </othertag> */
		cursor_skip_to_ch(&c, '>'); /* Skip over </othertag> */
		cursor_skip_content(&c);
	}
	errno = ENOENT;
	return NULL;	/* No more tags found in the parent */
}

/**
 * Finds an expanded key's edited value.
 * First looks in the edit list, then in the XML.
 * @param reqkey an expanded key
 * @returns pointer to the value span
 * @retval NULL [ENOENT] key not found or deleted
 */
const char *
find_key(struct mxml *m, const char *reqkey, int reqkeylen, size_t *sz_return)
{
	const struct edit *e;
	int implied_by_append = 0;
	const char *ret;

	/* Look in the most recent edits, first */
	for (e = m->edits; e; e = e->next) {
		switch (e->op) {
		case EDIT_DELETE:
			if (is_key_self_or_child(reqkey, reqkeylen,
			    e->key, strlen(e->key)))
			{
				errno = ENOENT;
				return NULL;
			}
			break;
		case EDIT_APPEND:
			/* Remember for later the appending of a descendent
			 * as it implies the creation of a parent */
			if (is_key_self_or_child(e->key, strlen(e->key),
			    reqkey, reqkeylen))
				implied_by_append = 1;
			/* Fallthrough */
		case EDIT_SET:
			if (strncmp(e->key, reqkey, reqkeylen) == 0 &&
			    e->key[reqkeylen] == '\0')
			{
				*sz_return = strlen(e->value);
				return e->value;
			}
			break;
		}
	}

	ret = find_key_noedit(m, reqkey, reqkeylen, sz_return);
	if (!ret && implied_by_append) {
		/* Fake an implied intermediate parent */
		ret = "";
		*sz_return = 0;
	}
	return ret;
}

