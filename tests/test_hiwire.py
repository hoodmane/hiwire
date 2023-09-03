import subprocess

import pytest
import wasmtime_runner
from wasmtime import ExitTrap

from build import ROOT_DIR, TEST_DIR, build_test, compile, make, run_process

TEST_BUILD_DIR = TEST_DIR / "build"
ALL_TESTS = ["basic", "many_refs", "versions", "deduplication"]
PLATFORMS = ["emcc", "wasi", "wasm_unknown"]
HOSTED_PLATFORMS = ["emcc", "wasi"]


def emcc_run(test):
    res = subprocess.run(["node", test], capture_output=True, encoding="utf8")
    if res.returncode:
        print(res.stdout)
        print(res.stderr)
        raise ExitTrap(f"Exited with i32 exit status {res.returncode}")
    return res.stdout


def run_test(platform, test_name):
    extension = "js" if platform == "emcc" else "wasm"
    test = TEST_BUILD_DIR / f"test_{test_name}.{extension}"
    expected = (TEST_DIR / "ctests" / f"test_{test_name}.out").read_text()
    if platform == "emcc":
        res = emcc_run(test)
    else:
        res = wasmtime_runner.run(test, platform == "wasi")
    assert res == expected


@pytest.mark.parametrize("platform", PLATFORMS)
def test_basic_static(platform):
    make(platform, opts=["STATIC_PAGES=1"])
    build_test(platform, "basic")
    run_test(platform, "basic")


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_basic_realloc(platform):
    make(platform)
    build_test(platform, "basic")
    run_test(platform, "basic")


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_many_refs_realloc(platform):
    make(platform, opts=[])
    build_test(platform, "many_refs")
    run_test(platform, "many_refs")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_many_refs_static_enough_space(platform):
    make(platform, opts=["STATIC_PAGES=5"])
    build_test(platform, "many_refs")
    run_test(platform, "many_refs")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_many_refs_static_too_little_space(platform):
    make(platform, opts=["STATIC_PAGES=4"])
    build_test(platform, "many_refs")
    with pytest.raises(ExitTrap, match="Exited with i32 exit status 1"):
        run_test(platform, "many_refs")


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_versions_realloc(platform):
    make(platform, opts=[])
    build_test(platform, "versions")
    run_test(platform, "versions")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_versions_static(platform):
    make(platform, opts=["STATIC_PAGES=1"])
    build_test(platform, "versions")
    run_test(platform, "versions")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_deduplication(platform):
    dedup = "EMSCRIPTEN_DEDUPLICATE" if platform == "emcc" else "EXTERN_DEDUPLICATE"
    make(platform, opts=["STATIC_PAGES=1", dedup])
    build_test(platform, "deduplication")
    run_test(platform, "deduplication")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_immortal(platform):
    make(platform, opts=["STATIC_PAGES=1"])
    build_test(platform, "immortal")
    run_test(platform, "immortal")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_immortal_deduplication(platform):
    dedup = "EMSCRIPTEN_DEDUPLICATE" if platform == "emcc" else "EXTERN_DEDUPLICATE"
    make(platform, opts=["STATIC_PAGES=1", dedup])
    build_test(platform, "immortal_deduplication")
    run_test(platform, "immortal_deduplication")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_extern_realloc(platform):
    make(platform, opts=["EXTERN_REALLOC"])
    build_test(platform, "extern_realloc")
    run_test(platform, "extern_realloc")


@pytest.mark.parametrize("test_name", ALL_TESTS)
def test_emcc_dylink(test_name):
    make("emcc", opts=["EMSCRIPTEN_DEDUPLICATE"])
    run_process(
        [
            "emcc",
            ROOT_DIR / "dist/lib/libhiwire.a",
            "-sSIDE_MODULE=1",
            "-o",
            TEST_BUILD_DIR / "libhiwire.so",
            "-O2",
        ]
    )
    cflags = [
        "-O2",
        "-mreference-types",
        "-I",
        ROOT_DIR / "dist/include/",
        "-I",
        ROOT_DIR / "src",
        "-fPIC",
        "-Werror=int-conversion",
        "-Werror=incompatible-pointer-types",
    ]
    ldflags = [
        "-O2",
        "-L",
        ROOT_DIR / "dist/lib",
        TEST_BUILD_DIR / "libhiwire.so",
        "testlib.o",
        "-sMAIN_MODULE=2",
    ]
    compile("emcc", cflags, "testlib", TEST_BUILD_DIR)
    compile("emcc", cflags, f"test_{test_name}", TEST_BUILD_DIR)
    run_process(
        ["emcc", "-o", f"test_{test_name}.js", f"test_{test_name}.o"] + ldflags,
        cwd=TEST_BUILD_DIR,
    )
    run_test("emcc", test_name)
