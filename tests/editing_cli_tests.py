#!/usr/bin/env python3

import struct
import subprocess
import sys
import tempfile
import wave
from pathlib import Path


def write_wave(path: Path, channels: int, sample_rate: int, frames: list[tuple[int, ...]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(
            b"".join(struct.pack("<" + "h" * channels, *frame) for frame in frames)
        )


def read_wave(path: Path) -> tuple[int, int, list[tuple[int, ...]]]:
    with wave.open(str(path), "rb") as wav_file:
        channels = wav_file.getnchannels()
        sample_rate = wav_file.getframerate()
        frame_count = wav_file.getnframes()
        values = struct.unpack(
            "<" + "h" * frame_count * channels, wav_file.readframes(frame_count)
        )
    frames = [
        tuple(values[index : index + channels])
        for index in range(0, len(values), channels)
    ]
    return channels, sample_rate, frames


def run(cli: Path, *arguments: object) -> None:
    result = subprocess.run(
        [str(cli), *(str(argument) for argument in arguments)],
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        raise AssertionError(
            f"command failed ({result.returncode}): {result.args!r}\n"
            f"stdout: {result.stdout}\nstderr: {result.stderr}"
        )


def main() -> int:
    cli = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix="wavlab-editing-cli-") as directory:
        root = Path(directory) / "directory.with.wav.in.name"
        stereo = root / "Example.Original.WAV"
        stereo_frames = [(1000, -1000), (2000, -2000), (3000, -3000)]
        write_wave(stereo, 2, 1000, stereo_frames)

        run(cli, "split-stereo", stereo)
        default_left = root / "Example.Original_l.WAV"
        default_right = root / "Example.Original_r.WAV"
        assert read_wave(default_left) == (1, 1000, [(1000,), (2000,), (3000,)])
        assert read_wave(default_right) == (1, 1000, [(-1000,), (-2000,), (-3000,)])

        explicit_left = root / "chosen-left.pcm"
        explicit_right = root / "chosen-right.pcm"
        run(cli, "split-stereo", stereo, explicit_left, explicit_right)
        assert read_wave(explicit_left)[2] == [(1000,), (2000,), (3000,)]
        assert read_wave(explicit_right)[2] == [(-1000,), (-2000,), (-3000,)]

        run(cli, "split-mid-side", stereo)
        assert (root / "Example.Original_mid.WAV").is_file()
        assert (root / "Example.Original_side.WAV").is_file()

        to_normalise = root / "level.wav"
        normalised = root / "level-normalised.wav"
        write_wave(to_normalise, 1, 1000, [(8192,), (-16384,)])
        run(cli, "normalise-db", to_normalise, -20, normalised)
        _, _, normalised_frames = read_wave(normalised)
        assert abs(max(abs(frame[0]) for frame in normalised_frames) - 3277) <= 1

        us_alias_output = root / "us-alias.wav"
        run(cli, "normalize", to_normalise, us_alias_output)
        assert max(abs(frame[0]) for frame in read_wave(us_alias_output)[2]) == 32768

        split_input = root / "Segments.WAV"
        write_wave(split_input, 1, 1000, [(0,), (100,), (200,), (300,), (400,), (500,)])
        run(cli, "split", split_input, 2, 5)
        assert read_wave(root / "Segments_1.WAV")[2] == [(0,), (100,)]
        assert read_wave(root / "Segments_2.WAV")[2] == [(200,), (300,), (400,)]
        assert read_wave(root / "Segments_3.WAV")[2] == [(500,)]

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
