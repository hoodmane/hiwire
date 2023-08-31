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


clean:
	rm -f src/*.o
	rm -rf dist
