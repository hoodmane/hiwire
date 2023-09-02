#include "stdalign.h"

_Static_assert(alignof(HwRef) == alignof(int),
               "HwRef should have the same alignment as int.");
_Static_assert(sizeof(HwRef) == sizeof(int),
               "HwRef should have the same size as int.");

#if __has_include("stdlib.h")
#include "stdlib.h"
#include "string.h"
#define hiwire_memset memset
#else

// wasm32-unknown-unknown, we need substitutes for the stdlib

typedef unsigned long size_t;
#define NULL ((void*)(0))

#if defined(__wasm_bulk_memory__)
#define hiwire_memset __builtin_memset
#else
#error "Need wasm bulk memory when targeting wasm32-unknown"
#endif
#endif

#if defined(_HIWIRE_STATIC_PAGES)
// provide a static realloc on request
int _table[_HIWIRE_STATIC_PAGES * ALLOC_INCREMENT];
static inline void*
_hiwire_realloc(void* orig, size_t sz)
{
  if (sz <= _HIWIRE_STATIC_PAGES * ALLOC_INCREMENT * sizeof(int)) {
    return _table;
  }
  return 0;
}
#elif defined(_HIWIRE_EXTERN_REALLOC)
#define _hiwire_realloc hiwire_realloc
#else
#define _hiwire_realloc realloc
void*
realloc(void* orig, size_t sz);
#endif
