import os
import subprocess
from pathlib import Path

TEST_DIR = Path(__file__).parent
ROOT_DIR = TEST_DIR.parent


def join_args(args: list[str | Path]) -> str:
    return " ".join(str(x) for x in args)


def run_process(cmd, **args):
    print("+ ", join_args(cmd))
    subprocess.run(cmd, check=True, **args)


def make(cc: str, *, cflags: str = "", opts: list[str] | None = None):
    if not opts:
        opts = []
    subprocess.run(["make", "clean"], cwd=ROOT_DIR)
    env = os.environ | {
        "CC": cc,
        "CFLAGS": join_args(cflags),
    }
    for opt in opts:
        [k, _, v] = opt.partition("=")
        env[k] = v
    subprocess.run(["make"], env=env, cwd=ROOT_DIR)


def build_test(
    test_name: str,
    cc: str,
    *,
    out_ext: str,
    cflags: list[str] | None = None,
    ldflags: list[str] | None = None,
):
    cflags = list(cflags) if cflags else []
    ldflags = list(ldflags) if cflags else []
    cflags += [
        "-mreference-types",
        "-I",
        ROOT_DIR / "dist/include/",
        "-fPIC",
        "-Werror=int-conversion",
        "-Werror=incompatible-pointer-types",
    ]
    ldflags += ["-L", ROOT_DIR / "dist/lib", "-lhiwire", "testlib.o"]
    print("cflags:", cflags)
    run_process([cc, "-c", "testlib.c"] + cflags, cwd=TEST_DIR)
    run_process([cc, "-c", f"test_{test_name}.c"] + cflags, cwd=TEST_DIR)
    run_process(
        [cc, "-o", f"test_{test_name}.{out_ext}", f"test_{test_name}.o"] + ldflags,
        cwd=TEST_DIR,
    )


def build_emcc():
    cc = "emcc"
    make(cc, opts=["EMSCRIPTEN_DEDUPLICATE"])
    for test in ["basic", "many_refs", "deduplication", "versions"]:
        build_test(test, cc, out_ext="js")


def build_wasi():
    cc = "clang"
    optflags = ["-O2"]
    targetflags = [
        "-target",
        "wasm32-wasi",
        "--sysroot",
        "/home/hood/Documents/programming/wasi-libc/sysroot",
    ]
    cflags = targetflags + optflags
    ldflags = targetflags + optflags

    make("clang", opts=["EXTERN_DEDUPLICATE"], cflags=cflags)
    for test in ["basic", "many_refs", "deduplication", "versions"]:
        build_test(test, cc, out_ext="wasm", cflags=cflags, ldflags=ldflags)


def build_wasm_unknown():
    cc = "clang"
    flags = ["-target", "wasm32"]
    cflags = flags
    ldflags = flags + ["-nostdlib"]

    make("clang", opts=["EXTERN_DEDUPLICATE", "STATIC_PAGES=1"], cflags=cflags)
    for test in ["basic", "many_refs", "deduplication", "versions"]:
        build_test(test, cc, out_ext="wasm", cflags=cflags, ldflags=ldflags)


# build_emcc()
# build_wasi()
# build_wasm_unknown()
