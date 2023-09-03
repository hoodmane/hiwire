#include "testlib.h"

#if defined(__wasi__)
int
extern_printf(const char*, ...)
  __attribute__((import_module("env"), import_name("printf")));

#define printf extern_printf
#endif

int
main()
{
  ASSERT(_hiwire_num_keys() == 0);
  HwRef id1 = hiwire_new(int_to_ref(5));
  ASSERT(_hiwire_num_keys() == 1);

  HwRef id2 = hiwire_new(int_to_ref(7));
  ASSERT(_hiwire_num_keys() == 2);
  HwRef id3 = hiwire_new(int_to_ref(10));
  ASSERT(_hiwire_num_keys() == 3);
  HwRef id4 = hiwire_new(int_to_ref(12));
  ASSERT(_hiwire_num_keys() == 4);

  ASSERT(ref_to_int(hiwire_get(id1)) == 5);
  ASSERT(ref_to_int(hiwire_get(id2)) == 7);
  ASSERT(ref_to_int(hiwire_get(id3)) == 10);
  ASSERT(ref_to_int(hiwire_get(id4)) == 12);

  hiwire_decref(id1);
  ASSERT(_hiwire_num_keys() == 3);
  ASSERT(is_null(hiwire_get(id1)));
  ASSERT(ref_to_int(hiwire_get(id2)) == 7);
  ASSERT(ref_to_int(hiwire_get(id3)) == 10);
  ASSERT(ref_to_int(hiwire_get(id4)) == 12);
  hiwire_decref(id3);
  ASSERT(is_null(hiwire_get(id1)));
  ASSERT(ref_to_int(hiwire_get(id2)) == 7);
  ASSERT(is_null(hiwire_get(id3)));
  ASSERT(ref_to_int(hiwire_get(id4)) == 12);
  ASSERT(_hiwire_num_keys() == 2);
  hiwire_decref(id2);
  ASSERT(_hiwire_num_keys() == 1);
  hiwire_decref(id4);
  ASSERT(_hiwire_num_keys() == 0);
  ASSERT(is_null(hiwire_get(id1)));
  ASSERT(is_null(hiwire_get(id2)));
  ASSERT(is_null(hiwire_get(id3)));
  ASSERT(is_null(hiwire_get(id4)));

  return 0;
}
