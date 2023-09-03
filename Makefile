SHELL := /bin/bash
CFLAGS+= \
	-fPIC \
	-mreference-types \
	-mbulk-memory \
	-Werror=int-conversion \
	-Werror=incompatible-pointer-types \
	$(DBGFLAGS) $(OPTFLAGS)

all: \
	dist/include/hiwire.h	\
	dist/include/_hiwire_config.h	\
	dist/lib/libhiwire.a

build/_hiwire_config.h:
	mkdir -p build
	touch $@
	for A in \
		EMSCRIPTEN_DEDUPLICATE \
		EXTERN_DEDUPLICATE \
		STATIC_PAGES \
		EXTERN_REALLOC \
	; do \
		echo $${!A+#define _HIWIRE_$$A $${!A}} >> $@ ; \
	done


dist/include/hiwire.h: src/hiwire.h
	mkdir -p dist/include
	cp $< $@

dist/include/_hiwire_config.h: build/_hiwire_config.h
	mkdir -p dist/include
	cp $< $@

dist/lib/libhiwire.a: build/hiwire.o build/wasm_table.o
	mkdir -p dist/lib
	if type emar > /dev/null 2>&1 ; then \
		emar rcs $@ $(filter %.o,$^) ; \
	else \
		ar rcs $@ $(filter %.o,$^) ; \
		if type llvm-ranlib > /dev/null 2>&1 ; then \
			llvm-ranlib $@ ; \
		fi \
	fi

build/wasm_table.o: src/wasm_table.c
	mkdir -p build
	$(CC) -o $@ -c $< $(CFLAGS) -Isrc/ -fno-PIC

build/hiwire.o: src/*.c src/*.h build/_hiwire_config.h
	mkdir -p build
	$(CC) -o $@ -c src/hiwire.c $(CFLAGS) -Isrc/ -Ibuild/

clean:
	rm -rf build dist
	rm -rf tests/*.{js,wasm,o}
