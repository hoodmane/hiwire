import sys
from pathlib import Path

from . import run


def main():
    TEST_DIR = Path(__file__).parents[1]
    name = sys.argv[-1]

    result = run(TEST_DIR / "build" / f"test_{name}.wasm", True)
    (TEST_DIR / "ctests" / f"test_{name}.out").write_text(result.stdout)
    print(result.stdout)


if __name__ == "__main__":
    main()
