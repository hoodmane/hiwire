#undef HIWIRE_H
#undef _STDIO_H
#include "hiwire.h"

#define HIWIRE_INTERNAL
#include "hiwire_macros.h"
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
_hiwire_slot_info(int);

__externref_t
int_to_ref(int x)
  __attribute__((import_module("env"), import_name("int_to_ref")));

__externref_t
get_obj(int x) __attribute__((import_module("env"), import_name("get_obj")));

int
ref_to_int(__externref_t x)
  __attribute__((import_module("env"), import_name("ref_to_int")));

int
is_null(__externref_t x)
  __attribute__((import_module("env"), import_name("is_null")));

#ifdef _HIWIRE_EXTERN_DEDUPLICATE

HwRef
_hiwire_deduplicate_get(__externref_t value)
  __attribute__((import_module("env"), import_name("_hiwire_deduplicate_get")));

void
_hiwire_deduplicate_set(__externref_t value, HwRef ref)
  __attribute__((import_module("env"), import_name("_hiwire_deduplicate_set")));

void
_hiwire_deduplicate_delete(__externref_t value)
  __attribute__((import_module("env"),
                 import_name("_hiwire_deduplicate_delete")));

#endif

#ifdef _HIWIRE_EXTERN_TRACEREFS
void
extern_hiwire_traceref(char* type,
                       HwRef ref,
                       int index,
                       __externref_t value,
                       int refcount)
  __attribute__((import_module("env"), import_name("hiwire_traceref")));
#endif

#if __has_include("stdio.h")
#include "stdio.h"
#else
int
printf(const char*, ...)
  __attribute__((import_module("env"), import_name("printf")));
#define NULL (void*)(0)
#endif

typedef unsigned long size_t;
