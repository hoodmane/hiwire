#include "testlib.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"

// clang-format off
EM_JS(__externref_t, int_to_ref, (int x), {
    return x;
})

EM_JS(__externref_t, get_obj, (int x), {
    return {x};
})

EM_JS(int, ref_to_int, (__externref_t x), {
    return x;
})

EM_JS(int, is_null, (__externref_t x), {
    return x == null;
})
// clang-format on

#endif

#if !defined(__wasi__) && !defined(__EMSCRIPTEN__)
int
main();
int
_start()
{
  return main();
}

int
puts(const char* x)
{
  return printf("%s\n", x);
}
#endif

#ifdef _HIWIRE_EXTERN_DEDUPLICATE

HwRef
hiwire_deduplicate_get(__externref_t value)
{
  return _hiwire_deduplicate_get(value);
}

void
hiwire_deduplicate_set(__externref_t value, HwRef ref)
{
  _hiwire_deduplicate_set(value, ref);
}

void
hiwire_deduplicate_delete(__externref_t value)
{
  _hiwire_deduplicate_delete(value);
}

#endif

#ifdef _HIWIRE_EXTERN_TRACEREFS

void
hiwire_traceref(char* type,
                HwRef ref,
                int index,
                __externref_t value,
                int refcount)
{
  extern_hiwire_traceref(type, ref, index, value, refcount);
}
#endif
