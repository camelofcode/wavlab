# Parity baseline and comparison contract

This document pins the implementations and comparison rules used throughout
the C++ modernization. Moving either reference requires a deliberate update
to this file and the parity matrix.

## Pinned revisions

| Implementation | Repository revision | Role |
|---|---|---|
| Original C++ baseline | `1e7deca52766628fc3857c02a7730d91d66e6a49` | Historical CLI and behavior inventory |
| Rust reference | `bcbebe9dc4b574f9f40a25655677fd22e8b9b382` | Intended validated behavior and feature set |
| Planning baseline | `6534a95f9b8b41e12f0cee41bfd35cd04b17d36a` | Start of the C++ modernization branch |

The Rust revision has 175 `#[test]` cases. At the time of pinning, this command
passes with 141 library tests and 34 executable tests:

```console
cd ../wavlab-rs
cargo test --all-targets
```

The C++ baseline has no automated tests. On the audit toolchain it configures
with CMake but fails to compile under GCC 16 because fixed-width integer types
are used without self-contained `<cstdint>` includes in `BinaryHelpers.cpp`
and `Delay.h`. Build repair belongs to Phase 1 and is not treated as intended
behavior.

## What parity means

Parity is evaluated at public behavior boundaries, not by translating Rust
source line for line.

- WAV comparisons use decoded samples and relevant metadata.
- Structural tools compare channel count, sample rate, bit depth, frame count,
  and samples.
- Stateful effects compare documented invariants and selected impulse
  responses.
- CLI comparisons cover parsing, exit status, stdout/stderr, output paths, and
  decoded output.
- Intentional C++ differences require a rationale and native tests in the same
  change.

Exact encoded file bytes are not a parity requirement. Equivalent files may
legally differ in chunk order, padding choices, or integer rounding at a
half-LSB boundary.

## Numeric tolerances

Use the narrowest applicable rule:

| Comparison | Tolerance |
|---|---|
| Integer and byte helpers | Exact |
| Metadata, sizes, channel counts, frame positions | Exact |
| Frame rearrangement and polarity operations in memory | Exact |
| Pure `double` calculations using the same formula | Absolute error at most `1e-12` |
| DSP output using equivalent formulas/order | Absolute error at most `1e-10`, unless the test documents why it needs more |
| PCM encode/decode round trip | At most one least-significant bit at the selected bit depth |
| Random noise | Shape, duration, channel, finiteness, and range invariants; no cross-runtime sample equality |

Tests must not silently loosen a global tolerance to accommodate one
algorithm. A looser local tolerance needs a comment explaining the numerical
source and an invariant that still detects meaningful regressions.

## Deterministic inputs

The generated fixtures in `tests/fixtures/` provide:

- mono integer PCM at 8, 16, 24, and 32 bits;
- stereo 16-bit PCM with distinct left and right values;
- an empty valid PCM file;
- an odd-sized unknown chunk with RIFF padding;
- BEXT before and after the data chunk, with deterministic extra bytes; and
- impulse, constant, ramp, and stereo channel-identity signals.

They contain no third-party audio. Run the standard-library generator after
changing fixture definitions:

```console
python3 tests/fixtures/generate_fixtures.py
python3 tests/fixtures/generate_fixtures.py --check
```

The checked-in manifest records format details, purpose, size, and SHA-256 for
every generated WAV. `--check` regenerates files in a temporary directory and
fails if either a fixture or the manifest is stale.

## Reference update procedure

To update the Rust pin:

1. Review all Rust commits since the existing revision.
2. Run `cargo test --all-targets` at the proposed revision.
3. Update the full revision and test count above.
4. Update `docs/rust-parity-matrix.md` for new or changed behavior.
5. Regenerate or extend fixtures only when the comparison contract changes.
6. Commit the pin, matrix, fixtures, and affected C++ tests together.
