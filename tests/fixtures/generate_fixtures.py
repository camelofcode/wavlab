#!/usr/bin/env python3
"""Generate small, deterministic PCM WAV fixtures without using WavLab."""

import argparse
import hashlib
import json
import struct
import tempfile
from pathlib import Path


FIXTURE_DIRECTORY = Path(__file__).resolve().parent
MANIFEST_NAME = "manifest.json"


def riff_chunk(chunk_id, payload):
    if len(chunk_id) != 4:
        raise ValueError("RIFF chunk identifiers must contain exactly four bytes")
    padding = b"\0" if len(payload) % 2 else b""
    return chunk_id + struct.pack("<I", len(payload)) + payload + padding


def encode_samples(bits_per_sample, samples):
    encoded = bytearray()
    for sample in samples:
        if bits_per_sample == 8:
            if not 0 <= sample <= 255:
                raise ValueError("8-bit PCM fixture samples must be unsigned bytes")
            encoded.extend(struct.pack("<B", sample))
        elif bits_per_sample == 16:
            encoded.extend(struct.pack("<h", sample))
        elif bits_per_sample == 24:
            if not -(1 << 23) <= sample < (1 << 23):
                raise ValueError("24-bit PCM fixture sample is out of range")
            encoded.extend(sample.to_bytes(3, byteorder="little", signed=True))
        elif bits_per_sample == 32:
            encoded.extend(struct.pack("<i", sample))
        else:
            raise ValueError("unsupported fixture bit depth: {}".format(bits_per_sample))
    return bytes(encoded)


def make_pcm_wave(
    channels,
    sample_rate,
    bits_per_sample,
    samples,
    chunks_before_data=(),
    chunks_after_data=(),
):
    if channels not in (1, 2):
        raise ValueError("fixtures support only mono and stereo")
    if len(samples) % channels:
        raise ValueError("interleaved samples must contain complete frames")

    bytes_per_sample = bits_per_sample // 8
    block_align = channels * bytes_per_sample
    byte_rate = sample_rate * block_align
    format_payload = struct.pack(
        "<HHIIHH",
        1,
        channels,
        sample_rate,
        byte_rate,
        block_align,
        bits_per_sample,
    )

    chunks = [riff_chunk(b"fmt ", format_payload)]
    chunks.extend(riff_chunk(chunk_id, payload) for chunk_id, payload in chunks_before_data)
    chunks.append(riff_chunk(b"data", encode_samples(bits_per_sample, samples)))
    chunks.extend(riff_chunk(chunk_id, payload) for chunk_id, payload in chunks_after_data)

    riff_payload = b"WAVE" + b"".join(chunks)
    return b"RIFF" + struct.pack("<I", len(riff_payload)) + riff_payload


def fixed_ascii(value, size):
    encoded = value.encode("ascii")
    if len(encoded) > size:
        raise ValueError("fixed BEXT field is too long")
    return encoded + bytes(size - len(encoded))


def make_bext_payload(label):
    known_fields = b"".join(
        (
            fixed_ascii("WavLab {} fixture".format(label), 256),
            fixed_ascii("wavlab tests", 32),
            fixed_ascii("fixture-{}".format(label), 32),
            fixed_ascii("2026-11-05", 10),
            fixed_ascii("12:34:56", 8),
            struct.pack("<IIH", 0x01234567, 0x00000001, 0),
        )
    )
    extra_data = bytes((index * 17 + 3) % 256 for index in range(254))
    payload = known_fields + extra_data
    if len(payload) != 602:
        raise AssertionError("BEXT v0 fixture payload must contain 602 bytes")
    return payload


