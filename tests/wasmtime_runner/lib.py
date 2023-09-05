import sys
from functools import partial
from struct import unpack

from wasmtime import (
    FuncType,
    ValType,
)

sigmap = {
    "e": ValType.externref(),
    "i": ValType.i32(),
}


def sig_to_functype(sig):
    if sig[0] == "v":
        restype = []
    else:
        restype = [sigmap[sig[0]]]
    argtype = [sigmap[c] for c in sig[1:]]
    return FuncType(argtype, restype)


def signature(sig, wasi=True):
    def dec(fn):
        fn._wasm_signature = sig_to_functype(sig)
        fn._wasi = wasi
        return fn

    return dec


def format_types(fmt: str):
    idx = 0
    while True:
        idx = fmt.find("%", idx)
        if idx == -1:
            return
        idx += 1
        if fmt[idx] == "%":
            continue
        yield fmt[idx]


class WasmLib:
    def __init__(self, stdout, is_wasi):
        self.stdout = stdout if stdout else sys.stdout
        self.d = {}
        self.is_wasi = is_wasi

    def set_store(self, store):
        self.store = store

    def set_memory(self, memory):
        self.memory = memory

    def set_exports(self, exports):
        if not self.is_wasi:
            return
        self.print_str = partial(exports["print_str"], self.store)
        self.malloc = partial(exports["malloc"], self.store)
        self.free = partial(exports["free"], self.store)

    def define_lib(self, linker):
        for k in type(self).__dict__:
            v = getattr(self, k)
            if not hasattr(v, "_wasm_signature"):
                continue
            if self.is_wasi and not v._wasi:
                continue
            linker.define_func("env", v.__name__, v._wasm_signature, v)

    def print(self, *args, end="\n", sep=" "):
        if self.is_wasi:
            # We're in wasi, route the string through wasi's print function so
            # that they interleave correctly with calls to printf.
            outstr = sep.join(args) + end
            ptr = self.str_to_new_utf8(outstr)
            self.print_str(ptr)
            self.free(ptr)
        else:
            print(*args, end=end, sep=sep, file=self.stdout)

    def decode_str(self, ptr):
        # Hopefully none of our strings are longer than 100 bytes...
        mem = self.memory.read(self.store, ptr, ptr + 100)
        idx = mem.find(b"\x00")
        return mem[0:idx].decode()

    def str_to_new_utf8(self, s):
        """Encode s to bytes, add a null byte to end, malloc a new ptr of the
        appropriate size and assign the bytes to it.

        Only works in wasi, otherwise we won't have exported malloc.
        """
        if not self.is_wasi:
            raise Exception("wasi only!")
        value = s.encode() + b"\x00"
        sz = len(s)
        ptr = self.malloc(sz)
        self.memory.write(self.store, value, ptr)
        return ptr

    def decode_i32(self, ptr):
        mem = self.memory.read(self.store, ptr, ptr + 4)
        return unpack("I", mem)[0]

    # stdlib functions for wasm32-unknown

    @signature("iii", wasi=False)
    def printf(self, fmt_ptr, varargs):
        """wasm32-unknown stdlib stub

        According to the clang wasm ABI, varargs is a void** pointing to the
        beginning of the varargs in the memory stack.

        Currently only handles %s and %d.
        """
        fmt = self.decode_str(fmt_ptr)
        repls = []
        varargs -= 4
        for fchar in format_types(fmt):
            varargs += 4
            if fchar == "s":
                repls.append(self.decode_str(self.decode_i32(varargs)))
                continue
            if fchar == "d":
                repls.append(self.decode_i32(varargs))
                continue
            print(f"Unknown fchar {fchar}")
            raise Exception(f"Unknown fchar {fchar}")
        result = fmt % tuple(repls)
        self.print(result, end="")
        return len(result)

    # Hiwire functionality: traceref and deduplication

    @signature("viiiei")
    def hiwire_traceref(self, type_ptr, ref, index, value_externref, refcount):
        value_int = self.ref_to_int(value_externref)
        type = self.decode_str(type_ptr)
        self.print(
            "hiwire traceref",
            f"{{ type: {type!r}, ref: {ref}, index: {index}, value: {value_int}, refcount: {refcount} }}",
        )
        self.stdout.flush()

    @signature("ie")
    def hiwire_deduplicate_get(self, k):
        return self.d.get(id(k), 0)

    @signature("vei")
    def hiwire_deduplicate_set(self, k, v):
        self.d[id(k)] = v

    @signature("ve")
    def hiwire_deduplicate_delete(self, k):
        del self.d[id(k)]

    # testlib functions

    @signature("e")
    def js_value(self):
        return {"a": 7}

    @signature("ve")
    def print_value(self, o):
        self.print(o)

    @signature("ei")
    def int_to_ref(self, x):
        # wasmtime won't let us treat an int as an externref so put it in a
        # tuple.
        return (x,)

    @signature("ie")
    def ref_to_int(self, x):
        # x could also be None if it's a null reference
        return x[0] if x else -1

    @signature("ie")
    def is_null(self, x):
        return x is None

    @signature("ei")
    def get_obj(self, x):
        return {"x": x}
