
.globl _hiwire_table
_hiwire_table:
  .tabletype _hiwire_table, externref

.globl _hiwire_immortal_table
_hiwire_immortal_table:
  .tabletype _hiwire_immortal_table, externref


.globl _hiwire_ref_null
_hiwire_ref_null:
  .functype _hiwire_ref_null () -> (externref)
  ref.null_extern
  end_function


.globl _hiwire_immortal_get
_hiwire_immortal_get:
  .functype _hiwire_immortal_get (i32) -> (externref)
  local.get 0
  table.get _hiwire_immortal_table
end_function

.globl _hiwire_immortal_add
_hiwire_immortal_add:
  .functype _hiwire_immortal_add (externref) -> (i32)
  .local i32
  local.get 0
  i32.const 1
  table.grow _hiwire_table
end_function


.globl _hiwire_table_init
_hiwire_table_init:
  .functype _hiwire_table_init () -> ()
  ref.null_extern
  i32.const 1
  table.grow _hiwire_table
  drop
end_function


.globl _hiwire_get
_hiwire_get:
  .functype _hiwire_get (i32) -> (externref)
  local.get 0
  table.get _hiwire_table
end_function

.globl _hiwire_delete
_hiwire_delete: 
  .functype _hiwire_delete (i32) -> ()
  local.get 0
  ref.null_extern
  table.set _hiwire_table
end_function

.globl _hiwire_set
_hiwire_set:
  .functype _hiwire_set (i32, externref) -> (i32)
  .local i32
  block
    local.get 0
    table.size _hiwire_table
    local.tee 2
    i32.ge_s
    br_if 0
    // index less than table size, normal assignment
    local.get 0
    local.get 1
    table.set _hiwire_table
    i32.const 0 // return 0 indicates success
    return
  end_block
  // Otherwise, index should be equal to table size and table is full, append
  // entry to end
  local.get 1
  i32.const 1
  table.grow _hiwire_table
end_function
