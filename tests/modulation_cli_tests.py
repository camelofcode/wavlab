#!/usr/bin/env python3

import struct
import subprocess
import sys
import tempfile
import wave
from pathlib import Path


def write_wave(path: Path) -> None:
    samples = [12000 if index == 0 else int(4000 * ((index % 16) / 15 - 0.5)) for index in range(128)]
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


def assert_wave(path: Path, channels: int) -> None:
    with wave.open(str(path), "rb") as wav_file:
        assert wav_file.getnchannels() == channels
        assert wav_file.getframerate() == 8000
        assert wav_file.getnframes() == 128


def main() -> int:
    cli = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix="wavlab-modulation-cli-") as directory:
        root = Path(directory)
        input_path = root / "input.wav"
        write_wave(input_path)

        commands: list[tuple[str, list[object], int]] = [
            ("chorus-mono", [input_path, 0.5, 1.0, 0.5], 1),
            ("chorus-stereo", [input_path, 0.5, 1.0, 1.0, 2.0, 0.5], 2),
            ("vibrato-mono", [input_path, 0.5, 1.0], 1),
            ("vibrato-stereo", [input_path, 0.5, 1.0, 1.0, 2.0], 2),
            ("flanger-mono", [input_path, 0.5, 1.0, -0.5, 0.5], 1),
            (
                "flanger-stereo",
                [input_path, 0.5, 1.0, -0.5, 1.0, 2.0, 0.5, 0.5],
                2,
            ),
            ("tremolo", [input_path, 0.5, 1.0, "triangle"], 1),
            ("autopan", [input_path, 0.5, 1.0, "square"], 2),
            (
                "phaser-mono",
                [input_path, 0.5, 1.0, 1000.0, "sine", 0.5],
                1,
            ),
            (
                "phaser-stereo",
                [input_path, 0.5, 1.0, 800.0, "sine", 1.0, 2.0, 1200.0, "triangle", 0.5],
                2,
            ),
            (
                "modulated-filter",
                [input_path, 200.0, 1500.0, 1.0, "low-pass", "triangle", 0.707],
                1,
            ),
        ]

        for command, arguments, channels in commands:
            output = root / f"{command}.wav"
            extra = ["--stages", 4] if command.startswith("phaser-") else []
            result = run(cli, command, *arguments, output, *extra)
            if result.returncode != 0:
                raise AssertionError(
                    f"{command} failed ({result.returncode})\n"
                    f"stdout: {result.stdout}\nstderr: {result.stderr}"
                )
            assert_wave(output, channels)

        invalid_output = root / "invalid.wav"
        invalid = run(cli, "tremolo", input_path, 0.5, 1.0, "sawtooth", invalid_output)
        assert invalid.returncode != 0
        assert "Waveform must be" in invalid.stderr
        assert not invalid_output.exists()

        help_result = run(cli, "--help")
        assert help_result.returncode == 0
        for command, _, _ in commands:
            assert command in help_result.stdout

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
