#!/usr/bin/env python3

import shutil
import struct
import subprocess
import sys
import tempfile
import wave
from pathlib import Path


def write_wave(path: Path) -> None:
    samples = [12000 if index == 0 else (index % 16 - 8) * 400 for index in range(128)]
    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(8000)
        wav_file.writeframes(b"".join(struct.pack("<h", sample) for sample in samples))


def run(cli: Path, *arguments: object) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(cli), *(str(argument) for argument in arguments)],
        text=True,
        capture_output=True,
        check=False,
    )


def require_success(result: subprocess.CompletedProcess[str]) -> None:
    if result.returncode != 0:
        raise AssertionError(
            f"command failed ({result.returncode}): {result.args!r}\n"
            f"stdout: {result.stdout}\nstderr: {result.stderr}"
        )


def assert_wave(path: Path, channels: int = 1) -> None:
    with wave.open(str(path), "rb") as wav_file:
        assert wav_file.getnchannels() == channels
        assert wav_file.getframerate() == 8000
        assert wav_file.getnframes() == 128


def main() -> int:
    cli = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix="wavlab-cli-execution-") as directory:
        root = Path(directory)
        input_path = root / "input.wav"
        write_wave(input_path)

        commands: list[tuple[str, list[object], int]] = [
            ("saturate", [input_path, 6], 1),
            ("digital-delay-mono", [input_path, 2, -0.5, 0.5], 1),
            ("digital-delay-stereo", [input_path, 2, -0.5, 3, 0.5, 0.5], 2),
            ("analog-delay-mono", [input_path, 2, -0.5, 0.5], 1),
            ("analog-delay-stereo", [input_path, 2, -0.5, 3, 0.5, 0.5], 2),
            ("tape-delay-mono", [input_path, 2, -0.5, 0.5, "--drive-db", 6], 1),
            (
                "tape-delay-stereo",
                [input_path, 2, -0.5, 3, 0.5, 0.5, "--drive-db", 6],
                2,
            ),
            ("reverb", [input_path, 0.5, 1.5, 0.5], 1),
            ("eq-peak", [input_path, 1000, -3, 1], 1),
            ("eq-lowshelf", [input_path, 1000, -3, 0.7], 1),
            ("eq-highshelf", [input_path, 1000, 3, 0.7], 1),
            ("eq-allpass", [input_path, 1000, 1], 1),
            ("eq-bandpass", [input_path, 1000, 1], 1),
            ("eq-lowpass", [input_path, 1000, 0.7], 1),
            ("eq-highpass", [input_path, 1000, 0.7], 1),
        ]
        for command, arguments, channels in commands:
            output = root / f"{command}.wav"
            require_success(run(cli, command, *arguments, output))
            assert_wave(output, channels)

        overwrite = root / "overwrite.wav"
        shutil.copyfile(input_path, overwrite)
        before = overwrite.read_bytes()
        require_success(run(cli, "gain", overwrite, 0.5))
        assert overwrite.read_bytes() != before
        assert_wave(overwrite)

        metadata = run(cli, "metadata", "--full", input_path)
        require_success(metadata)
        assert "Levels" in metadata.stdout
        assert "Peak:" in metadata.stdout
        assert "RMS:" in metadata.stdout

        version = run(cli, "--version")
        require_success(version)
        assert version.stdout.strip() == "0.1.0"

        help_result = run(cli, "digital-delay-mono", "--help")
        require_success(help_result)
        assert "DELAY_MS" in help_result.stdout
        assert "--tail-ms" in help_result.stdout

        invalid_output = root / "invalid.wav"
        invalid = run(cli, "digital-delay-mono", input_path, 2, 0.5, 1.1, invalid_output)
        assert invalid.returncode != 0
        assert "not in range [0 - 1]" in invalid.stderr
        assert not invalid_output.exists()

        missing = run(cli, "normalise", root / "missing.wav")
        assert missing.returncode != 0
        assert "File does not exist" in missing.stderr

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
