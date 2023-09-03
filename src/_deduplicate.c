#ifdef HIWIRE_EXTERN_DEDUPLICATE
#define _hiwire_deduplicate_get hiwire_deduplicate_get
#define _hiwire_deduplicate_set hiwire_deduplicate_set
#define _hiwire_deduplicate_delete hiwire_deduplicate_delete
#endif

#ifdef _HIWIRE_EMSCRIPTEN_DEDUPLICATE
#include "emscripten.h"

HwRef _deduplicate_map = NULL;

// clang-format off
EM_JS(__externref_t, __hiwire_deduplicate_new, (void), {
  return new Map();
});

EM_JS(HwRef, __hiwire_deduplicate_get, (__externref_t map, __externref_t value), {
  return map.get(value);
});

EM_JS(void, __hiwire_deduplicate_set, (__externref_t map, __externref_t value, HwRef ref), {
  map.set(value, ref);
});

EM_JS(void, __hiwire_deduplicate_delete, (__externref_t map, __externref_t value), {
  map.delete(value);
});
// clang-format on

static void
deduplicate_init()
{
  _deduplicate_map = hiwire_intern(__hiwire_deduplicate_new());
}

static HwRef
_hiwire_deduplicate_get(__externref_t value)
{
  return __hiwire_deduplicate_get(hiwire_get(_deduplicate_map), value);
}

static void
_hiwire_deduplicate_set(__externref_t value, HwRef ref)
{
  return __hiwire_deduplicate_set(hiwire_get(_deduplicate_map), value, ref);
}

static void
_hiwire_deduplicate_delete(__externref_t value)
{
  return __hiwire_deduplicate_delete(hiwire_get(_deduplicate_map), value);
}

#else
#define deduplicate_init()
#endif

#ifdef _HIWIRE_CAN_DEDUPLICATE
HwRef
_hiwire_deduplicate_get(__externref_t value);
void
_hiwire_deduplicate_set(__externref_t value, HwRef ref);
void
_hiwire_deduplicate_delete(__externref_t value);

HwRef
hiwire_incref_deduplicate(HwRef ref)
{
  __externref_t value = hiwire_get(ref);
  HwRef result = _hiwire_deduplicate_get(value);
  if (!result) {
    // not present, use ref
    result = ref;
    _hiwire_deduplicate_set(value, result);
    if (!IS_IMMORTAL(ref)) {
      // Record that we need to remove this entry from obj_to_key when the
      // reference is freed. (Touching a map is expensive, avoid if possible!)
      _hiwire.slotInfo[HEAP_REF_TO_INDEX(result)] |= DEDUPLICATED_BIT;
    }
  }
  hiwire_incref(result);
  return result;
}

#else

// These are used in hiwire.c, make into no-ops.
#define _hiwire_deduplicate_set(a, b)
#define _hiwire_deduplicate_delete(a)

#endif
