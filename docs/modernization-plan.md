# WavLab C++ modernization plan

This is an implementation plan for bringing the C++ project to roughly the
same level of correctness, reuse, test coverage, documentation, and features
as `wavlab-rs`, while keeping the result idiomatic for C++.

The plan is based on these revisions:

- C++: `1e7deca`
- Rust: `bcbebe9`
- Audit date: 2026-11-05

See [rust-parity-matrix.md](rust-parity-matrix.md) for the detailed behavior,
bug, feature, and test inventory that feeds this plan.

## Decisions up front

### Keep CMake, but replace the current CMake design

Meson is entirely appropriate for C++: it supports C++ projects, library and
executable targets, tests, installation, and static/shared library selection.
It would be a reasonable choice for a new project.

For this repository, modern CMake is the lower-risk choice because CMake is
already present and is supported directly by Visual Studio and CLion. The
problem is not that CMake cannot model the project; the current file simply
models every source as one executable and applies global flags. Replacing that
with target-based CMake avoids combining an architecture rewrite with an
unnecessary build-system migration.

Use CMake 3.24 or later, CMake presets, CTest, install/export rules, and Ninja
for the recommended local workflow. Reconsider Meson only if maintainers
actively prefer its syntax and are willing to maintain two build definitions
during a transition.

References:

- <https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html>
- <https://mesonbuild.com/Build-targets.html>
- <https://mesonbuild.com/Unit-tests.html>

### Use C++20 for the refreshed API, with a defined C++23 upgrade gate

C++20 gives the library useful standard facilities such as `std::span`,
`std::numbers`, stronger type traits, and improved tooling support. C++23 is
published and is the next target, but the project will wait until production
`/std:c++23` support is available in stable MSVC and the complete compiler
matrix passes. The upgrade gate is recorded in
[development.md](development.md). The supported compiler matrix is enforced
in CI.

### Use exceptions at the library boundary, exit codes at the CLI boundary

Library code must never call `exit`, print to stdout/stderr, or parse CLI
arguments. It should return values normally and report failures with a small,
documented exception hierarchy derived from `std::runtime_error` and
`std::invalid_argument`. The executable catches those exceptions once,
formats a human-readable error, and returns a non-zero exit code.

This is the simplest dependency-free C++ equivalent of the Rust `Result`
design. A result type can be reconsidered later, but introducing one should
not block the architecture work.

### Treat the Rust implementation as the intended behavior, not as code to translate

Port its tests and externally visible rules. Reimplement algorithms in
idiomatic C++, retain useful C++ APIs where appropriate, and document any
intentional divergence. Do not compare encoded WAV files byte-for-byte when a
sample-level or metadata-level comparison expresses the real contract.

## Target architecture

The desired dependency direction is:

```text
wavlab CLI  --->  wavlab C++ library  --->  C++ standard library
tests       --->  wavlab C++ library
examples    --->  wavlab C++ library
```

The library must not depend on the CLI. A practical target layout is:

```text
wavlab/
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/
├── include/wavlab/
│   ├── wav_file.hpp
│   ├── error.hpp
│   ├── eq.hpp
│   ├── generators.hpp
│   ├── processing.hpp
│   └── effects.hpp
├── src/
│   ├── lib/
│   │   ├── wav_file.cpp
│   │   ├── wav_io.cpp
│   │   ├── sample_conversion.cpp
│   │   ├── tools/
│   │   ├── eq/
│   │   ├── generators/
│   │   └── effects/
│   └── cli/
│       ├── main.cpp
│       ├── cli.cpp
│       └── cli.hpp
├── tests/
│   ├── unit/
│   ├── integration/
│   └── fixtures/
├── examples/
└── docs/
```

Exact filenames can evolve. The important boundaries are public headers,
private implementation, CLI-only code, and tests.

Build targets should be:

- `wavlab::wavlab`: reusable library target and installed package target.
- `wavlab_cli`: internal CMake target whose output name is `wavlab`.
- `wavlab_tests`: one or more test executables linked only to the library,
  plus separate black-box CLI tests.
- `wavlab_warnings` and `wavlab_sanitizers`: private interface targets for
  development options; neither leaks flags to consumers.

Support both static and shared libraries through `BUILD_SHARED_LIBS`. Do not
force static executable linking.

## Delivery strategy

Implement the phases in order. Each phase should be a reviewable pull request
or a short series of commits. Keep the command-line executable usable at the
end of every merged phase. When moving an operation into the library, move its
tests first or in the same commit, then reduce the old command handler to a
thin adapter.

