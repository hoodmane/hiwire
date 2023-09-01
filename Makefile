OPTFLAGS ?= -O2
DBGFLAGS?=-g3
CFLAGS+= -fPIC -mreference-types -Werror=int-conversion -Werror=incompatible-pointer-types $(DBGFLAGS) $(OPTFLAGS)
ifdef HIWIRE_STATIC_PAGES
	# Passing this when compiling .s leads to a warning...
	NOT_S_FLAGS = -DHIWIRE_STATIC_PAGES=$(HIWIRE_STATIC_PAGES)
endif

LDFLAGS=$(DBGFLAGS) $(OPTFLAGS)

all: \
	dist/include/hiwire.h	\
	dist/lib/libhiwire.a

dist/include/hiwire.h: src/hiwire.h
	mkdir -p dist/include
	cp $< $@

dist/lib/libhiwire.a: \
  src/wasm_table.o \
  src/hiwire.o
	mkdir -p dist/lib
	$(AR) rcs $@ $(filter %.o,$^)

src/wasm_table.o: src/wasm_table.c
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/ -fno-PIC

%.o: %.c $(wildcard src/*.h) src/_deduplicate.c
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/ $(NOT_S_FLAGS)

test-emcc: \
	dist/include/hiwire.h \
	dist/lib/libhiwire.a

	emcc -I dist/include -c tests/testlib.c -o tests/testlib.o -mreference-types -fPIC

	emcc -I dist/include -c tests/test_basic.c -o tests/test_basic.o -mreference-types -fPIC
	emcc -L dist/lib -lhiwire tests/test_basic.o tests/testlib.o -o tests/test_basic.js -sMAIN_MODULE
	node tests/test_basic.js

	# emcc -I dist/include -c tests/test_many_refs.c -o tests/test_many_refs.o -mreference-types -fPIC
	# emcc -L dist/lib -lhiwire tests/test_many_refs.o tests/testlib.o -o tests/test_many_refs.js -sMAIN_MODULE
	# node tests/test_many_refs.js

	emcc -DHIWIRE_EMSCRIPTEN_DEDUPLICATE -I dist/include -c tests/test_deduplication.c -o tests/test_deduplication.o -mreference-types -fPIC
	emcc -L dist/lib -lhiwire tests/test_deduplication.o tests/testlib.o -o tests/test_deduplication.js -sMAIN_MODULE
	node tests/test_deduplication.js

	emcc -I dist/include -c tests/test_versions.c -o tests/test_versions.o -mreference-types -fPIC
	emcc -L dist/lib -lhiwire tests/test_versions.o tests/testlib.o -o tests/test_versions.js -sMAIN_MODULE
	node tests/test_versions.js



clean:
	rm -f src/*.o
	rm -rf dist
