CFLAGS+= -fPIC -mreference-types -Werror=int-conversion -Werror=incompatible-pointer-types -g2
LDFLAGS=-g2


dist/blah.js: \
  src/wasm_table.o \
  src/hiwire.o \
  src/main.o
	$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)
	node dist/blah.js

src/wasm_table.o: src/wasm_table.s
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/


%.o: %.c $(wildcard src/*.h) src/_deduplicate.c
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/

