#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pool.h"

#define POOL_PAGE_SIZE 4000

struct pool_header {
	struct pool_header *older;
	size_t offset;
};

void *
pool_alloc(POOL *pool, size_t size)
{
	// allocate new page if neccessary
	struct pool_header *header = *pool;
	if (!header || POOL_PAGE_SIZE - size < header->offset) {
		struct pool_header *new_header = malloc(POOL_PAGE_SIZE);
		new_header->older = header;
		new_header->offset = sizeof(struct pool_header);
		assert((new_header->offset & 15) == 0);
		*pool = header = new_header;
	}

	// allocate from the top page
	void *pointer = (char *)*pool + header->offset;
	header->offset += size;
	
	// realign offset to 16-byte boundary
	header->offset = (header->offset + 15) & ~(size_t)15;

	memset(pointer, 0, size);
	return pointer;
}

void
pool_release(POOL *pool)
{
	struct pool_header *header;
	while ((header = *pool)) {
		*pool = header->older;
		free(header);
	}
}
