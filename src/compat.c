#include "hiwire.h"
#include "stdalign.h"

_Static_assert(alignof(HwRef) == alignof(uint),
               "HwRef should have the same alignment as int.");
_Static_assert(sizeof(HwRef) == sizeof(uint),
               "HwRef should have the same size as int.");

// memset, NULL, size_t
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

// realloc
#if defined(_HIWIRE_STATIC_PAGES) && defined(_HIWIRE_EXTERN_REALLOC)
#error "only define one of STATIC_PAGES or EXTERN_REALLOC"
#endif

#if defined(_HIWIRE_STATIC_PAGES)
// provide a static realloc on request
uint _table[_HIWIRE_STATIC_PAGES * ALLOC_INCREMENT];
static inline void*
_hiwire_realloc(void* orig, size_t sz)
{
  if (sz <= _HIWIRE_STATIC_PAGES * ALLOC_INCREMENT * sizeof(uint)) {
    return _table;
  }
  return 0;
}
#elif defined(_HIWIRE_EXTERN_REALLOC)
#define _hiwire_realloc hiwire_realloc
#else
#define _hiwire_realloc realloc
#endif

void*
_hiwire_realloc(void* orig, size_t sz);

// tracerefs
#if defined(_HIWIRE_EMSCRIPTEN_TRACEREFS) && !defined(__EMSCRIPTEN__)
#error "EMSCRIPTEN_TRACEREFS only works with Emscripten"
#endif
#if defined(_HIWIRE_EMSCRIPTEN_TRACEREFS) && defined(_HIWIRE_EXTERN_TRACEREFS)
#error "only define one of EMSCRIPTEN_TRACEREFS or EXTERN_TRACEREFS"
#endif

#if defined(_HIWIRE_EXTERN_TRACEREFS)
#define _hiwire_traceref hiwire_traceref
#endif

void
_hiwire_traceref(char* type,
                 HwRef ref,
                 uint index,
                 __externref_t value,
                 uint refcount);

#ifdef _HIWIRE_EMSCRIPTEN_TRACEREFS
#include "emscripten.h"

// clang-format off
EM_JS(void,
_hiwire_traceref,
(char* type_ptr, HwRef ref, uint index, __externref_t value, uint refcount),
{
  const type = UTF8ToString(type_ptr);
  console.log("hiwire traceref", { type, ref, index, value, refcount });
});
// clang-format on

EM_JS_DEPS(_hiwire_traceref, "$UTF8ToString");
#endif

// fail
#if defined(_HIWIRE_EXTERN_FAIL) && defined(_HIWIRE_ABORT)
#error "only define one of EMSCRIPTEN_TRACEREFS or EXTERN_TRACEREFS"
#endif

#if defined(_HIWIRE_EXTERN_FAIL)
void
hiwire_invalid_ref(int, HwRef);

#elif defined(_HIWIRE_ABORT_FAIL)

#if !__has_include("stdlib.h")
#error "Cannot use ABORT_FAIL with wasm32-unknown, use EXTERN_FAIL instead"
#endif
#include "stdio.h"
#include "stdlib.h"

static void
hiwire_invalid_ref(int type, HwRef ref)
{
  char* typestr = NULL;
  char* reason = NULL;
  switch (type) {
    case HIWIRE_FAIL_GET:
      typestr = "hiwire_get";
      break;
    case HIWIRE_FAIL_INCREF:
      typestr = "hiwire_incref";
      break;
    case HIWIRE_FAIL_DECREF:
      typestr = "hiwire_decref";
      break;
  }
  if (ref == NULL) {
    reason = "null";
  } else {
    reason = "freed";
  }
  fprintf(stderr, "%s failed: reference %d is %s\n", typestr, (int)ref, reason);
  abort();
}

#else
#define hiwire_invalid_ref(type, ref)
#endif
