#include <stdlib.h>	/* size_t */

#define HAVE_CACHE	1	/* Enable cache by default */

#define KEY_MAX		256	/* maximum length of expanded key */

/* In-memory XML parser and editor */
struct mxml {
	const char *start;	/* First char of XML document */
	size_t size;		/* Length of XML document */
	struct edit *edits;	/* Reverse list of edits */
#if HAVE_CACHE
# define CACHE_MAX 32
	/* A small cache of previously-found key prefixes
	 * within the XML body start[0..size-1] */
	struct cache {
		char key[KEY_MAX];
		int keylen;
		const char *data; /* First byte after <tag> */
		size_t size;
	} cache[CACHE_MAX];
	unsigned int cache_next;
#endif
};

/* An edit record. These are always held unintegrated */
struct edit {
	struct edit *next;
	char *key;
	char *value;	/* Must be NULL when op=EDIT_DELETE */
	enum edit_op { EDIT_DELETE, EDIT_SET, EDIT_APPEND } op;
};

/* A bounded text cursor */
struct cursor {
	const char *pos;
	const char *end;
};

struct token {
	enum { TOK_EMPTY, TOK_EOF, TOK_OPEN, TOK_VALUE, TOK_CLOSE } type;
	char key[KEY_MAX];
	int keylen;
	const char *value;
	int valuelen;		/* -1 if the value is NUL-terminated
				 * and needs XML-encoding */
};

/* Export these functions */
#define EXPORT __attribute__((visibility ("default")))
EXPORT int mxml_append();
EXPORT int mxml_delete();
EXPORT int mxml_exists();
EXPORT char *mxml_expand_key();
EXPORT void mxml_free();
EXPORT void mxml_free_keys();
EXPORT char *mxml_get();
EXPORT char **mxml_keys();
EXPORT struct mxml *mxml_new();
EXPORT int mxml_set();
EXPORT int mxml_update();
EXPORT int mxml_write();

/* mxml_cursor.c */
int cursor_is_at_eof(const struct cursor *c);
int cursor_is_at(const struct cursor *c, const char *s);
int cursor_eatn(struct cursor *c, const char *s, unsigned int slen);
int cursor_eatch(struct cursor *c, char ch);
int cursor_eat_white(struct cursor *c);
void cursor_skip_to_ch(struct cursor *c, char ch);
void cursor_skip_content(struct cursor *c);
void cursor_skip_to_close(struct cursor *c);

/* mxml_cache.c */
#if HAVE_CACHE
void cache_init(struct mxml *m);
const char *cache_get(struct mxml *m, const char *ekey, int ekeylen,
		      size_t *sz_return);
void cache_set(struct mxml *m, const char *ekey, int ekeylen,
	       const char *data, size_t size);
#endif

/* mxml_ekey.c */
int expand_key(struct mxml *m, char *outbuf, size_t outbufsz, const char *key);
int parse_uint(const char *s, int n, unsigned int *retval);


/* mxml_find.c */
const char *find_key(struct mxml *m, const char *ekey,
	int ekeylen, size_t *sz_return);

/* mxml_flatten.c */
int flatten_edits(const struct mxml *m,
	          int (*fn)(void *context, const struct token *token),
	          void *context);
