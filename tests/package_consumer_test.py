#!/usr/bin/env python3

"""Install WavLab and verify a completely separate CMake consumer."""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import tempfile


def run(command: list[str]) -> None:
    subprocess.run(command, check=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cmake", required=True)
    parser.add_argument("--ctest", required=True)
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--config", required=True)
    parser.add_argument("--generator", required=True)
    parser.add_argument("--platform", default="")
    parser.add_argument("--toolset", default="")
    arguments = parser.parse_args()

    with tempfile.TemporaryDirectory(prefix="wavlab-package-test-") as temporary:
        root = Path(temporary)
        prefix = root / "install"
        consumer_build = root / "consumer-build"

        run(
            [
                arguments.cmake,
                "--install",
                str(arguments.build_dir),
                "--config",
                arguments.config,
                "--prefix",
                str(prefix),
            ]
        )

        configure = [
            arguments.cmake,
            "-S",
            str(arguments.source_dir),
            "-B",
            str(consumer_build),
            "-G",
            arguments.generator,
            f"-DCMAKE_PREFIX_PATH={prefix}",
            f"-DCMAKE_BUILD_TYPE={arguments.config}",
        ]
        if arguments.platform:
            configure.extend(["-A", arguments.platform])
        if arguments.toolset:
            configure.extend(["-T", arguments.toolset])
        run(configure)
        run(
            [
                arguments.cmake,
                "--build",
                str(consumer_build),
                "--config",
                arguments.config,
                "--parallel",
                "2",
            ]
        )
        run(
            [
                arguments.ctest,
                "--test-dir",
                str(consumer_build),
                "--build-config",
                arguments.config,
                "--output-on-failure",
            ]
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
