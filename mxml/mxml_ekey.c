#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>	/* snprintf */

#include "mxml_int.h"

/**
 * Parse an unsigned integer.
 * Leading and trailing whitespace is permiited.
 * @param s the buffer containing the unsigned integer to parse
 * @param n the number of bytes in the input @a s
 * @param retval (optional) storage for the returned unsigned integer
 * @retval 0 success
 * @retval -1 [ERANGE] the integer in @a s was too big
 * @retval -1 [EINVAL] the input was not an unsigned integer.
 */
int
parse_uint(const char *s, int n, unsigned int *retval)
{
	unsigned int val = 0;

	while (n && isspace(*s))
		n--, s++;
	if (!(n && isdigit(*s)))
		goto inval;
	while (n && isdigit(*s)) {
		if (val > (UINT_MAX / 10) ||
		    (val == (UINT_MAX / 10) && (*s - '0') > (UINT_MAX % 10)))
		{
			errno = ERANGE;
			return -1;
		}
		val = (val * 10) + (*s - '0');
		n--, s++;
	}
	while (n && isspace(*s))
		n--, s++;
	if (n)
		goto inval;
	if (retval)
		*retval = val;
	return 0;
inval:
	errno = EINVAL;
	return -1;
}

/**
 * Expands a user key into a fully-dotted form.
 * For example, "foo.bar[4].baz" expands to "foo.bars.bar4.baz".
 *              "foo.bar[#]" expands to "foo.bars.total"
 *              "foo.bar[$]" expands to "foo.bars.bar<N>"
 *              "foo.bar[+]" expands to "foo.bars.bar<N+1>"
 *		where <N> is the current foo.bars.total value.
 * @param outbuf   the return buffer for the expanded key
 * @param outbufsz the size of the return buffer
 * @param key   the source key to expand
 * @returns length of expanded key in @a outbuf
 * @retval -1 [ENOMEM] if the expanded key exhausted the buffer
 * @retval -1 [EINVAL] the key was malformed
 */
int
expand_key(struct mxml *m, char *outbuf, size_t outbufsz, const char *key)
{
	char * const end = outbuf + outbufsz;
	char *b = outbuf;
	const char *tagstart;	/* points into key after last . */
	const char *w;

	/*
	 * A collection of macros to safely append
	 * to the output buffer
	 */
#	define OUTB(ch) /* Appends char ch to the buffer */ \
	    do { \
		if (b < end) *b++ = (ch); \
		else goto nomem; \
	    } while (0)
#	define OUTNUL() /* Sets last byte to NUL */ \
	    do { \
		if (b < end) *b = '\0'; \
		else goto nomem; \
	    } while (0)
#	define OUTSTR(s) /* Appends string s to the buffer */ \
	    do { \
		const char *_s; \
		for (_s = (s); *_s; _s++) OUTB(*_s); \
	    } while (0)
#	define OUTUINT(u) /* Appends unsigned integer u to the buffer */ \
	    do { \
	        int _z = snprintf(b, end - b, "%u", (u)); \
		if (b + _z < end) b += _z; \
		else goto nomem; \
	    } while (0)

	tagstart = key;
	while (*key) {
		if (*key == '.') {
			if (key == tagstart)
				goto invalid; /* double . */
			/* The next tag starts after the . */
			tagstart = key + 1;
		}
		if (*key != '[') {
			OUTB(*key++);
			continue;
		}
		/* Now handling ".tag[...]" */
		if (tagstart == key)
			goto invalid; /* missing tag before [ */
		OUTB('s');
		key++; /* Skip input '[' */
		if (*key >= '1' && *key <= '9') {
			/* handling .tag[123] -> .tags.tag123 */
			OUTB('.');
			for (w = tagstart; *w != '['; w++)
				OUTB(*w);
			while (*key >= '0' && *key <= '9')
				OUTB(*key++);
		} else if (*key == '*') {
			/* handling .tag[*] -> .tags */
			if (key[1] != ']' || key[2] != '\0')
				goto invalid; /* [*] must be last */
			key++;
		} else if (*key == '#' || *key == '$' || *key == '+') {
			/* handling .tag[#] -> .tags.total
			 *          .tag[$] -> .tags.tag<N>
			 *          .tag[+] -> .tags.tag<N+1>
			 * where N is the current total value */
			char *b_save;

			/* Construct ".tags.total" now because either
			 * that will be our final output, or we will
			 * be using outbuf as a temporary to extract
			 * the actual current tags.total value. */
			OUTB('.');
			b_save = b;
			OUTSTR("total");
			if (*key == '#') {
				/* tag[#] => tags.total */
				if (key[1] != ']' || key[2] != '\0')
					goto invalid; /* [#] must be last */
			} else if (*key == '$' || *key == '+') {
				unsigned int total;
				size_t totalsz = 0;
				/* fetch the value for ".tags.total" */
				const char *totalstr = find_key(m,
				    outbuf, b - outbuf, &totalsz);
				/* convert it into a number */
				if (parse_uint(totalstr, totalsz, &total) < 0)
					total = 0; /* default to 0 */
				if (*key == '+')
					total++;
				b = b_save; /* back up to "tags." */
				for (w = tagstart; *w != '['; w++)
					OUTB(*w); /* "tags.tag" */
				OUTUINT(total);   /* "tags.tagN" */
			} else {
				goto invalid; /* expected tag[<int>|$|+|#] */
			}
			key++;
		} else
			goto invalid; /* invalid char after [ */
		if (*key++ != ']')
			goto invalid;	/* expected ] */
		if (*key && *key != '.')
			goto invalid;	/* expected EOS or . after ] */
	}
	if (tagstart == b)
		goto invalid;	/* empty or no tag after last . */
	OUTNUL();
	return b - outbuf;
nomem:
	errno = ENOMEM;
	return -1;
invalid:
	errno = EINVAL;
	return -1;

#undef OUTUINT
#undef OUTSTR
#undef OUTNUL
#undef OUTB
}

