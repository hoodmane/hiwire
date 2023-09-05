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

#ifdef __wasi__
// If we're in wasi, in order to interleave correctly the strings printed by
// Python with those printed by wasi, we need to route the python strings
// through wasi print.
// To do this we need puts, malloc, and free.
__attribute__((export_name("print_str"))) int
print_str(char* str)
{
  return fputs(str, stdout);
}

#include "stdlib.h"
__attribute__((export_name("malloc"))) void*
malloc_export(int size)
{
  return malloc(size);
}

__attribute__((export_name("free"))) void
free_export(void* ptr)
{
  free(ptr);
}
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
                uint index,
                __externref_t value,
                uint refcount)
{
  extern_hiwire_traceref(type, ref, index, value, refcount);
}
#endif
