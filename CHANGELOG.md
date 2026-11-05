# Changelog

All notable changes to the modern C++ implementation are documented here.

## [Unreleased]

The next release is version 0.1.0 and intentionally replaces the historical
pre-release C++ API and command syntax.

### Added

- A reusable C++20 library target and a separately linked `wavlab` executable.
- Installable CMake package metadata exposing `wavlab::wavlab`.
- CMake presets for Debug, Release, sanitizers, shared libraries, and MSVC.
- Cross-platform GitHub Actions builds for GCC, Clang, Apple Clang, and MSVC.
- Deterministic PCM/BEXT fixtures and library, CLI, DSP, package-consumer, and
  generated-documentation tests.
- Chorus, vibrato, flanging, tremolo, autopanning, phasing, and modulated
  low/high-pass filtering.
- Generated CLI reference documentation and conventional help/version output.

### Changed

- Public declarations now live under namespace `wavlab` in `include/wavlab`.
- The CLI uses normalised `[0, 1]` mix/depth values, signed feedback ratios,
  milliseconds for delay/tail values, RT60 seconds, and decibels for drive.
- `normalise` and `normalise-db` are canonical; American spellings remain
  aliases. `metadata` retains `stat` as an alias.
- WAV writes use a temporary sibling and replace the destination only after a
  complete successful write.

### Fixed

- Portable little-endian PCM conversion, including unsigned 8-bit and signed
  24-bit samples, checked stream operations, and finite-value handling.
- RIFF chunk scanning, odd-byte padding, BEXT position/extra-data preservation,
  stereo block alignment, byte rates, frame counts, and truncated-file errors.
- Empty/silent normalisation, frame-aware reversal and splitting, stereo and
  mid/side combination, mismatched-length mixing, and channel-safe joining.
- Sub-second generators, Nyquist-limited additive synthesis, validated biquad
  parameters, fractional delays, independent stereo effect state, and true
  Schroeder all-pass reverb stages.

### Removed

- Library-level process exits, terminal output, CLI argument handling, global
  linker flags, forced static runtime linking, and the duplicated legacy
  command parser/help system.
