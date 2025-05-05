#include "testlib.h"

int
main()
{
  __externref_t o1 = get_obj(7);
  __externref_t o2 = get_obj(9);
  HwRef id11 = hiwire_new(o1);
  ASSERT(!SLOT_DEDUPLICATED(id11));
  HwRef id12 = hiwire_new_deduplicate(o1);
  ASSERT(SLOT_DEDUPLICATED(id12));
  HwRef id13 = hiwire_new(o1);
  HwRef id14 = hiwire_new_deduplicate(o1);
  ASSERT(id11 != id12);
  ASSERT(id12 == id14);
  ASSERT(HEAP_REF_TO_INDEX(id11) == 1);
  ASSERT(HEAP_REF_TO_INDEX(id12) == 2);
  ASSERT(HEAP_REF_TO_INDEX(id13) == 3);
  ASSERT(HEAP_REF_TO_INDEX(id14) == 2);
  ASSERT(SLOT_REFCOUNT(id11) == 1);
  ASSERT(SLOT_REFCOUNT(id12) == 2);
  ASSERT(SLOT_REFCOUNT(id13) == 1);
  ASSERT(!SLOT_DEDUPLICATED(id11));
  ASSERT(SLOT_DEDUPLICATED(id12));
  ASSERT(!SLOT_DEDUPLICATED(id13));

  HwRef id21 = hiwire_new(o2);
  HwRef id22 = hiwire_new_deduplicate(o2);
  HwRef id23 = hiwire_new(o2);
  HwRef id24 = hiwire_new_deduplicate(o2);
  ASSERT(id21 != id22);
  ASSERT(id22 == id24);
  ASSERT(id22 != id12);

  hiwire_decref(id21);
  ASSERT(is_null(hiwire_get(id21)));
  hiwire_decref(id22);
  ASSERT(!is_null(hiwire_get(id22)));
  hiwire_decref(id22);
  ASSERT(is_null(hiwire_get(id22)));
  ASSERT(!is_null(hiwire_get(id23)));
  HwRef id31 = hiwire_new(o2);
  ASSERT(!SLOT_DEDUPLICATED(id31));
  HwRef id32 = hiwire_new_deduplicate(o2);
  ASSERT(SLOT_DEDUPLICATED(id32));
  ASSERT(id31 != id22);
  HwRef id33 = hiwire_new(o2);
  HwRef id34 = hiwire_new_deduplicate(o2);
  ASSERT(is_null(hiwire_get(id21)));
  ASSERT(id31 != id32);
  ASSERT(id32 == id34);
  ASSERT(id21 != id31);

  return 0;
}
