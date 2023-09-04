import contextlib
from pathlib import Path
from tempfile import NamedTemporaryFile

from wasmtime import Engine, ExitTrap, Linker, Module, Store, WasiConfig

from .lib import WasmLib


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

    with tempfilepath() as path, open(path, "w") as f:
        if include_wasi:
            wasi = WasiConfig()
            wasi.stdout_file = str(path)
            store.set_wasi(wasi)
            linker.define_wasi()

        lib = WasmLib(stdout=f)
        lib.define_lib(linker)

        m = linker.instantiate(store, module)
        exports = m.exports(store)
        main = exports["_start"]
        memory = exports["memory"]
        lib.set_store(store)
        lib.set_memory(memory)
        try:
            result = main(store)
            if result:
                raise ExitTrap(f"Exited with i32 exit status {result}")
        except Exception:
            f.flush()
            print(path.read_text())
            raise
        f.flush()
        return path.read_text()
