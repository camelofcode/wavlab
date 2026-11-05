#!/usr/bin/env python3

"""Check a generated WAV with ffprobe when it is available."""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main() -> int:
    cli = Path(sys.argv[1]).resolve()
    ffprobe = Path(sys.argv[2]).resolve()

    with tempfile.TemporaryDirectory(prefix="wavlab-ffprobe-") as directory:
        output = Path(directory) / "generated-24-bit.wav"
        subprocess.run(
            [str(cli), "sine-wave", str(output), "440", "48000", "24", "125"],
            check=True,
        )
        result = subprocess.run(
            [
                str(ffprobe),
                "-v",
                "error",
                "-select_streams",
                "a:0",
                "-show_entries",
                "stream=codec_name,sample_rate,channels,bits_per_sample,bits_per_raw_sample",
                "-of",
                "json",
                str(output),
            ],
            check=True,
            capture_output=True,
            text=True,
        )

    streams = json.loads(result.stdout)["streams"]
    if len(streams) != 1:
        raise AssertionError(f"expected one audio stream, got {streams!r}")
    stream = streams[0]
    assert stream["codec_name"] == "pcm_s24le"
    assert stream["sample_rate"] == "48000"
    assert stream["channels"] == 1
    assert 24 in {
        stream.get("bits_per_sample"),
        stream.get("bits_per_raw_sample"),
    }
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
