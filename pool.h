typedef void *POOL;

void *pool_alloc  (POOL *pool, size_t size);
void  pool_release(POOL *pool);
