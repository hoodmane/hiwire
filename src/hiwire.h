#ifndef HIWIRE_H
#define HIWIRE_H

#if !__wasm32__
#error "Only for wasm32"
#endif
#if !defined(__wasm_reference_types__)
#error "Wasm reference types are required. Compile with -mreference-types."
#endif

#if defined(__EMSCRIPTEN__) || defined(EXTERN_DEDUPLICATE)
#define _HIWIRE_CAN_DEDUPLICATE 1
#else
#define _HIWIRE_CAN_DEDUPLICATE 0
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

HwRef
hiwire_intern(__externref_t value);

HwRef
hiwire_new(__externref_t ref);

__externref_t
hiwire_get(HwRef ref);

void
hiwire_incref (HwRef ref);

void
hiwire_decref (HwRef ref);

__externref_t
hiwire_pop (HwRef ref);


#if _HIWIRE_CAN_DEDUPLICATE
HwRef hiwire_incref_deduplicate(HwRef ref);
#endif
#endif
