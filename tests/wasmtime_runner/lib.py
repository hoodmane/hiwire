import sys
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


def signature(sig):
    def dec(fn):
        fn._wasm_signature = sig_to_functype(sig)
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
    def __init__(self, stdout=None):
        self.stdout = stdout if stdout else sys.stdout
        self.d = {}

    def set_store(self, store):
        self.store = store

    def set_memory(self, memory):
        self.memory = memory

    def set_exports(self, exports):
        self.exports = exports
        self.print_str = exports.get("print_str")
        self.malloc = exports.get("malloc")
        self.free = exports.get("free")

    def define_lib(self, linker):
        for k in type(self).__dict__:
            v = getattr(self, k)
            if not hasattr(v, "_wasm_signature"):
                continue
            linker.define_func("env", v.__name__, v._wasm_signature, v)

    def print(self, *args, end="\n", sep=" "):
        if self.print_str:
            # We're in wasi, route the string through wasi's print function so
            # that they interleave correctly with calls to printf.
            outstr = sep.join(args) + end
            ptr = self.str_to_new_utf8(outstr)
            self.print_str(self.store, ptr)
            self.free(self.store, ptr)
        else:
            print(*args, end=end, sep=sep, file=self.stdout)

    @signature("e")
    def js_value(self):
        return {"a": 7}

    @signature("ve")
    def print_value(self, o):
        self.print(o)

    def decode_str(self, ptr):
        mem = self.memory.read(self.store, ptr, ptr + 100)
        idx = mem.find(b"\x00")
        return mem[0:idx].decode()

    def str_to_new_utf8(self, s):
        value = s.encode() + b"\x00"
        sz = len(s)
        ptr = self.malloc(self.store, sz)
        self.memory.write(self.store, value, ptr)
        return ptr

    def decode_i32(self, ptr):
        mem = self.memory.read(self.store, ptr, ptr + 4)
        return unpack("I", mem)[0]

    @signature("viiiei")
    def hiwire_traceref(self, type_ptr, ref, index, value, refcount):
        type = self.decode_str(type_ptr)
        self.print(
            "hiwire traceref",
            f"{{ type: {type!r}, ref: {ref}, index: {index}, value: {value[0]}, refcount: {refcount} }}",
        )
        self.stdout.flush()

    @signature("iii")
    def printf(self, x, y):
        fmt = self.decode_str(x)
        repls = []
        y -= 4
        for fchar in format_types(fmt):
            y += 4
            if fchar == "s":
                repls.append(self.decode_str(self.decode_i32(y)))
                continue
            if fchar == "d":
                repls.append(self.decode_i32(y))
                continue
            print("Unknown fchar", fchar)
            raise Exception("oops!")
        result = fmt % tuple(repls)
        self.print(result, end="")
        return len(result)

    @signature("ie")
    def _hiwire_deduplicate_get(self, k):
        return self.d.get(id(k), 0)

    @signature("vei")
    def _hiwire_deduplicate_set(self, k, v):
        self.d[id(k)] = v

    @signature("ve")
    def _hiwire_deduplicate_delete(self, k):
        del self.d[id(k)]

    @signature("ei")
    def int_to_ref(self, x):
        return [x]

    @signature("ie")
    def ref_to_int(self, x):
        return x[0] if x else -1

    @signature("ie")
    def is_null(self, x):
        return x is None

    @signature("ei")
    def get_obj(self, x):
        return {"x": x}