### Phase 0 — Record contracts and fixtures

Goal: make refactoring measurable before changing behavior.

Tasks:

1. Decide whether the next release is CLI-compatible or a major-version CLI
   cleanup. Record the decision in an ADR or `docs/cli-compatibility.md`.
2. Make Rust revision `bcbebe9` the initial parity reference. Pin a revision
   rather than comparing against a moving branch.
3. Copy or generate small, redistributable fixtures covering mono/stereo and
   8/16/24/32-bit PCM, empty data, odd-sized chunks, unknown chunks, BEXT
   before data, and BEXT after data.
4. Add a script or documented procedure that generates deterministic impulse,
   constant, ramp, and channel-identity inputs. Avoid committing large audio
   files.
5. Define numerical tolerances:
   - exact equality for integer/binary helpers and structural operations;
   - about `1e-12` for pure in-memory `double` calculations unless an
     algorithm needs a documented looser bound;
   - at most one encoded least-significant bit after a WAV round trip.
6. Capture the existing CLI names and help output for compatibility testing.
7. Add `LICENSE` after confirming that the C++ repository should use the same
   GPL-3.0-or-later license as the Rust repository.

Progress recorded on 2026-11-05:

- CLI compatibility is decided in
  [ADR 0001](decisions/0001-cli-compatibility.md).
- Reference revisions and numeric tolerances are pinned in
  [parity-baseline.md](parity-baseline.md).
- Historical command syntax and semantics are frozen in
  [legacy-cli-contract.md](legacy-cli-contract.md).
- Thirteen independently generated fixtures, their expected values, and a
  reproducibility manifest are checked in under `tests/fixtures/`.
- The repository now carries the same GPL-3.0-or-later license as `wavlab-rs`,
  following explicit owner confirmation.

Phase 0 status: complete.

Exit criteria:

- The parity revision, fixture provenance, tolerances, and compatibility
  policy are documented.
- Test data can be regenerated without the CLI implementation under test.

### Phase 1 — Establish a reliable build and test harness

Goal: one predictable local workflow on Linux, Windows, and macOS.

Tasks:

1. Fix self-contained includes first. The audited tree currently fails with
   GCC 16 because `BinaryHelpers.cpp` and `Delay.h` use fixed-width integer
   types without including `<cstdint>` themselves.
2. Replace global `CMAKE_CXX_FLAGS`, linker flags, and source lists with the
   target structure described above.
3. Remove unconditional `-static`. Make static/shared library selection an
   ordinary CMake option and leave runtime linkage to the toolchain/user.
4. Set `CXX_STANDARD`, warnings, include paths, and compile definitions on
   individual targets. Enable strict warnings without rewriting MSVC warning
   flags globally.
5. Add checked-in configure/build/test presets for:
   - `dev` (Debug, tests, compile commands);
   - `release` (Release, tests optional);
   - `asan-ubsan` where supported;
   - a Windows-friendly preset/configuration.
6. Integrate CTest and a pinned test framework. Catch2 is a good default. Make
   dependency resolution work through `find_package` with a pinned
   `FetchContent` fallback, or vendor the small dependency if offline builds
   are a priority.
7. Add smoke tests that build the library and executable, run `wavlab --help`,
   and link a tiny external consumer against `wavlab::wavlab`.
8. Add formatting and static-analysis entry points (`clang-format`, optional
   `clang-tidy`) without silently rewriting sources during a normal build.
9. Add CI for GCC and Clang on Linux and MSVC on Windows; add Apple Clang when
   a macOS runner is available. Cache dependencies, not build products.

Phase 1 implementation notes:

- The initial CTest smoke harness is dependency-free so a clean checkout can
  build and test offline. Pinning Catch2 (or another framework) is deferred to
  the first broad unit-test change rather than making the foundational build
  depend on a network fetch.
- The current mixed translation units are compiled into a transitional
  library target. Removing command handlers from the library is Phase 2.

Expected local workflow:

