OPTFLAGS ?= -O2
CFLAGS+= -fPIC -mreference-types -Werror=int-conversion -Werror=incompatible-pointer-types $(DBGFLAGS) $(OPTFLAGS)

LDFLAGS=$(DBGFLAGS) $(OPTFLAGS)

all: \
	dist/include/hiwire.h	\
	dist/lib/libhiwire.a

dist/include/hiwire.h: src/hiwire.h
	mkdir -p dist/include
	cp $< $@

dist/lib/libhiwire.a: src/hiwire.o src/wasm_table.o
	mkdir -p dist/lib
	if type emar > /dev/null ; then \
		emar rcs $@ $(filter %.o,$^) ; \
	else \
		ar rcs $@ $(filter %.o,$^) ; \
		if type llvm-ranlib  > /dev/null ; then \
			llvm-ranlib $@ ; \
		fi \
	fi

-src/wasm_table.o: src/wasm_table.c
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/ -fno-PIC



src/hiwire.o: src/*.c src/*.h
	$(CC) -c src/hiwire.c -o src/hiwire.o $(CFLAGS) -Isrc/

%.o: %.c $(wildcard src/*.h) src/_deduplicate.c
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/ $(NOT_S_FLAGS)

clean:
	rm -f src/*.o
	rm -rf dist
	rm -rf tests/*.{js,wasm,o}
