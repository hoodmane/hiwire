#include "testlib.h"

// clang-format off
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#else
#define EM_JS(ret, name, params, rest...) \
    __attribute__((import_module("env"), import_name(# name))) \
    ret name params;

#endif

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