```console
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Exit criteria:

- A clean checkout builds the library and CLI and runs at least one test.
- CI exercises at least GCC, Clang, and MSVC.
- Public headers compile when included individually.
- No consumer inherits WavLab's warning or sanitizer flags.

### Phase 2 — Create the library boundary and error model

Goal: make every operation usable without `argc`, `argv`, files, or console
I/O.

Tasks:

1. Introduce namespace `wavlab`; remove new global classes and namespaces.
2. Move public API declarations to `include/wavlab/`. Keep implementation
   helpers private under `src/lib/`.
3. Replace `WavFile(path, bool)` with explicit operations such as:
   - `WavFile::open(path)`;
   - `WavFile::open_metadata(path)`;
   - `WavFile::create(channels, sample_rate, bit_depth)` or a validated
     constructor.
4. Replace all `exit` calls in library code with typed exceptions. Include the
   path, field, or invalid value in diagnostic messages.
5. Remove `std::cout`/`std::cerr` from the library. Return metadata values or a
   report object; format them in the CLI.
6. Give sample access explicit ownership semantics:
   - `std::span<const double> samples() const`;
   - `std::span<double> samples()` for fixed-size mutation;
   - `set_samples(std::vector<double>)` for replacement.
7. Enforce the invariant `samples.size() % channel_count == 0`. Expose both
   interleaved sample count and frame count with unambiguous names.
8. Make operation inputs const-correct. Multi-file operations should take
   `std::span<const WavFile>` or `std::span<const std::reference_wrapper<const
   WavFile>>`, not mutable vectors copied by value.
9. Add small settings types for multi-parameter effects rather than long
   positional parameter lists.
10. Add a minimal library example to `examples/` and compile it in CI.

Phase 2 status: complete.

Implementation notes:

- Public headers now live under `include/wavlab/`; CLI sources and private
  implementation helpers are separate.
- `WavFile::open`, `openMetadata`, and `create` replace the ambiguous legacy
  constructors. `WavError` carries a machine-readable category and a
  contextual diagnostic.
- Sample access uses fixed-size spans, replacement validates complete frames,
  and distinct sample/frame counts remove the old stereo ambiguity.
- Library code no longer prints or exits. The CLI catches exceptions once and
  remains responsible for diagnostics and exit status.
- Multi-input tools are const-correct, delay/reverb parameters use settings
  types, and `examples/normalise.cpp` compiles as a library-only consumer.
- The API suite covers creation errors, frame invariants, metadata-only reads,
  mutable spans, typed I/O errors, and a stereo save/open round trip.

Exit criteria:

- A library consumer can open, process, and save a WAV without including CLI
  headers.
- Invalid input never terminates the process from library code.
- The CLI is the only layer that prints user-facing output or chooses exit
  codes.

### Phase 3 — Rebuild and test WAV I/O

Goal: make the data model and serialization trustworthy before porting more
DSP.

Tasks:

1. Replace host-endian unions and raw integer writes with explicit
   little-endian byte helpers. Check every read/write and report truncated
   input.
2. Correct PCM conversion:
   - 8-bit PCM is unsigned with silence at 128;
   - 16/24/32-bit PCM is signed little-endian;
   - clamp out-of-range floating samples;
   - define NaN handling (the Rust reference converts it to silence);
   - cover both positive and negative full-scale asymmetry.
3. Parse RIFF chunks in a bounded loop rather than assuming a fixed order.
   Respect even-byte padding, skip unknown chunks safely, reject duplicates
   where appropriate, and reject chunks extending beyond the RIFF boundary.
4. Validate `RIFF`, `WAVE`, `fmt `, PCM format code, channel count, sample
   rate, bit depth, block alignment, byte rate, data-frame alignment, and
   representable sizes.
5. Calculate the layout from first principles:
   - bytes per sample = bits per sample / 8;
   - block align = channels * bytes per sample;
   - byte rate = sample rate * block align;
   - data bytes = interleaved sample count * bytes per sample.
6. Preserve BEXT data, including its original header/footer position and extra
   fields. Zero-pad fixed strings safely; never ask `ostream::write` to read
   beyond a string's storage. Serialize correct chunk headers and padding.
7. Keep metadata-only reads genuinely sample-free while retaining sample/frame
   counts.
8. Add overflow checks before allocation and before narrowing to RIFF's 32-bit
   sizes.
9. Write to a temporary file and atomically replace the destination when the
   CLI overwrites its input, so a failed save does not destroy the source.

Phase 3 status: complete.

Implementation notes:

- Checked stream helpers now encode and decode integer PCM explicitly in
  little-endian order, including signed 24-bit values.
- Floating-point conversion handles unsigned 8-bit PCM correctly, preserves
  full-scale asymmetry, rounds to the nearest integer, clamps out-of-range
  values, and maps NaN to silence.
- RIFF input is scanned as bounded, padded chunks. Unknown chunks are skipped;
  missing, duplicate, truncated, inconsistent, and partial-frame structures
  are rejected with typed errors.
- BEXT chunks retain their header/footer location and all unknown extension
  bytes. Fixed strings are trimmed when read and safely zero-padded when
  written.
- Serialization includes all chunk headers and odd-byte padding, checks every
  narrowing calculation, and writes through a temporary sibling before
  replacing the destination.
- Tests exercise every fixture, literal byte order, conversion boundaries,
  corrupt inputs, byte-identical BEXT and odd-data round trips, sanitizer
  builds, and Python's independent standard-library WAV reader.

Required tests:

- Little-endian helpers and signed 24-bit boundary values.
- Conversion boundaries, representative round trips, clipping, and NaN.
- Empty valid WAV and all four supported bit depths.
- Mono and stereo header calculations.
- Metadata-only open.
- Truncated/corrupt headers and samples.
- Unknown and odd-sized chunks.
- BEXT before and after `data`, including unknown extra bytes.
- Public save/open round trip using a temporary directory.

Exit criteria:

- Generated files are accepted by an independent WAV reader.
- All fixtures round-trip at the metadata/sample level.
- AddressSanitizer and UndefinedBehaviorSanitizer pass the I/O suite.

### Phase 4 — Port the core editing tools with regression tests

Goal: establish correct frame-aware operations and remove the CLI code from
`Tools.cpp`.

Implement and test in this order:

1. Level measurement, linear gain, dB gain, normalization, and new
   `normalize_to_db`.
2. Reverse by frames, polarity inversion, and channel swap.
3. Stereo to left/right and mid/side splits.
4. Left/right and mid/side combination with silence padding.
5. Split at increasing millisecond positions on frame boundaries.
6. Mix compatible files with mono-to-stereo promotion and silence padding.
7. Join files only when sample rate and channel count are compatible; choose
   the maximum input bit depth for output encoding.

Fix the audited defects as tests are introduced:

- Normalizing silence must leave it silent, not divide by zero.
- Stereo reverse must retain every frame and preserve L/R order.
- Empty files must not underflow `size_t` loop bounds.
- Pair-combination must not drop the last sample.
- Mixing must test `index < samples.size()` before indexing shorter inputs.
- Split conversion must not truncate `sample_rate / 1000`, include a frame in
  the wrong segment, leak heap allocations, accept duplicate/descending split
  points, or create accidental empty segments.
- Join must not concatenate mono and stereo interleaved data into a malformed
  output.
- Derived output filenames must preserve directories and extensions, and the
  CLI should accept explicit split destinations as the Rust version does.

Exit criteria:

- Each operation has unit tests for normal, empty, mono/stereo, and invalid
  cases where applicable.
- Library tests use in-memory `WavFile` values; only CLI integration tests use
  actual input/output paths.

### Phase 5 — Port generators and EQ behavior

Goal: correct the existing algorithms and make parameter validation uniform.

Generator tasks:

1. Centralize milliseconds-to-frame conversion using checked floating-point
   or wide-integer arithmetic. The current `length_ms / 1000` integer division
   produces empty output below one second and truncates fractional seconds.
2. Validate finite frequency in `(0, Nyquist)`, non-zero sample rate, supported
   bit depth, non-zero harmonic count, and representable output length before
   changing samples.
3. Implement sine, direct square/triangle/sawtooth, additive variants, white
   noise, and pink noise.
4. Omit additive harmonics at or above Nyquist.
5. Inject or seed the random engine in tests so duration/channel assertions
   are deterministic without asserting exact platform-specific noise values.
6. Preserve identical generated values across channels.

EQ tasks:

1. Keep a reusable per-channel biquad processor in the library.
2. Give each channel independent filter state.
3. Validate finite gain, positive Q/bandwidth, and center/cutoff frequency
   below Nyquist before mutating samples.
4. Preserve the Rust distinction between bandwidth-in-octaves for peak,
   band-pass, and all-pass filters and Q for low/high-pass and shelving
   filters. Name parameters accordingly.
5. Port coefficient and impulse-response tests for all-pass, band-pass,
   low/high-pass, low/high-shelf, and peak filters.

Exit criteria:

- Sub-second generation has the correct frame count.
- Invalid settings leave the input unchanged.
- Mono and stereo EQ results prove independent per-channel state.

### Phase 6 — Replace DSP primitives and stabilize existing effects

Goal: give delays, saturation, and reverb validated reusable foundations.

Tasks:

1. Replace the current delay indexing with a clear ring-buffer abstraction.
   Support fractional milliseconds rounded to the nearest sample, require at
   least one sample, validate maximum/current delay, and expose explicit wet,
   feedback, feedback-processor, and modulation-offset operations.
2. Add an LFO with sine, triangle, and square waveforms, finite positive
   frequency validation, normalized output, and stable phase progression.
3. Define shared conversion helpers for milliseconds/frames and dB/amplitude.
4. Port tape saturation with drive in dB and test negative, zero, and positive
   drive.
5. Port digital, analog, and tape delays:
   - signed feedback with magnitude below 1;
   - mix in `[0, 1]`;
   - fractional delay times;
   - independent stereo state;
   - mono-to-stereo promotion for stereo settings;
   - additional tail duration rather than ambiguous absolute output length;
   - configurable analog/tape low-pass cutoff and tape drive.
6. Replace reverb's final feedback delays with true Schroeder all-pass stages,
   use published delay times, interpret decay as RT60 seconds, and give every
   channel independent comb/all-pass state.
7. Remove post-effect auto-normalization. Wet/dry mix must not secretly change
   global level; clipping policy belongs to encoding or an explicit normalize
   operation.
8. Validate all settings before mutation and make failure preserve input.

Exit criteria:

- Impulse-response tests demonstrate expected delay positions, feedback
  decay, stereo independence, reverb comb/all-pass behavior, and tail length.
- Delay and reverb tests pass under sanitizers for zero/very short/large
  settings without invalid allocation or indexing.

### Phase 7 — Add Rust-only effects

Goal: reach feature parity after the shared delay, LFO, and EQ primitives are
stable.

Port in dependency order:

1. Chorus and vibrato.
2. Flanger, including signed feedback stability limits.
3. Tremolo.
4. Autopan, always producing stereo output.
5. Phaser with configurable stages and independent stereo settings.
6. LFO-modulated low-pass/high-pass filter.

For every effect:

- introduce a settings struct with documented units and valid ranges;
- validate the entire settings object before changing samples;
- test zero-depth/dry behavior as an identity where applicable;
- test the maximum documented depth/range;
- test selected LFO waveforms;
- test mono, ordinary stereo, and stereo settings applied to mono input;
- prove independent per-channel state;
- add a CLI integration test only after the library tests pass.

Exit criteria:

- Every Rust CLI effect has a C++ library operation and CLI route, or an
  explicitly documented intentional omission.
- The parity matrix contains no unexplained gaps.

### Phase 8 — Replace the command layer with a thin CLI

Goal: command code should only parse, call, format, and choose an exit code.

Tasks:

1. Use a maintained C++ argument parser such as CLI11, pinned through the same
   dependency strategy as the test framework. Do not rebuild another parser
   inside the audio library.
2. Put all command definitions and help text in the CLI target. `main.cpp`
   should parse once, dispatch once, catch once, and return once.
3. Match the Rust canonical command set and units where a CLI-breaking release
   is allowed. In particular, use normalized `[0, 1]` mix/depth values,
   seconds for RT60, dB for saturation drive, and an explicit additional tail.
4. If compatibility is required:
   - keep `stat` as an alias for `metadata`;
   - keep `normalise` as an alias for `normalize`;
   - retain peak/RMS commands as aliases for `metadata --full` or deprecate
     them visibly;
   - do not silently reinterpret old delay percentage positionals as normalized
     values. Provide an explicit legacy mode or make the semantic change part
     of a major release.
5. Add `--version`, top-level `--help`, command help, validation messages,
   consistent stderr use, and stable success/failure exit codes.
6. Generate `docs/cli.md` from the parser definitions and fail a test if the
   committed reference is stale.
7. Test parsing separately from execution. Add black-box tests for successful
   writes, omitted-output overwrite behavior, invalid arguments, missing
   files, and help/version output.

Exit criteria:

- No CLI handler contains DSP or WAV parsing logic.
- Library tests do not instantiate the argument parser.
- Generated CLI documentation and `--help` cannot drift apart.

### Phase 9 — Package, document, and harden

Goal: make the project pleasant to consume as both a library and a tool.

Tasks:

1. Add install rules for the executable, library, headers, license, and CMake
   package files. Export `wavlab::wavlab` and provide version/config files.
2. Verify a separate consumer project can use
   `find_package(wavlab CONFIG REQUIRED)` and link `wavlab::wavlab` after
   install.
3. Add CPack only if distributable archives/installers are wanted; keep it
   separate from basic build correctness.
4. Rewrite the README around:
   - project purpose and limitations;
   - three-command local build/test flow;
   - CLI examples;
   - CMake library-consumer example;
   - API documentation generation;
   - supported platforms/compilers.
5. Generate API docs with Doxygen only if they add value beyond documented
   public headers. Treat warning-free docs as CI-checkable output.
6. Add source references for DSP algorithms, matching or improving the Rust
   README.
7. Add release CI for a clean Release build, tests, install, consumer test,
   and archive inspection.

Exit criteria:

- Both build-tree and installed-package consumers work.
- A new contributor can build and test from the README alone.
- Release artifacts contain no test-only dependencies or private headers.

### Phase 10 — Final parity and release gate

Goal: prove the modernization is complete rather than merely rearranged.

Run the following gate:

1. All unit, integration, CLI, generated-doc, install-consumer, and sanitizer
   tests pass.
2. GCC, Clang, MSVC, and Apple Clang builds are warning-clean at the project's
   chosen warning level.
3. The parity matrix has been reviewed line by line.
4. Run selected C++ and Rust operations on identical generated inputs and
   compare decoded samples within the documented tolerance:
   - frame tools and combinations;
   - generators;
   - every EQ;
   - impulse responses for each delay/reverb/modulation effect.
5. Inspect generated WAVs with an independent tool such as `ffprobe` or
   `sox --i` in CI where available.
6. Confirm overwrite operations are recoverable on failure.
7. Confirm the CLI compatibility/deprecation policy and version number match
   the actual changes.
8. Publish a changelog separating fixed defects, API changes, CLI changes,
   new features, and known limitations.

## Test organization and coverage policy

Use behavior-focused tests rather than targeting a percentage alone:

- `tests/unit/wav_io_*`: byte helpers, conversion, parser, serializer.
- `tests/unit/tools_*`: in-memory transformations and validation.
- `tests/unit/eq_*`, `generators_*`, `effects_*`: algorithms and settings.
- `tests/integration/round_trip_*`: temporary files and real encoding.
- `tests/integration/cli_*`: process-level parsing, stdout/stderr, exit status,
  path behavior, and generated output.
- `tests/integration/package_consumer`: clean installed-library consumer.

Every defect fixed from the inventory must first receive a test that fails for
the intended reason. Every public operation should have at least:

- a representative success case;
- boundary/empty input where meaningful;
- invalid input and unchanged-state assertion;
- mono/stereo behavior where meaningful;
- a CLI smoke test if exposed by the executable.

Use sanitizers in addition to ordinary tests. Coverage reporting is useful for
finding unvisited branches, but it is not a substitute for the parity matrix
or carefully chosen DSP invariants.

## Recommended pull-request sequence

This sequence keeps reviews small and minimizes simultaneous behavior and
structure changes:

1. Build repair, target split, presets, empty test harness.
2. Error model and public `WavFile` skeleton.
3. Binary helpers and sample conversion tests/fixes.
4. RIFF/WAV/BEXT parser and serializer.
5. Levels/gain/normalize and frame transforms.
6. Split/combine/mix/join.
7. Generators.
8. EQ.
9. Delay/LFO primitives.
10. Saturation, delays, and reverb.
11. Chorus/vibrato/flanger.
12. Tremolo/autopan/phaser/modulated filter.
13. New CLI, generated CLI docs, and compatibility aliases.
14. Install/export, consumer test, documentation, and release gate.

Do not perform a repository-wide rename/format at the same time as a logic
change. Mechanical moves, formatting, behavioral fixes, and new features are
much easier to verify when they are separate commits.

## Definition of done

The modernization is complete when:

- the C++ project builds a reusable library and a CLI executable;
- library code has no dependency on CLI parsing, terminal output, or process
  termination;
- the public API has documented ownership, units, validation, and errors;
- the known defects in the parity matrix have regression tests and fixes;
- all shared and Rust-only features are implemented or explicitly declined;
- the test suite covers I/O, algorithms, CLI, and package consumption;
- local builds use presets and CI covers the supported toolchains;
- generated help/reference documentation cannot drift;
- the installed CMake package works from an external project; and
- the README accurately describes build, library, and CLI usage.
