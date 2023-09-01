#include "testlib.h"


int main() {
    HwRef id1 = hiwire_new(int_to_ref(7));
    ASSERT(HEAP_REF_TO_INDEX(id1) == 1);
    ASSERT(REF_VERSION(id1) == 0);

    ASSERT(SLOT_VERSION(id1) == 0);
    ASSERT(SLOT_REFCOUNT(id1) == 1);
    ASSERT(SLOT_OCCUPIED(id1));
    ASSERT(ref_to_int(hiwire_get(id1)) == 7);

    hiwire_decref(id1);
    ASSERT(SLOT_VERSION(id1) == 1);
    ASSERT(!SLOT_OCCUPIED(id1));
    ASSERT(is_null(hiwire_get(id1)));


    HwRef id2 = hiwire_new(int_to_ref(8));
    ASSERT(HEAP_REF_TO_INDEX(id2) == 1);
    ASSERT(REF_VERSION(id2) == 1);

    ASSERT(SLOT_VERSION(id2) == 1);
    ASSERT(SLOT_REFCOUNT(id2) == 1);
    ASSERT(SLOT_OCCUPIED(id2));
    ASSERT(ref_to_int(hiwire_get(id2)) == 8);
    ASSERT(is_null(hiwire_get(id1)));
    hiwire_decref(id2);

    HwRef id3 = hiwire_new(int_to_ref(123));
    hiwire_decref(id3);

    for (int i = 0; i < 49; i++) {
      hiwire_decref(hiwire_new(int_to_ref(i + 10)));
    }
    HwRef id4 = hiwire_new(int_to_ref(49 + 10));
    ASSERT(HEAP_REF_TO_INDEX(id3) == 1);
    ASSERT(REF_VERSION(id4) == 52);
    ASSERT(SLOT_VERSION(id4) == 52);
    ASSERT(SLOT_REFCOUNT(id4) == 1);
    ASSERT(SLOT_OCCUPIED(id4));
    ASSERT(ref_to_int(hiwire_get(id4)) == 59);
    ASSERT(is_null(hiwire_get(id3)));
    ASSERT(is_null(hiwire_get(id2)));
    ASSERT(is_null(hiwire_get(id1)));
    hiwire_decref(id4);

    for (int i = 0; i < 13; i++) {
      hiwire_decref(hiwire_new(int_to_ref(i + 60)));
    }
    HwRef id_dup = hiwire_new(int_to_ref(678));

    ASSERT(HEAP_REF_TO_INDEX(id_dup) == 1);
    ASSERT(REF_VERSION(id_dup) == 2);
    ASSERT(SLOT_VERSION(id_dup) == 2);
    ASSERT(SLOT_REFCOUNT(id_dup) == 1);
    ASSERT(SLOT_OCCUPIED(id_dup));

    ASSERT(ref_to_int(hiwire_get(id3)) == 678);
    return 0;
}
