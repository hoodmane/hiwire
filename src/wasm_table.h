#include "hiwire.h"

typedef unsigned int uint;

__externref_t _hiwire_immortal_get(uint);
uint _hiwire_immortal_add(__externref_t);

void
_hiwire_table_init(void);
__externref_t _hiwire_get(uint);
uint _hiwire_set(uint, __externref_t);
void _hiwire_delete(uint);
