# Hiwire: Wasm Reference Type support for C/C++

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
`hiwire_pop` is a convenience method which first uses `hiwire_get` and
then `hiwire_decref`.
```C
__externref_t hiwire_get(HwRef ref);
```
There are also immortal references which can be created with `hiwire_intern`.
These are never released and `hiwire_incref` and `hiwire_decref` are no-ops.
```C
HwRef hiwire_intern(__externref_t value)
```



## Compiler flags

To use `__externref_t` you must compile with `-mreference-types`. I also
recommend compiling with adding `-Werror=int-conversion` and
`-Werror=incompatible-pointer-types` to prevent implicit casts between `HwRef`
and other types.

## Compiler requirements

Requires either Emscripten >= 3.1.42 or clang more recent than June 10th 2023.
As of this writing, no stable clang has been released since June 10th, but
17.0.0-rc1 is a candidate. Also, to be able to use this you need to compile with
`-mreference-types`.

## Runtime requirements

Requires the WebAssembly runtime to support `externref`. Support for `externref`
is universal among recent WebAssembly runtimes:
* Chrome >= 96 (Released November 16th, 2021)
* Firefox >= 79 (Released June 1st, 2020)
* Safari >= 15.0 (Released September 20th, 2021)
* Node >= 18.0 (Released April 19th, 2022) (also supported in Node 16 with the
  `--experimental-wasm-reftypes` flag)


## Design

Each reference contains an index and a 6 bit version so that when indices are


### Related Projects

In


The following is a similar project for Rust: https://github.com/slowli/externref