def definitions():
    bext_before = make_bext_payload("header")
    bext_after = make_bext_payload("footer")
    return [
        {
            "name": "pcm_u8_mono.wav",
            "purpose": "unsigned 8-bit PCM boundaries and silence at 128",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 8,
            "samples": [0, 64, 128, 192, 255],
        },
        {
            "name": "pcm_s16_mono.wav",
            "purpose": "signed 16-bit PCM boundaries and values around zero",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 16,
            "samples": [-32768, -1, 0, 1, 32767],
        },
        {
            "name": "pcm_s24_mono.wav",
            "purpose": "signed 24-bit PCM boundaries and values around zero",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 24,
            "samples": [-(1 << 23), -1, 0, 1, (1 << 23) - 1],
        },
        {
            "name": "pcm_s32_mono.wav",
            "purpose": "signed 32-bit PCM boundaries and values around zero",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 32,
            "samples": [-(1 << 31), -1, 0, 1, (1 << 31) - 1],
        },
        {
            "name": "pcm_s16_stereo.wav",
            "purpose": "stereo frame layout with distinct left and right samples",
            "channels": 2,
            "sample_rate": 8000,
            "bits_per_sample": 16,
            "samples": [1000, -1000, 2000, -2000, 3000, -3000],
        },
        {
            "name": "empty_pcm_s16_mono.wav",
            "purpose": "valid PCM structure with an empty data chunk",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 16,
            "samples": [],
        },
        {
            "name": "unknown_odd_chunk.wav",
            "purpose": "unknown odd-sized RIFF chunk and required padding",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 16,
            "samples": [1234, -1234],
            "chunks_before_data": [(b"XTRA", b"abcde")],
        },
        {
            "name": "bext_before_data.wav",
            "purpose": "BEXT metadata located before the audio data chunk",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 16,
            "samples": [111, -222, 333],
            "chunks_before_data": [(b"bext", bext_before)],
        },
        {
            "name": "bext_after_data.wav",
            "purpose": "BEXT metadata located after the audio data chunk",
            "channels": 1,
            "sample_rate": 8000,
            "bits_per_sample": 16,
            "samples": [444, -555, 666],
            "chunks_after_data": [(b"bext", bext_after)],
        },
        {
            "name": "signal_impulse_mono_s16.wav",
            "purpose": "short full-scale impulse followed by silence",
            "channels": 1,
            "sample_rate": 1000,
            "bits_per_sample": 16,
            "samples": [32767, 0, 0, 0, 0, 0, 0, 0],
        },
        {
            "name": "signal_constant_mono_s16.wav",
            "purpose": "short constant-amplitude signal",
            "channels": 1,
            "sample_rate": 1000,
            "bits_per_sample": 16,
            "samples": [8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192],
        },
        {
            "name": "signal_ramp_mono_s16.wav",
            "purpose": "ordered values for reversal, splitting, and gain tests",
            "channels": 1,
            "sample_rate": 1000,
            "bits_per_sample": 16,
            "samples": [-32768, -24576, -16384, -8192, 0, 8192, 16384, 24576, 32767],
        },
        {
            "name": "signal_channel_identity_stereo_s16.wav",
            "purpose": "opposite-sign channels for routing and state-isolation tests",
            "channels": 2,
            "sample_rate": 1000,
            "bits_per_sample": 16,
            "samples": [1000, -1000, 2000, -2000, 3000, -3000, 4000, -4000],
        },
    ]


def chunk_order(definition):
    order = ["fmt "]
    order.extend(chunk_id.decode("ascii") for chunk_id, _ in definition.get("chunks_before_data", ()))
    order.append("data")
    order.extend(chunk_id.decode("ascii") for chunk_id, _ in definition.get("chunks_after_data", ()))
    return order


def generate(destination):
    destination.mkdir(parents=True, exist_ok=True)
    manifest_entries = []

    for definition in definitions():
        wav_bytes = make_pcm_wave(
            definition["channels"],
            definition["sample_rate"],
            definition["bits_per_sample"],
            definition["samples"],
            definition.get("chunks_before_data", ()),
            definition.get("chunks_after_data", ()),
        )
        path = destination / definition["name"]
        path.write_bytes(wav_bytes)
        manifest_entries.append(
            {
                "name": definition["name"],
                "purpose": definition["purpose"],
                "channels": definition["channels"],
                "sample_rate": definition["sample_rate"],
                "bits_per_sample": definition["bits_per_sample"],
                "frames": len(definition["samples"]) // definition["channels"],
                "interleaved_integer_samples": definition["samples"],
                "chunk_order": chunk_order(definition),
                "size_bytes": len(wav_bytes),
                "sha256": hashlib.sha256(wav_bytes).hexdigest(),
            }
        )

    manifest = {
        "generator": "tests/fixtures/generate_fixtures.py",
        "schema_version": 1,
        "fixtures": manifest_entries,
    }
    manifest_bytes = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
    (destination / MANIFEST_NAME).write_bytes(manifest_bytes)
    return [entry["name"] for entry in manifest_entries] + [MANIFEST_NAME]


def check():
    with tempfile.TemporaryDirectory(prefix="wavlab-fixtures-") as temporary_directory:
        expected_directory = Path(temporary_directory)
        generated_names = generate(expected_directory)
        mismatches = []
        for name in generated_names:
            checked_in = FIXTURE_DIRECTORY / name
            expected = expected_directory / name
            if not checked_in.exists():
                mismatches.append("missing: {}".format(name))
            elif name == MANIFEST_NAME:
                checked_in_manifest = json.loads(checked_in.read_text(encoding="utf-8"))
                expected_manifest = json.loads(expected.read_text(encoding="utf-8"))
                if checked_in_manifest != expected_manifest:
                    mismatches.append("stale: {}".format(name))
            elif checked_in.read_bytes() != expected.read_bytes():
                mismatches.append("stale: {}".format(name))

        if mismatches:
            print("fixture verification failed:")
            for mismatch in mismatches:
                print("  {}".format(mismatch))
            print("run: python3 tests/fixtures/generate_fixtures.py")
            return 1

    print("all generated WAV fixtures and manifest are current")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify checked-in fixtures without modifying them",
    )
    arguments = parser.parse_args()

    if arguments.check:
        return check()

    generated_names = generate(FIXTURE_DIRECTORY)
    print("generated {} fixtures and manifest".format(len(generated_names) - 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
