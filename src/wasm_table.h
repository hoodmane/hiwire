#ifndef HIWIRE_INTERNAL
#error "private header"
#endif
#include "hiwire.h"

__externref_t
_hiwire_immortal_get(int);
int _hiwire_immortal_add(__externref_t);

void
_hiwire_table_init(void);
__externref_t
_hiwire_get(int);
int
_hiwire_set(int, __externref_t);
void
_hiwire_delete(int);
