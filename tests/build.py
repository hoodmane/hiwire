import os
import subprocess
from pathlib import Path

TEST_DIR = Path(__file__).parent
ROOT_DIR = TEST_DIR.parent
CTESTS_DIR = TEST_DIR / "ctests"


def join_args(args: list[str | Path]) -> str:
    return " ".join(str(x) for x in args)


def run_process(cmd, **args):
    print("+ ", join_args(cmd))
    subprocess.run(cmd, check=True, **args)


def make_inner(cc: str, *, cflags: str = "", opts: list[str] | None = None):
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


def compile(cc: str, cflags: list[str], name: str, build_dir: Path):
    infile = CTESTS_DIR / f"{name}.c"
    outfile = build_dir / f"{name}.o"
    run_process([cc, "-c", infile, "-o", outfile] + cflags)


def build_test_inner(
    test_name: str,
    cc: str,
    *,
    build_dir: Path,
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
        "-I",
        ROOT_DIR / "src",
        "-fPIC",
        "-Werror=int-conversion",
        "-Werror=incompatible-pointer-types",
    ]
    ldflags += ["-L", ROOT_DIR / "dist/lib", "-lhiwire", "testlib.o"]
    compile(cc, cflags, "testlib", build_dir)
    compile(cc, cflags, f"test_{test_name}", build_dir)

    run_process(
        [cc, "-o", f"test_{test_name}.{out_ext}", f"test_{test_name}.o"] + ldflags,
        cwd=build_dir,
    )


def default_optflags(optflags):
    if optflags is None:
        return ["-O2"]
    return optflags


def emcc_cc(optflags):
    return {"cflags": optflags, "cc": "emcc"}


def make_emcc(opts=None, optflags=None):
    if opts is None:
        opts = ["EMSCRIPTEN_DEDUPLICATE"]
    optflags = default_optflags(optflags)

    make_inner(opts=opts, **emcc_cc(optflags))


def build_test_emcc(test_name: str, optflags=None, build_dir=TEST_DIR / "build"):
    optflags = default_optflags(optflags)
    build_test_inner(
        test_name,
        **emcc_cc(optflags),
        out_ext="js",
        ldflags=optflags,
        build_dir=build_dir,
    )


WASI_TARGET_FLAGS = [
    "-target",
    "wasm32-wasi",
    "--sysroot",
    "/home/hood/Documents/programming/wasi-libc/sysroot",
]


def wasi_cc(optflags):
    return {"cflags": WASI_TARGET_FLAGS + optflags, "cc": "clang"}


def make_wasi(opts=None, optflags=None):
    if opts is None:
        opts = ["EXTERN_DEDUPLICATE"]
    optflags = default_optflags(optflags)
    make_inner(opts=opts, **wasi_cc(optflags))


def build_test_wasi(test_name: str, optflags=None, build_dir=TEST_DIR / "build"):
    optflags = default_optflags(optflags)
    ldflags = WASI_TARGET_FLAGS + optflags
    build_test_inner(
        test_name,
        out_ext="wasm",
        **wasi_cc(optflags),
        ldflags=ldflags,
        build_dir=build_dir,
    )


WASM_UNKNOWN_TARGET_FLAGS = ["-target", "wasm32", "-nostdlib"]


def wasm_unknown_cc(optflags):
    return {"cflags": WASM_UNKNOWN_TARGET_FLAGS + optflags, "cc": "clang"}


def make_wasm_unknown(opts=None, optflags=None):
    if opts is None:
        opts = ["EXTERN_DEDUPLICATE", "STATIC_PAGES=1"]
    optflags = default_optflags(optflags)
    make_inner(opts=opts, **wasm_unknown_cc(optflags))


def build_test_wasm_unknown(
    test_name: str, optflags=None, build_dir=TEST_DIR / "build"
):
    optflags = default_optflags(optflags)
    ldflags = WASM_UNKNOWN_TARGET_FLAGS + optflags
    build_test_inner(
        test_name,
        **wasm_unknown_cc(optflags),
        out_ext="wasm",
        ldflags=ldflags,
        build_dir=build_dir,
    )


def make(platform, opts=None, optflags=None):
    f = {"emcc": make_emcc, "wasi": make_wasi, "wasm_unknown": make_wasm_unknown}[
        platform
    ]
    f(opts, optflags)


def build_test(platform, test_name: str, optflags=None, build_dir=TEST_DIR / "build"):
    f = {
        "emcc": build_test_emcc,
        "wasi": build_test_wasi,
        "wasm_unknown": build_test_wasm_unknown,
    }[platform]
    f(test_name, optflags, build_dir)
