#include <string.h>

#include "mxml.h"
#include "mxml_int.h"
#include "mxml_mem.h"

struct keys_context {
    char **keys;
    unsigned int nkeys;
};

static int keys_token(void *context, const struct token *token) {
    if (token->type == TOK_OPEN) {
        struct keys_context *c = context;
        char **keys;
        char *key = strndup(token->key, token->keylen);

        if (!key)
            return -1;
        keys = realloc(c->keys, (c->nkeys + 1) * sizeof *keys);
        if (!keys) {
            _mxml_free(key);
            return -1;
        }
        c->keys = keys;
        c->keys[c->nkeys++] = key;
    }
    return 0;
}

char** mxml_keys(const struct mxml *m, unsigned int *nkeys_return) {
    struct keys_context c;

    c.keys = NULL;
    c.nkeys = 0;
    if (flatten_edits(m, keys_token, &c) == -1) {
        mxml__mxml_free_keys(c.keys, c.nkeys);
        *nkeys_return = 0;
        return NULL;
    }
    *nkeys_return = c.nkeys;
    return c.keys ? c.keys : _mxml_malloc(1);
}

void mxml__mxml_free_keys(char **keys, unsigned int nkeys) {
    unsigned int i;

    for (i = 0; i < nkeys; i++)
        _mxml_free(keys[i]);
    _mxml_free(keys);
}
