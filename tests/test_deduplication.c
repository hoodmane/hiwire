#include "testlib.h"


int main() {
    __externref_t o1 = get_obj(7);
    __externref_t o2 = get_obj(9);
    JsRef id11 = hiwire_new_value(o1);
    ASSERT(!SLOT_DEDUPLICATED(id11));
    JsRef id12 = hiwire_incref_deduplicate(id11);
    ASSERT(SLOT_DEDUPLICATED(id11));
    JsRef id13 = hiwire_new_value(o1);
    JsRef id14 = hiwire_incref_deduplicate(id13);
    ASSERT(id11 == id12);
    ASSERT(id12 == id14);
    ASSERT(HEAP_REF_TO_INDEX(id11) == 1);
    ASSERT(HEAP_REF_TO_INDEX(id13) == 2);
    ASSERT(SLOT_REFCOUNT(id11) == 3);
    ASSERT(SLOT_REFCOUNT(id13) == 1);
    ASSERT(SLOT_DEDUPLICATED(id11));
    ASSERT(!SLOT_DEDUPLICATED(id13));

    JsRef id21 = hiwire_new_value(o2);
    JsRef id22 = hiwire_incref_deduplicate(id21);
    JsRef id23 = hiwire_new_value(o2);
    JsRef id24 = hiwire_incref_deduplicate(id23);
    ASSERT(id21 == id22);
    ASSERT(id22 == id24);
    ASSERT(id22 != id12);

    hiwire_decref(id21);
    ASSERT(!is_null(hiwire_get_value(id21)));
    hiwire_decref(id21);
    ASSERT(!is_null(hiwire_get_value(id21)));
    hiwire_decref(id21);
    ASSERT(is_null(hiwire_get_value(id21)));
    ASSERT(!is_null(hiwire_get_value(id23)));
    JsRef id31 = hiwire_new_value(o2);
    ASSERT(!SLOT_DEDUPLICATED(id31));
    JsRef id32 = hiwire_incref_deduplicate(id31);
    ASSERT(SLOT_DEDUPLICATED(id31));
    JsRef id33 = hiwire_new_value(o2);
    JsRef id34 = hiwire_incref_deduplicate(id33);
    ASSERT(is_null(hiwire_get_value(id21)));
    ASSERT(id31 == id32);
    ASSERT(id32 == id34);
    ASSERT(id21 != id31);
    return 0;
}