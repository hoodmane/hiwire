#include "testlib.h"

void
hiwire_invalid_id(int type, HwRef ref)
{
  printf("type: %d, ref: %d\n", type, ref);
}

int
main()
{
  hiwire_get(NULL);
  HwRef id1 = hiwire_new(int_to_ref(5));
  hiwire_decref(id1);
  hiwire_get(id1);
  hiwire_incref(id1);
  hiwire_decref(id1);

  return 0;
}
