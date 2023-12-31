#include "hiwire.h"
#include "stdio.h"

__attribute__((import_module("env"),
               import_name("print_value"))) void print_value(__externref_t);

__attribute__((import_module("env"), import_name("js_value"))) __externref_t
js_value(void);

int
main()
{
  __externref_t f = js_value();
  print_value(f);
  HwRef res1 = hiwire_new(f);
  printf("res: %d\n", (int)res1);
  print_value(hiwire_get(res1));
  HwRef res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  res2 = hiwire_new(f);
  printf("res2: %d\n", (int)res2);
  // HwRef r1 = hiwire_incref_deduplicate(res1);
  // HwRef r2 = hiwire_incref_deduplicate(res2);
  // printf("r1: %d r2: %d\n", (int)r1, (int)r2);
}
