#include "hiwire.h"
#include "stdalign.h"

#define CAN_DEDUPLICATE _HIWIRE_CAN_DEDUPLICATE
#define ALLOC_INCREMENT 1024

#include "compat.c"


_Static_assert(alignof(HwRef) == alignof(int),
               "HwRef should have the same alignment as int.");
_Static_assert(sizeof(HwRef) == sizeof(int),
               "HwRef should have the same size as int.");

#define HIWIRE_INTERNAL
#include "hiwire_macros.h"
#include "wasm_table.h"

#define TRACEREFS(...)
#define FAIL_INVALID_ID(ref) // printf("Fail!!\n")



struct _hiwire_data_t {
  int freeHead;
  int* slotInfo;
  int slotInfoSize;
  int numKeys;
};

static struct _hiwire_data_t _hiwire = {
  .freeHead = 1,
  .slotInfo = 0,
  .numKeys = 0,
  .slotInfoSize = 0,
};

#include "_deduplicate.c"

HwRef
hiwire_intern(__externref_t value) {
  int index = _hiwire_immortal_add(value);
  if (index == -1) {
    // TODO: operation failed..
    return NULL;
  }
  HwRef result = IMMORTAL_INDEX_TO_REF(index);
  #if CAN_DEDUPLICATE
    _hiwire_deduplicate_set(value, result);
  #endif
  return result;
}
#include "stdio.h"

HwRef
hiwire_new_value(__externref_t value) {
  int index = _hiwire.freeHead;
  if (_hiwire.slotInfoSize == 0) {
    _hiwire_table_init();
  }
  int needed_size = sizeof(int[index + 1]);
  int orig_size = sizeof(int[_hiwire.slotInfoSize]);
  if (needed_size > orig_size) {
    int new_size = sizeof(int[_hiwire.slotInfoSize + ALLOC_INCREMENT]);
    int* newSlotInfo = _hiwire_realloc(_hiwire.slotInfo, new_size);
    if (!newSlotInfo) {
      // TODO: fatal?
      return NULL;
    }
    memset(((char*)newSlotInfo) + orig_size, 0, new_size - orig_size);
    _hiwire.slotInfoSize += ALLOC_INCREMENT;
    _hiwire.slotInfo = newSlotInfo;
  }
  _hiwire.numKeys ++;
  int info = _hiwire.slotInfo[index];
  _hiwire.freeHead = HEAP_INFO_TO_NEXTFREE(info); 
  if(_hiwire.freeHead == 0) {
    _hiwire.freeHead = _hiwire.numKeys + 1;
  }
  _hiwire.slotInfo[index] = HEAP_NEW_OCCUPIED_INFO(info);
  if (_hiwire_set(index, value) == -1) {
    // TODO: operation failed...
    return NULL;
  }
  return HEAP_NEW_REF(index, info);
}

// for testing purposes.
int 
_hiwire_num_keys(void) {
  return _hiwire.numKeys;
}

int
_hiwire_slot_info(int index) {
  return _hiwire.slotInfo[index];
};

__externref_t 
hiwire_get_value(HwRef ref) {
  if (!ref) {
    FAIL_INVALID_ID(ref);
    return __builtin_wasm_ref_null_extern();
  }
  if (IS_IMMORTAL(ref)) {
    return _hiwire_immortal_get(IMMORTAL_REF_TO_INDEX(ref));
  }
  int index = HEAP_REF_TO_INDEX(ref);
  int info = _hiwire.slotInfo[index];
  if (HEAP_REF_IS_OUT_OF_DATE(ref, info)) {
    FAIL_INVALID_ID(ref);
    return __builtin_wasm_ref_null_extern();
  }
  return _hiwire_get(index);
};

void
hiwire_incref (HwRef ref) {
  if (IS_IMMORTAL(ref)) {
    return;
  }
  // heap reference
  int index = HEAP_REF_TO_INDEX(ref);
  TRACEREFS("hw.incref", index, ref, _hiwire.objects[index]);
  int info = _hiwire.slotInfo[index];
  if (HEAP_REF_IS_OUT_OF_DATE(ref, info)) {
    FAIL_INVALID_ID(ref);
    return;
  }
  HEAP_INCREF(_hiwire.slotInfo[index]);
  return;
};

void hiwire_decref(HwRef ref) {
  if (IS_IMMORTAL(ref)) {
    return;
  }
  int index = HEAP_REF_TO_INDEX(ref);
  int info = _hiwire.slotInfo[index];
  if (HEAP_REF_IS_OUT_OF_DATE(ref, info)) {
    FAIL_INVALID_ID(ref);
    return;
  }
  HEAP_DECREF(info);
  if (HEAP_IS_REFCNT_ZERO(info)) {
#if CAN_DEDUPLICATE
    if (HEAP_IS_DEDUPLICATED(info)) {
      _hiwire_deduplicate_delete(_hiwire_get(index));
    }
#endif
    _hiwire_delete(index);
    _hiwire.numKeys--;
    info = FREE_LIST_INFO(info, _hiwire.freeHead);
    _hiwire.freeHead = index;
  }
  _hiwire.slotInfo[index] = info; 
}

__externref_t
hiwire_pop (HwRef ref) {
    __externref_t value = hiwire_get_value(ref);
    hiwire_decref(ref);
    return value;
}
