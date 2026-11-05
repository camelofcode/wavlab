#!/usr/bin/env python3

import pathlib
import subprocess
import sys
import tempfile
import wave


def main():
    executable = pathlib.Path(sys.argv[1])
    with tempfile.TemporaryDirectory(prefix="wavlab-independent-reader-") as directory:
        output = pathlib.Path(directory) / "generated.wav"
        subprocess.run(
            [str(executable), "sine-wave", str(output), "440", "8000", "16", "250"],
            check=True,
        )
        with wave.open(str(output), "rb") as wav_file:
            actual = (
                wav_file.getnchannels(),
                wav_file.getframerate(),
                wav_file.getsampwidth(),
                wav_file.getnframes(),
            )
        expected = (1, 8000, 2, 2000)
        if actual != expected:
            raise AssertionError("expected WAV layout {}, got {}".format(expected, actual))


if __name__ == "__main__":
    main()
