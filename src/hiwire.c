#include "hiwire.h"

#define ALLOC_INCREMENT 1024

typedef unsigned int uint;

struct _hiwire_data_t
{
  uint freeHead;
  uint* slotInfo;
  uint slotInfoSize;
  uint numKeys;
  uint immortalInitialized;
};

static struct _hiwire_data_t _hiwire = {
  .freeHead = 1,
  .slotInfo = 0,
  .numKeys = 0,
  .slotInfoSize = 0,
  .immortalInitialized = 0,
};

#include "hiwire_macros.h"
#include "wasm_table.h"

#include "_deduplicate.c"
#include "compat.c"

HwRef
hiwire_intern(__externref_t value)
{
  if (!_hiwire.immortalInitialized) {
    _hiwire.immortalInitialized = 1;
    _hiwire_immortal_add(__builtin_wasm_ref_null_extern());
  }
  uint index = _hiwire_immortal_add(value);
  if (index == -1) {
    // TODO: fatal?
    return NULL;
  }
  return IMMORTAL_INDEX_TO_REF(index);
}

HwRef
hiwire_new(__externref_t value)
{
  uint index = _hiwire.freeHead;
  if (_hiwire.slotInfoSize == 0) {
    _hiwire_table_init();
    deduplicate_init();
  }
  uint needed_size = sizeof(uint[index + 1]);
  uint orig_size = sizeof(uint[_hiwire.slotInfoSize]);
  if (needed_size > orig_size) {
    if (index > MAX_INDEX) {
      // TODO: fatal?
      return NULL;
    }
    uint new_size = sizeof(uint[_hiwire.slotInfoSize + ALLOC_INCREMENT]);
    uint* newSlotInfo = _hiwire_realloc(_hiwire.slotInfo, new_size);
    if (!newSlotInfo) {
      // TODO: fatal?
      return NULL;
    }
    hiwire_memset(((char*)newSlotInfo) + orig_size, 0, new_size - orig_size);
    _hiwire.slotInfoSize += ALLOC_INCREMENT;
    _hiwire.slotInfo = newSlotInfo;
  }
  _hiwire.numKeys++;
  uint info = _hiwire.slotInfo[index];
  _hiwire.freeHead = HEAP_INFO_TO_NEXTFREE(info);
  if (_hiwire.freeHead == 0) {
    _hiwire.freeHead = _hiwire.numKeys + 1;
  }
  _hiwire.slotInfo[index] = HEAP_NEW_OCCUPIED_INFO(info);
  if (_hiwire_set(index, value) == -1) {
    // TODO: operation failed...
    return NULL;
  }
  HwRef ref = HEAP_NEW_REF(index, info);
  TRACEREFS("new", ref);
  return ref;
}

// for testing purposes.
int
hiwire_num_refs(void)
{
  return _hiwire.numKeys;
}

uint
_hiwire_slot_info(uint index)
{
  return _hiwire.slotInfo[index];
};

__externref_t
hiwire_get(HwRef ref)
{
  if (!ref) {
    hiwire_invalid_id(HIWIRE_FAIL_GET, ref);
    return __builtin_wasm_ref_null_extern();
  }
  if (IS_IMMORTAL(ref)) {
    return _hiwire_immortal_get(IMMORTAL_REF_TO_INDEX(ref));
  }
  uint index = HEAP_REF_TO_INDEX(ref);
  uint info = _hiwire.slotInfo[index];
  if (HEAP_REF_IS_OUT_OF_DATE(ref, info)) {
    hiwire_invalid_id(HIWIRE_FAIL_GET, ref);
    return __builtin_wasm_ref_null_extern();
  }
  return _hiwire_get(index);
};

void
hiwire_incref(HwRef ref)
{
  if (IS_IMMORTAL(ref)) {
    return;
  }
  // heap reference
  TRACEREFS("incref", ref);
  uint index = HEAP_REF_TO_INDEX(ref);
  uint info = _hiwire.slotInfo[index];
  if (HEAP_REF_IS_OUT_OF_DATE(ref, info)) {
    hiwire_invalid_id(HIWIRE_FAIL_INCREF, ref);
    return;
  }
  HEAP_INCREF(_hiwire.slotInfo[index]);
  return;
};

void
hiwire_decref(HwRef ref)
{
  if (IS_IMMORTAL(ref)) {
    return;
  }
  TRACEREFS("decref", ref);
  uint index = HEAP_REF_TO_INDEX(ref);
  uint info = _hiwire.slotInfo[index];
  if (HEAP_REF_IS_OUT_OF_DATE(ref, info)) {
    hiwire_invalid_id(HIWIRE_FAIL_DECREF, ref);
    return;
  }
  HEAP_DECREF(info);
  if (HEAP_IS_REFCNT_ZERO(info)) {
    if (HEAP_IS_DEDUPLICATED(info)) {
      _hiwire_deduplicate_delete(_hiwire_get(index));
    }
    _hiwire_delete(index);
    _hiwire.numKeys--;
    info = FREE_LIST_INFO(info, _hiwire.freeHead);
    _hiwire.freeHead = index;
  }
  _hiwire.slotInfo[index] = info;
}

__externref_t
hiwire_pop(HwRef ref)
{
  __externref_t value = hiwire_get(ref);
  hiwire_decref(ref);
  return value;
}
