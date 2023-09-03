# Hiwire: Wasm Reference Type helpers for C/C++

WebAssembly `externref` values are very exciting: they allow us to pass host
references around in WebAssembly efficiently without doing any complicated dance
on the host. However, `externref` values cannot be stored in linear memory, so
they cannot have addresses and cannot be struct fields. They can appear as local
variables and as arguments and return values and be stored as indices in tables.
Since so much of the C programming language is built around a linear memory
abstraction this limits what can be done with them.

In situations where an address is needed, the `externref` can be stored into a
table and the table index (which is just a number) can be used as a reference.
This number can live in linear memory, can be a struct field, can have an
address, and generally plays well with C language features.

The purpose of this library is to do reference counted table management. The
design is inspired by the Rust
[slotmap](https://github.com/orlp/slotmap/blob/master/src/basic.rs) project. To
quote [the slotmap description](https://docs.rs/slotmap/latest/slotmap/):

> This library provides a container with persistent unique keys to access stored
> values. Upon insertion a key is returned that can be used to later access or
> remove the values. Insertion, removal and access all take O(1) time with low
> overhead.


## API

An `externref` can be placed into the table with `hiwire_new` which
returns a `HwRef``:
```C
HwRef hiwire_new(__externref_t value);
```
The resulting reference can be converted back to an `externref` with
`hiwire_get`:
```C
__externref_t hiwire_get(HwRef ref);
```

The reference count can be adjusted with `hiwire_incref` and `hiwire_decref`:
```C
void hiwire_incref(HwRef ref);
void hiwire_decref(HwRef ref);
```
if the reference count reaches zero, the value is removed from the map, which
allows the host to garbage collect it. The reference becomes invalid.

`NULL` is a distinct `HwRef`. Calling `hiwire_get(NULL)` will [TODO what will it do?]
`hiwire_incref` and `hiwire_decref` both are no-ops on `NULL`.

There are also immortal references which can be created with `hiwire_intern`.
These are never released and `hiwire_incref` and `hiwire_decref` are no-ops.
```C
HwRef hiwire_intern(__externref_t value)
```

`hiwire_pop` is a convenience method which first uses `hiwire_get` and
then `hiwire_decref`.
```C
__externref_t hiwire_pop(HwRef ref);
```

Finally, there is `hiwire_incref_deduplicate`:
```C
HwRef hiwire_incref_deduplicate(HwRef ref)
```
the purpose of this method is to ensure that the returned `HwRef`s will be equal
if the references point to the same host object. This allows you to use the
resulting `HwRef` as C map keys and it will index on host object equality.

To use `hiwire_incref_deduplicate` you need to define either
`EMSCRIPTEN_DEDUPLICATE` or `EXTERN_DEDUPLICATE`, see
compatibility. With `DEDUPLICATE` you can choose the notion of
host object equality that is used, with `EMSCRIPTEN_DEDUPLICATE`, two
externrefs are equal if the pointed-to values are `===` to each other.

Note that with `EMSCRIPTEN_DEDUPLICATE`, `hiwire_incref_deduplicate` is ~100x
slower than the other APIs here and makes freeing the reference also 100x
slower.


## Limits

You can have at most 2^25 = 33,554,432 keys at once. Each key has a maximum
reference count of 2^24 = 16,777,216.


## Compiler requirements

Requires either Emscripten >= 3.1.42 or clang more recent than June 10th 2023.
Specifically, we need clang to include [this
commit](https://github.com/llvm/llvm-project/commit/55aeb23fe0084d930ecd7335092d712bd71694c7).
As of this writing, no stable clang has been released since June 10th, but
17.0.0-rc3 is a candidate.

It's possible to install clang-17 from apt on debian. You need to install the
packages `clang-17`, `lld-17`, and `libclang-rt-17-dev-wasm32`. Annoyingly, the
archive `libclang_rt.builtins-wasm32.a` that Debian provides is missing an index
but `wasm-ld` refuses to link archives without indexes. To add an index, run
```sh
llvm-ranlib /usr/lib/llvm-17/lib/clang/17/lib/wasi/libclang_rt.builtins-wasm32.a
```
See the instructions [here](https://apt.llvm.org/) or the
[Dockerfile](./Dockerfile).

To use wasi-libc with clang-17 you either need tip of tree unreleased wasi-libc
or to apply [this patch](patches/wasi-libc-clang-17-compat.patch) to wasi-libc
version 20.

## Compiler flags

To use `__externref_t` you must compile with `-mreference-types`. I also
recommend compiling with adding `-Werror=int-conversion` and
`-Werror=incompatible-pointer-types` to prevent implicit casts between `HwRef`
and other types.


## `wasm32-unknown-unknown` Compatibility

By default, `hiwire` uses `realloc` from the standard library. For
`wasm32-unknown-unknown` there are three options:

1. Define `STATIC_PAGES=n`. This statically allocates the memory that
   hiwire needs but limits you to having a maximum of n*1024 distinct references
   in total. If you exceed this number, `hiwire_new` will fail by returning
   `NULL`.
2. Provide an implementation of `realloc` (say by linking a malloc library)
3. Define `EXTERN_REALLOC` and provide an implementation of
   `hiwire_realloc`.

You can also use `STATIC_PAGES=n` with `wasm32-emscripten` or `wasm32-wasi`, but
the default behavior will just use `realloc`

`hiwire_incref_deduplicate` also needs extra runtime support. You can define
`HIWIRE_EMSCRIPTEN_DEDUPLICATE` (Emscripten only) which will use `EM_JS`
functions to provide the functionality needed. Otherwise, you can define
`HIWIRE_EXTERN_DEDUPLICATE` and then you will need to define three host
functions: `hiwire_deduplicate_get`, `hiwire_deduplicate_set`, and
`hiwire_deduplicate_delete`. These have the following signatures and should act
like a map from `externref` to integers:
```C
HwRef hiwire_deduplicate_get(__externref_t value);
void hiwire_deduplicate_set(__externref_t value, HwRef ref);
void hiwire_deduplicate_delete(__externref_t value);
```


## Runtime requirements

Requires the WebAssembly runtime to support `externref`. Support for `externref`
is universal among recent WebAssembly runtimes:
* Chrome >= 96 (Released November 16th, 2021)
* Firefox >= 79 (Released June 1st, 2020)
* Safari >= 15.0 (Released September 20th, 2021)
* Node >= 18.0 (Released April 19th, 2022) (also supported in Node 16 with the
  `--experimental-wasm-reftypes` flag)


### Related Projects

Loosely based on the Rust [slotmap package](https://docs.rs/slotmap/latest/slotmap/).
Made for use with Pyodide.
