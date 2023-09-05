import contextlib
from collections import namedtuple
from pathlib import Path
from tempfile import NamedTemporaryFile

from wasmtime import Engine, ExitTrap, Linker, Module, Store, Trap, WasiConfig

from .lib import WasmLib

Result = namedtuple("Result", "stdout stderr returncode")


@contextlib.contextmanager
def tempfilepath():
    try:
        f = NamedTemporaryFile(delete=False)
        path = Path(f.name)
        f.close()
        yield path
    finally:
        path.unlink()


def run(path, include_wasi):
    engine = Engine()

    store = Store(engine)
    module = Module.from_file(store.engine, path)

    linker = Linker(engine)

    with (
        tempfilepath() as stdoutpath,
        open(stdoutpath, "w") as fstdout,
        tempfilepath() as stderrpath,
    ):
        if include_wasi:
            wasi = WasiConfig()
            wasi.stdout_file = str(stdoutpath)
            wasi.stderr_file = str(stderrpath)
            store.set_wasi(wasi)
            linker.define_wasi()

        lib = WasmLib(stdout=fstdout)
        lib.define_lib(linker)

        m = linker.instantiate(store, module)
        exports = m.exports(store)
        main = exports["_start"]
        memory = exports["memory"]
        lib.set_store(store)
        lib.set_memory(memory)
        lib.set_exports(exports)
        try:
            result = main(store)
        except (ExitTrap, Trap):
            result = 1
        if result is None:
            result = 0
        fstdout.flush()
        return Result(stdoutpath.read_text(), stderrpath.read_text(), result)
