
#if __has_include("stdlib.h")
#include "stdlib.h"
#else

typedef unsigned long size_t;
#define NULL ((void*)(0))
// we'll need a definition of realloc!
void*
realloc(void* orig, size_t sz);
#endif
// memset is an intrinsic so it works fine even in wasm32-unknown-unknown
void*
memset(void*, int, size_t);

#if defined(HIWIRE_STATIC_PAGES)
// provide a static realloc on request
int _table[HIWIRE_STATIC_PAGES * ALLOC_INCREMENT];
static inline void*
_hiwire_realloc(void* orig, size_t sz)
{
  if (sz < HIWIRE_STATIC_PAGES * ALLOC_INCREMENT * sizeof(int)) {
    return _table;
  }
  return 0;
}
#elif defined(HIWIRE_EXTERN_REALLOC)
#define _hiwire_realloc hiwire_realloc
#else
#define _hiwire_realloc realloc
#endif
