# wavlab

[![C++ build and test](https://github.com/camelofcode/wavlab/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/camelofcode/wavlab/actions/workflows/ci.yml)

WavLab is a C++20 library and command-line utility for inspecting, generating,
editing, and applying effects to PCM WAV files. It supports ordinary WAV files
and version-0 BWF/BEXT metadata with 8-, 16-, 24-, and 32-bit integer samples.

The project began as a way to become familiar with C++ again after more than a
decade away from the language. It deliberately implements the WAV and DSP
logic directly rather than wrapping a general audio library.

## Build and test

The recommended local workflow requires a C++20 compiler, CMake 3.24 or later,
Ninja, and Python 3. CLI11 is found from an installed CMake package when
available, otherwise CMake fetches the pinned release automatically.

```console
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

The build produces:

- `wavlab::wavlab`, the reusable C++ library target;
- `out/build/dev/wavlab`, the command-line executable;
- the CTest suite, including parser, black-box CLI, DSP, fixture, and I/O tests.

See [Development](docs/development.md) for supported toolchains, presets,
sanitizers, formatting, and static analysis.

## Command line

CLI11 provides generated top-level and per-command help, typed arguments,
consistent validation errors, and version output:

```console
./out/build/dev/wavlab --help
./out/build/dev/wavlab --version
./out/build/dev/wavlab chorus-mono --help
```

Examples:

```console
# Inspect metadata and scan peak/RMS levels
wavlab metadata --full input.wav

# Canonical British spelling; normalize remains an alias
wavlab normalise-db input.wav -6 output.wav

# Mix is normalised from 0 to 1; tail duration is an explicit option
wavlab digital-delay-mono input.wav 250 0.45 0.35 output.wav --tail-ms 2000

# Independent stereo chorus settings
wavlab chorus-stereo input.wav 0.5 0.8 1.0 1.2 0.5 output.wav

# Three-stage phaser is the default; override it explicitly when wanted
wavlab phaser-mono input.wav 0.8 0.5 1000 triangle 0.5 output.wav --stages 6
```

The complete generated reference is in [docs/cli.md](docs/cli.md). A test
fails if that checked-in document differs from the parser definitions.

## Features

- PCM WAV and BWF/BEXT v0 reading and writing
- Metadata inspection with optional peak and RMS analysis
- Gain, normalisation, reversal, phase inversion, and channel swapping
- Stereo and mid/side splitting and recombination
- Mixing, joining, and time-based splitting
- Direct and additive waveform generation plus white and pink noise
- Peaking, shelving, all-pass, band-pass, low-pass, and high-pass biquads
- Tape saturation; digital, analogue-style, and tape-style delays
- Schroeder reverb with independent per-channel state
- Chorus, vibrato, flanging, tremolo, and automatic panning
- Configurable phasing and LFO-modulated low/high-pass filters

Effects validate complete settings before replacing input samples. Stereo
operations use independent channel state, and commands that accept an omitted
output path overwrite the input deliberately.

## Library

Public headers are under `include/wavlab`. The CLI links the library but the
library has no dependency on CLI11, terminal output, argument parsing, or
process exit codes.

```cpp
#include <wavlab/Tools.h>
#include <wavlab/WavFile.h>

auto wave = wavlab::WavFile::open("input.wav");
wavlab::Tools::normaliseToDecibels(wave, -6.0);
wave.save("output.wav");
```

Installation and exported-package support are planned for the packaging phase;
the build-tree target is currently the supported library-consumer interface.

## Current limitations

- Integer PCM WAV only; floating-point WAV and compressed codecs are not
  supported.
- BEXT writing currently targets version 0.
- Unknown chunks are read safely but are not all preserved when saving.
- Sample-rate and bit-depth conversion, dynamics, fades, and ADSR processing
  are not implemented yet.

## Licence

Copyright (C) 2026 Christopher Clayton.

This project is free software licensed under the
[GNU General Public License version 3 or later](LICENSE).
