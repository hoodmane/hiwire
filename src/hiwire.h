#ifndef HIWIRE_H
#define HIWIRE_H

#if !__wasm32__
#error "Only for wasm32"
#endif
#if !defined(__wasm_reference_types__)
#error "Wasm reference types are required. Compile with -mreference-types."
#endif

#if defined(HIWIRE_EMSCRIPTEN_DEDUPLICATE) && !defined(__EMSCRIPTEN__)
#error "HIWIRE_EMSCRIPTEN_DEDUPLICATE only works on with Emscripten"
#endif
#if defined(HIWIRE_EMSCRIPTEN_DEDUPLICATE) && defined(HIWIRE_EXTERN_DEDUPLICATE)
#error                                                                         \
  "only define one of HIWIRE_EMSCRIPTEN_DEDUPLICATE or HIWIRE_EXTERN_DEDUPLICATE"
#endif
#if defined(HIWIRE_STATIC_PAGES) && defined(HIWIRE_EXTERN_REALLOC)
#error "only define one of HIWIRE_STATIC_PAGES or HIWIRE_EXTERN_REALLOC"
#endif

#ifdef __cplusplus
extern “C”
{
#endif
#if true && false
  // Prevent clang-format from indenting extern "C" body.
  // It's smart enough to understand #if false and ignore it, but not true &&
  // false!
}
#endif

/**
 * hiwire: A super-simple framework for converting values between C and
 * JavaScript.
 *
 * Arbitrary JavaScript objects are referenced from C using an opaque HwRef
 * value.
 *
 * JavaScript objects passed to the C side must be manually reference-counted.
 * Use `hiwire_incref` if you plan to store the object on the C side. Use
 * `hiwire_decref` when done. Internally, the objects are stored in a global
 * object. There may be one or more keys pointing to the same object.
 */

// HwRef is a NewType of int. We need
//
// alignof(HwRef) = alignof(int) = 4 and
// sizeof(HwRef) = sizeof(int) = 4
//
// To be future proof, we have _Static_asserts for this in hiwire.c.
struct _HwRefStruct
{};

typedef struct _HwRefStruct* HwRef;

/**
 * Place a reference counted value into the map under a new key and return the
 * key.
 *
 * It's possible to have at most 2^25 = 33,554,432 distinct references. If
 * allocation fails, will return `NULL`.
 */
HwRef
hiwire_new(__externref_t value);

/**
 * Place an immortal reference into the map under a new key and return the
 * key. It's possible to have at most 2^31 immortal references.
 */
HwRef
hiwire_intern(__externref_t value);

/**
 * Look up the externref corresponding to the given HwRef
 */
__externref_t
hiwire_get(HwRef ref);

void
hiwire_incref(HwRef ref);

void
hiwire_decref(HwRef ref);

/**
 * Use hiwire_get to look up the value then decrement the reference count.
 */
__externref_t
hiwire_pop(HwRef ref);

/**
 * If ref1 and ref2 point to the same host value, hiwire_incref_deduplicate
 * will return equal values. This can be used to check equality of the host
 * values with C equality checks.
 */
HwRef
hiwire_incref_deduplicate(HwRef ref)
#if !defined(HIWIRE_EMSCRIPTEN_DEDUPLICATE) &&                                 \
  !defined(HIWIRE_EXTERN_DEDUPLICATE)
  __attribute__((unavailable(
    "To use hiwire_incref_deduplicate you must compile with "
    "-DHIWIRE_EMSCRIPTEN_DEDUPLICATE or -DHIWIRE_EXTERN_DEDUPLICATE")))
#endif
  ;

#ifdef __cplusplus
}
#endif

#endif // HIWIRE_H
