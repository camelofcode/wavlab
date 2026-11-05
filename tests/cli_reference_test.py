#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path


def main() -> int:
    cli = Path(sys.argv[1]).resolve()
    reference = Path(sys.argv[2]).resolve()
    result = subprocess.run(
        [str(cli), "_generate-cli-docs", "-"],
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        raise AssertionError(
            f"CLI reference generation failed ({result.returncode})\n"
            f"stdout: {result.stdout}\nstderr: {result.stderr}"
        )
    expected = reference.read_text(encoding="utf-8").replace("\r\n", "\n")
    actual = result.stdout.replace("\r\n", "\n")
    if actual != expected:
        raise AssertionError(
            "docs/cli.md is stale; regenerate it with "
            "'./out/build/dev/wavlab _generate-cli-docs docs/cli.md'"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
