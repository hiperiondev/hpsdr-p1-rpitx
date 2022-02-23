#include <string.h>

#include "mxml_int.h"

#if HAVE_CACHE
void
cache_init(struct mxml *m)
{
	unsigned int i;

	for (i = 0; i < CACHE_MAX; i++)
		m->cache[i].keylen = 0;
	m->cache_next = 0;
}

const char *
cache_get(struct mxml *m, const char *ekey, int ekeylen, size_t *sz_return)
{
	unsigned int i;

	if (ekeylen > KEY_MAX)
		return NULL;
	for (i = 0; i < CACHE_MAX; i++) {
		const struct cache *cache = &m->cache[i];
		if (cache->keylen == ekeylen &&
		    memcmp(cache->key, ekey, ekeylen) == 0)
		{
			*sz_return = cache->size;
			return cache->data;
		}
	}
	return NULL;
}

void
cache_set(struct mxml *m, const char *ekey, int ekeylen,
	  const char *data, size_t size)
{
	struct cache *cache;

	if (ekeylen <= KEY_MAX) {
		cache = &m->cache[m->cache_next];
		cache->keylen = ekeylen;
		memcpy(cache->key, ekey, ekeylen);
		cache->data = data;
		cache->size = size;
		/* simple round robin cache eviction */
		m->cache_next = (m->cache_next + 1) % CACHE_MAX;
	}
}
#endif /* HAVE_CACHE */
