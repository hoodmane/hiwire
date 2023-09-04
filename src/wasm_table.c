// These table builtins do not work with -fPIC. But luckily this file has no
// illegal relocations. So we compile this file with -fno-PIC and the other
// compilation unit with -fPIC and it seems to work okay...

static __externref_t _hiwire_table[0];
static __externref_t _hiwire_immortal_table[0];
typedef unsigned int uint;

__externref_t
_hiwire_immortal_get(uint index)
{
  return __builtin_wasm_table_get(_hiwire_immortal_table, index);
}

uint
_hiwire_immortal_add(__externref_t value)
{
  return __builtin_wasm_table_grow(_hiwire_immortal_table, value, 1);
}

void
_hiwire_table_init(void)
{
  __builtin_wasm_table_grow(_hiwire_table, __builtin_wasm_ref_null_extern(), 1);
}

void
_hiwire_immortal_table_init(void)
{
  __builtin_wasm_table_grow(_hiwire_table, __builtin_wasm_ref_null_extern(), 1);
}

__externref_t
_hiwire_get(uint index)
{
  return __builtin_wasm_table_get(_hiwire_table, index);
}

void
_hiwire_delete(uint index)
{
  __builtin_wasm_table_set(
    _hiwire_table, index, __builtin_wasm_ref_null_extern());
}

uint
_hiwire_set(uint index, __externref_t value)
{
  uint table_size = __builtin_wasm_table_size(_hiwire_table);
  if (index == table_size) {
    return __builtin_wasm_table_grow(_hiwire_table, value, 1);
  }
  if (index > table_size) {
    // assertion error...
    return -1;
  }
  __builtin_wasm_table_set(_hiwire_table, index, value);
  return 0;
}
