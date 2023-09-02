#undef HIWIRE_H
#undef _STDIO_H
#include "hiwire.h"
#include "stdio.h"
#define HIWIRE_INTERNAL
#include "../src/hiwire_macros.h"
#undef HIWIRE_INTERNAL

#define INFO_IS_OCCUPIED(info) ((info)&OCCUPIED_BIT)
#define INFO_REFCOUNT(info) (((info)&REFCOUNT_MASK) >> 2)

#define REF_VERSION(ref) (((unsigned int)(ref)) >> VERSION_SHIFT)

#define SLOT_VERSION(ref) REF_VERSION(_hiwire_slot_info(HEAP_REF_TO_INDEX(ref)))
#define SLOT_OCCUPIED(ref)                                                     \
  INFO_IS_OCCUPIED(_hiwire_slot_info(HEAP_REF_TO_INDEX(ref)))
#define SLOT_REFCOUNT(ref)                                                     \
  INFO_REFCOUNT(_hiwire_slot_info(HEAP_REF_TO_INDEX(ref)))
#define SLOT_DEDUPLICATED(ref)                                                 \
  HEAP_IS_DEDUPLICATED(_hiwire_slot_info(HEAP_REF_TO_INDEX(ref)))

#define PASS_STR "\033[94;1mPASS \033[0m"
#define FAIL_STR "\033[31;1mFAIL!\033[0m"

#define ASSERT_STR(cond, check)                                                \
  do {                                                                         \
    if (cond) {                                                                \
      printf(PASS_STR "   %s\n", check);                                       \
    } else {                                                                   \
      printf(FAIL_STR "   %s\n", check);                                       \
    }                                                                          \
  } while (0)

#define ASSERT(cond) ASSERT_STR(cond, #cond)

int
_hiwire_num_keys(void);
int
_hiwire_slot_info(int);

__externref_t
int_to_ref(int x);
__externref_t
get_obj(int x);
int
ref_to_int(__externref_t x);
int
is_null(__externref_t x);
