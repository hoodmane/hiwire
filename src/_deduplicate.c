#ifdef HIWIRE_EXTERN_DEDUPLICATE
#define _hiwire_deduplicate_get hiwire_deduplicate_get
#define _hiwire_deduplicate_set hiwire_deduplicate_set
#define _hiwire_deduplicate_delete hiwire_deduplicate_delete
#endif

#ifdef HIWIRE_EMSCRIPTEN_DEDUPLICATE
#include "emscripten.h"

EM_JS(HwRef, _hiwire_deduplicate_get, (__externref_t value), {
  let result = _hiwire_deduplicate_map.get(value);
  return result;
}
var _hiwire_deduplicate_map = new Map();
);

EM_JS(void,  _hiwire_deduplicate_set, (__externref_t value, HwRef ref), {
  _hiwire_deduplicate_map.set(value, ref);
});

EM_JS(void, _hiwire_deduplicate_delete, (__externref_t value), {
  _hiwire_deduplicate_map.delete(value);
});
#endif

#ifdef CAN_DEDUPLICATE
HwRef hiwire_incref_deduplicate(HwRef ref) {
  __externref_t value = hiwire_get(ref);
  HwRef result = _hiwire_deduplicate_get(value);
  if (!result) {
    // not present, use ref
    result = ref;
    _hiwire_deduplicate_set(value, result);
    // Record that we need to remove this entry from obj_to_key when the
    // reference is freed. (Touching a map is expensive, avoid if possible!)
    _hiwire.slotInfo[HEAP_REF_TO_INDEX(result)] |= DEDUPLICATED_BIT;
  }
  hiwire_incref(result);
  return result;
}
#endif
