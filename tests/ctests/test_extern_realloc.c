#include "testlib.h"

HwRef blah[10000];

void*
hiwire_realloc(void* orig, size_t sz)
{
  printf("realloc! %d\n", sz);
  if (sz > sizeof(blah)) {
    return 0;
  }
  return blah;
}

int
main()
{
  int offset = 80000;
  HwRef refs[5000];
  for (int i = 0; i < 5000; i++) {
    refs[i] = hiwire_new(int_to_ref(offset + i));
  }
  for (int i = 0; i < 5000; i++) {
    if (ref_to_int(hiwire_get(refs[i])) != offset + i) {
      printf(FAIL_STR " %d\n", i);
      return 1;
    }
  }
  printf(PASS_STR "\n");
  for (int i = 0; i < 5000; i++) {
    hiwire_decref(refs[i]);
  }
  offset *= 6;
  for (int i = 0; i < 5000; i++) {
    refs[i] = hiwire_new(int_to_ref(offset + 2 * i));
  }
  for (int i = 0; i < 5000; i++) {
    if (ref_to_int(hiwire_get(refs[i])) != offset + 2 * i) {
      printf(FAIL_STR "\n");
      return 1;
    }
  }
  printf(PASS_STR "\n");

  return 0;
}
