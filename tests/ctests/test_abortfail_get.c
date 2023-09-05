#include "testlib.h"

int
main()
{
  HwRef id1 = hiwire_new(int_to_ref(5));
  hiwire_decref(id1);
  hiwire_get(id1);

  return 0;
}
