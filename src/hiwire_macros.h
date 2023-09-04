typedef unsigned int uint;

#if defined(_HIWIRE_EMSCRIPTEN_TRACEREFS) || defined(_HIWIRE_EXTERN_TRACEREFS)
#define TRACEREFS(type, ref)                                                   \
  _hiwire_traceref(                                                            \
    type,                                                                      \
    ref,                                                                       \
    HEAP_REF_TO_INDEX(ref),                                                    \
    _hiwire_get(HEAP_REF_TO_INDEX(ref)),                                       \
    (_hiwire.slotInfo[HEAP_REF_TO_INDEX(ref)] & REFCOUNT_MASK) >> 2)

#if defined(_HIWIRE_EXTERN_TRACEREFS)
#define _hiwire_traceref hiwire_traceref
#endif
void
_hiwire_traceref(char* type,
                 HwRef ref,
                 uint index,
                 __externref_t value,
                 uint refcount);
#else
#define TRACEREFS(...)
#endif

// HwRefs are:
// * heap             if they are odd,
// * immortal         if they are even
//
// Note that "NULL" is immortal which is important.
#define IS_IMMORTAL(ref) ((((uint)(ref)) & 1) == 0)
#define IMMORTAL_REF_TO_INDEX(ref) (((uint)(ref)) >> 1)
#define IMMORTAL_INDEX_TO_REF(index) ((HwRef)((index) << 1))

// clang-format off
// Heap slotmap layout macros

// The idea of a slotmap is that we use a list for storage. we use the empty
// slots in the list to maintain a linked list of freed indices in the same
// place as the values. This means that the next slot we assign is always the
// most recently freed. This leads to the possibility of masking use after free
// errors, since a recently freed reference will likely point to a valid but
// different object. To deal with this, we include as part of the reference a 6
// bit version for each slot. Only if the same slot is freed and reassigned 32
// times can the two references be the same. The references look as follows:
//
//   [version (6 bits)][index (25 bits)]1
//
// The highest order 6 bits are the version, the middle 25 bits are the index,
// and the least order bit indicates that it is a heap reference. Since we have
// 25 bits for the index, we can store up to 2^25 = 33,554,432 distinct objects.
// For each slot we associate an 32 bit "info" integer, which we store as part
// of the slotmap state. So references, occupied slot info, and unoccupied slot
// info all look like:
//
//   [version (6 bits)][multipurpose field (25 bits)][1 bit]
//
// The least significant bit is set in the references to indicate that they are
// heap references. The least significant bit is set in the info if the slot is
// occupied and unset if the slot is unoccupied.
//
// In a reference, the mulipurpose field contains the slot index.
//
//          reference: [version (6 bits)][index (25 bits)]1
//
// If a slot is unoccupied, the multipurpose field of the slotInfo contains the
// index of the next free slot in the free list or zero if this is the last free
// slot (for this reason, we do not use slot 0).
//
//    unoccupied slot: [version (6 bits)][next free index (25 bits)]0
//
// If a slot is occupied, the multipurpose field of the slotInfo contains a 24
// bit reference count and an IS_DEDUPLICATED bit.
//
//      occupied slot: [version (6 bits)][refcount (24 bits)][IS_DEDUPLICATED bit]1
//
// Deduplicated HwRefs are ~50x more expensive to allocate/deallocate.
// clang-format on

// 1 occupied bit, 25 bits of index/nextfree/refcount, then the version
#define OCCUPIED_BIT (1 << 0)
#define DEDUPLICATED_BIT (1 << 1)
#define REFCOUNT_INTERVAL (1 << 2)

#define VERSION_SHIFT 26
#define VERSION_MASK (((uint)(-1)) << VERSION_SHIFT)

_Static_assert(VERSION_MASK == 0xFc000000         , "oops!");

#define VERSION_OCCUPIED_MASK (VERSION_MASK | OCCUPIED_BIT)
#define REFCOUNT_MASK (~(VERSION_MASK | OCCUPIED_BIT | DEDUPLICATED_BIT))
#define INDEX_MASK (~(VERSION_MASK | OCCUPIED_BIT))
#define MAX_INDEX (INDEX_MASK >> 1)

#define NEW_INFO_FLAGS (REFCOUNT_INTERVAL | OCCUPIED_BIT)

#define HEAP_REF_TO_INDEX(ref) ((((uint)(ref)) & INDEX_MASK) >> 1)
#define HEAP_INFO_TO_NEXTFREE(info) HEAP_REF_TO_INDEX(info)

// The ref is always odd so this is truthy if info is even (meaning unoccupied)
// or info has a different version than ref. Masking removes the bits that form
// the index in the reference and the refcount/next free index in the info.
#define HEAP_REF_IS_OUT_OF_DATE(ref, info)                                     \
  ((((uint)(ref)) ^ (info)) & VERSION_OCCUPIED_MASK)

#define HEAP_IS_REFCNT_ZERO(info) (!((info)&REFCOUNT_MASK))
#define HEAP_IS_DEDUPLICATED(info) ((info)&DEDUPLICATED_BIT)

#define HEAP_INCREF(info) info += REFCOUNT_INTERVAL
#define HEAP_DECREF(info) info -= REFCOUNT_INTERVAL

// increment the version in info.
#define _NEXT_VERSION(info) (info + (1 << VERSION_SHIFT))
// assemble version, field, and occupied
#define _NEW_INFO(version, field_and_flag)                                     \
  (((version)&VERSION_MASK) | (field_and_flag))

// make a new reference with the same version as info and the given index.
#define HEAP_NEW_REF(index, info) ((HwRef)_NEW_INFO(info, ((index) << 1) | 1))
// new occupied info: same version as argument info, NEW_INFO_FLAGS says
// occupied with refcount 1
#define HEAP_NEW_OCCUPIED_INFO(info) _NEW_INFO(info, NEW_INFO_FLAGS)
// new unoccupied info, increment version and nextfree in the field
#define FREE_LIST_INFO(info, nextfree)                                         \
  _NEW_INFO(_NEXT_VERSION(info), (nextfree) << 1)
