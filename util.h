#include <stddef.h>
#include <stdbool.h>

#define IS_POW2(x) (!((x) & ((x)-1)))
#define GROW_ARRAY(arr, len) do {							\
		if (IS_POW2(len)) {							\
			arr = reallocarray(arr, len ? len * 2 : 1, sizeof(arr[0]));	\
		}									\
	} while (0)

#define DIV_ROUNDUP(a, b) (((a)+(b)-1)/(b))
#define UINT_BITS (8*sizeof(unsigned))
#define SET_BIT(b,i) ((b)[(i)/UINT_BITS] |= 1<<((i)%UINT_BITS))
#define CLR_BIT(b,i) ((b)[(i)/UINT_BITS] &= ~(1<<((i)%UINT_BITS)))
#define GET_BIT(b,i) (((b)[(i)/UINT_BITS] >> (i%UINT_BITS)) & 1)

void *reallocarray(void *, size_t, size_t);
uint32_t memhash(const void *, size_t);
uint32_t strhash(const char *);
uint32_t xorfold(uint32_t, int);

