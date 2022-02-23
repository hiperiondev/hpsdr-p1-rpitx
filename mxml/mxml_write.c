#include <string.h>

#include "mxml.h"
#include "mxml_int.h"

struct write_token_context {
	int (*writefn)(void *context, const char *text, unsigned int len);
	void *context;
};

static int
write_token(void *context, const struct token *token)
{
	struct write_token_context *c = context;
	const char *first;
	const char *entity;
	const char *text;
	int ret = 0;

	if (token->valuelen > 0) {
		return c->writefn(c->context, token->value, token->valuelen);
	}
	if (token->valuelen == 0)
		return 0;

#define OUT(s, len) do { \
		int _len = (len); \
		if (_len) { \
			const char *_s = (s); \
			int _n = c->writefn(c->context, _s, _len); \
			if (_n == -1) return -1; \
			ret += _n; \
		} \
	} while (0)

	text = token->value;
	while ((text = strpbrk(first = text, "<>&"))) {
		OUT(first, text - first);
		if (*text == '<')
			entity = "&lt;";
		else if (*text == '>')
			entity = "&gt;";
		else /* if (*text == '&') */
			entity = "&amp;";
		OUT(entity, strlen(entity));
		text++;
	}
	OUT(first, strlen(first));
	return ret;
#undef OUT
}

int
mxml_write(const struct mxml *m,
	   int (*writefn)(void *context, const char *text, unsigned int len),
	   void *context)
{
	struct write_token_context c = {
		.writefn = writefn,
		.context = context
	};
	return flatten_edits(m, write_token, &c);
}
