#include "hiwire.h"
#include "emscripten.h"
#include <stdio.h>

EM_JS(__externref_t, js_value, (void), {
    return {a: 7, b: 5};
})

EM_JS(void, print_value, (__externref_t o), {
    console.log(o);
})

int main() {
    __externref_t f = js_value();
    print_value(f);
    JsRef res1 = hiwire_new_value(f);
    printf("res: %d\n", (int)res1);
    print_value(hiwire_get_value(res1));
    JsRef res2 = hiwire_new_value(f);
    JsRef r1 = hiwire_incref_deduplicate(res1);
    JsRef r2 = hiwire_incref_deduplicate(res2);
    printf("r1: %d r2: %d\n", (int)r1, (int)r2);
}
