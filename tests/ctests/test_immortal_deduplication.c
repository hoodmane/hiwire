#include "testlib.h"

int
main()
{
  __externref_t o1 = get_obj(7);
  HwRef id1 = hiwire_new(o1);
  ASSERT(!SLOT_DEDUPLICATED(id1));
  HwRef id2 = hiwire_incref_deduplicate(id1);
  ASSERT(SLOT_DEDUPLICATED(id1));
  ASSERT(id1 == id2);
  HwRef id_immortal = hiwire_intern(o1);
  HwRef id3 = hiwire_incref_deduplicate(id_immortal);
  ASSERT(id3 == id1);
  ASSERT(SLOT_REFCOUNT(id1) == 3);

  hiwire_decref(id1);
  hiwire_decref(id1);
  hiwire_decref(id1);

  HwRef id4 = hiwire_incref_deduplicate(id_immortal);
  ASSERT(id4 == id_immortal);

  HwRef id5 = hiwire_new(o1);
  HwRef id6 = hiwire_incref_deduplicate(id5);
  ASSERT(!SLOT_DEDUPLICATED(id5));
  ASSERT(id6 == id_immortal);
  ASSERT(SLOT_REFCOUNT(id5) == 1);

  return 0;
}
