#include "testlib.h"

int
main()
{
  hiwire_incref(NULL);
  hiwire_incref(NULL);
  hiwire_incref(NULL);
  hiwire_decref(NULL);
  hiwire_decref(NULL);
  hiwire_decref(NULL);

  ASSERT(_hiwire_num_keys() == 0);
  HwRef id1 = hiwire_intern(int_to_ref(5));
  HwRef id2 = hiwire_intern(int_to_ref(6));
  HwRef id3 = hiwire_intern(int_to_ref(7));
  ASSERT(_hiwire_num_keys() == 0);
  ASSERT(ref_to_int(hiwire_get(id1)) == 5);
  ASSERT(ref_to_int(hiwire_get(id2)) == 6);
  ASSERT(ref_to_int(hiwire_get(id3)) == 7);
  hiwire_decref(id1);
  ASSERT(ref_to_int(hiwire_get(id1)) == 5);
  hiwire_incref(id1);
  ASSERT(ref_to_int(hiwire_get(id1)) == 5);
  hiwire_decref(id1);
  hiwire_decref(id1);
  hiwire_decref(id1);
  ASSERT(ref_to_int(hiwire_get(id1)) == 5);
  ASSERT(ref_to_int(hiwire_get(id2)) == 6);
  ASSERT(ref_to_int(hiwire_get(id3)) == 7);

  return 0;
}
