import subprocess

import pytest
import wasmtime_runner

from build import ROOT_DIR, TEST_DIR, build_test, compile, make, run_process

TEST_BUILD_DIR = TEST_DIR / "build"
ALL_TESTS = ["basic", "many_refs", "versions", "deduplication"]
PLATFORMS = ["emcc", "wasi", "wasm_unknown"]
HOSTED_PLATFORMS = ["emcc", "wasi"]


def emcc_run(test):
    return subprocess.run(["node", test], capture_output=True, encoding="utf8")


def run_test(platform, test_name):
    extension = "js" if platform == "emcc" else "wasm"
    test = TEST_BUILD_DIR / f"test_{test_name}.{extension}"
    if platform == "emcc":
        return emcc_run(test)
    else:
        return wasmtime_runner.run(test, platform == "wasi")


def run_test_assert_match(platform, test_name):
    res = run_test(platform, test_name)
    out = TEST_DIR / "ctests" / f"test_{test_name}.out"
    expected = out.read_text()
    print("stdout:")
    print("=" * 7)
    print(res.stdout)
    print("stderr:")
    print("=" * 7)
    print(res.stderr)
    assert res.returncode == 0
    assert res.stdout == expected


@pytest.mark.parametrize("platform", PLATFORMS)
def test_basic_static(platform):
    make(platform, opts=["STATIC_PAGES=1"])
    build_test(platform, "basic")
    run_test_assert_match(platform, "basic")


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_basic_realloc(platform):
    make(platform)
    build_test(platform, "basic")
    run_test_assert_match(platform, "basic")


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_many_refs_realloc(platform):
    make(platform, opts=[])
    build_test(platform, "many_refs")
    run_test_assert_match(platform, "many_refs")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_many_refs_static_enough_space(platform):
    make(platform, opts=["STATIC_PAGES=5"])
    build_test(platform, "many_refs")
    run_test_assert_match(platform, "many_refs")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_many_refs_static_too_little_space(platform):
    make(platform, opts=["STATIC_PAGES=4"])
    build_test(platform, "many_refs")
    result = run_test(platform, "many_refs")
    assert result.returncode == 1


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_versions_realloc(platform):
    make(platform, opts=[])
    build_test(platform, "versions")
    run_test_assert_match(platform, "versions")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_versions_static(platform):
    make(platform, opts=["STATIC_PAGES=1"])
    build_test(platform, "versions")
    run_test_assert_match(platform, "versions")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_deduplication(platform):
    dedup = "EMSCRIPTEN_DEDUPLICATE" if platform == "emcc" else "EXTERN_DEDUPLICATE"
    make(platform, opts=["STATIC_PAGES=1", dedup])
    build_test(platform, "deduplication")
    run_test_assert_match(platform, "deduplication")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_immortal(platform):
    make(platform, opts=["STATIC_PAGES=1"])
    build_test(platform, "immortal")
    run_test_assert_match(platform, "immortal")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_extern_realloc(platform):
    make(platform, opts=["EXTERN_REALLOC"])
    build_test(platform, "extern_realloc")
    run_test_assert_match(platform, "extern_realloc")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_tracerefs(platform):
    traceref = "EMSCRIPTEN_TRACEREFS" if platform == "emcc" else "EXTERN_TRACEREFS"
    make(platform, opts=["STATIC_PAGES=1", traceref])
    build_test(platform, "tracerefs")
    run_test_assert_match(platform, "tracerefs")


@pytest.mark.parametrize("platform", HOSTED_PLATFORMS)
def test_abortfail(platform):
    make(platform, opts=["STATIC_PAGES=1", "ABORT_FAIL"])
    for test in ["get", "getnull", "incref", "decref"]:
        build_test(platform, f"abortfail_{test}")
    result = run_test(platform, "abortfail_get")
    assert result.returncode != 0
    assert result.stderr.startswith("hiwire_get failed: reference 3 is freed")
    result = run_test(platform, "abortfail_getnull")
    assert result.returncode != 0
    assert result.stderr.startswith("hiwire_get failed: reference 0 is null")
    result = run_test(platform, "abortfail_incref")
    assert result.returncode != 0
    assert result.stderr.startswith("hiwire_incref failed: reference 3 is freed")
    result = run_test(platform, "abortfail_decref")
    assert result.returncode != 0
    assert result.stderr.startswith("hiwire_decref failed: reference 3 is freed")


@pytest.mark.parametrize("platform", PLATFORMS)
def test_externfail(platform):
    make(platform, opts=["STATIC_PAGES=1", "EXTERN_FAIL"])
    build_test(platform, "externfail")
    run_test_assert_match(platform, "externfail")


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
    run_test_assert_match("emcc", test_name)
