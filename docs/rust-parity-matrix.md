# Rust parity and C++ defect inventory

This document records the audit behind the modernization plan. It is a living
checklist: update a row in the same pull request that implements and tests it.

Reference revisions:

- C++: `1e7deca`
- Rust: `bcbebe9`
- Audit date: 2026-11-05

The C++ working tree already contained a whole-file formatting edit to
`WavLab/WavFile.h` during the audit. The initial planning commit drops that
formatting churn but retains its useful self-contained `<cstdint>` include.

## Baseline summary

| Area | C++ baseline | Rust reference | Target |
|---|---|---|---|
| Build products | `wavlab::wavlab` library plus linked executable since Phase 1 | Library crate plus executable | Complete CLI-free public library |
| Build UX | Target-based CMake presets since Phase 1 | Standard Cargo workflow | Preserve the three-command preset workflow |
| Current build | Debug, Release, shared, warning-as-error, and sanitizer presets pass locally | `cargo test --all-targets` passes | Keep the supported-toolchain matrix clean |
| Automated tests | Phase 1 smoke, CLI, fixture, and header checks | 175 `#[test]` cases | Port behavior coverage, not necessarily test-for-test syntax |
| Library/CLI boundary | Command help/parsing and DSP share `.cpp` files | Reusable library with thin dispatch layer | No CLI dependencies in the library |
| Error behavior | Library constructors/functions print and call `exit` | Typed errors propagated to `main` | Typed C++ exceptions caught by CLI |
| Documentation | Hand-maintained long README command list | Generated CLI reference plus library docs | Generated CLI reference and public C++ API docs |
| Packaging | No install/export package | Cargo package model | Installable `wavlab::wavlab` CMake package |
| License file | GPL-3.0-or-later added in Phase 0 | GPL-3.0-or-later | Keep license and notices aligned |

## Feature parity checklist

Legend:

- **Present**: functionality exists, but it may still need the fixes below.
- **Missing**: no equivalent public C++ operation/command was found.
- **Different**: both exist but public semantics differ and require a decision.

### WAV model and common tools

| Capability | C++ | Rust behavior to port or decide | Status |
|---|---|---|---|
| Integer PCM 8/16/24/32 | Present | Validated mono/stereo layout and safe conversion | [ ] |
| Metadata-only load | Present | Samples absent but count/metadata retained | [ ] |
| Arbitrary RIFF chunk scan | Missing | Unknown chunks skipped safely in any supported order | [ ] |
| BEXT preservation | Partial | Preserve header/footer position and extra bytes | [ ] |
| Metadata report | Present as `stat` | Canonical `metadata`; optional `--full` peak/RMS scan | [ ] |
| Normalise to 0 dBFS | Present as `normalise` | Silence remains unchanged | [x] |
| Normalise to target dBFS | `normalise-db`; Rust-compatible alias | Target dBFS with unclamped in-memory samples | [x] |
| Linear and dB gain | Present | Explicit clipping-on-save behaviour | [x] |
| Peak and RMS | Separate commands | Silence and empty input are `-inf` | [x] |
| Reverse frames | Frame-aware | Preserve channel order and every frame | [x] |
| Flip polarity | Present | All interleaved samples | [x] |
| Swap channels | Present | Stereo swap; mono unchanged | [x] |
| Split stereo L/R | Complete | Optional explicit outputs or derived safe paths | [x] |
| Split stereo M/S | Complete | Optional explicit outputs or derived safe paths | [x] |
| Split at times | Frame-aware | Increasing millisecond points on frame boundaries | [x] |
| Combine L/R | Complete | Silence-pad unequal mono inputs | [x] |
| Combine M/S | Complete | Invert M/S transform and silence-pad | [x] |
| Mix | Checked | Minimum two, compatible rates, mono promotion, padding, normalise | [x] |
| Join | Checked | Minimum two, same rate/channels, maximum bit depth | [x] |

### Generators and EQ

| Capability | C++ | Rust behavior to port or decide | Status |
|---|---|---|---|
| Sine | Checked | Checked sub-second duration and frequency validation | [x] |
| Direct square/triangle/sawtooth | Phase-based | Defined phase/shape; reject invalid frequency | [x] |
| Additive square/triangle/sawtooth | Band-limited | Reject zero harmonics; omit harmonics at/above Nyquist | [x] |
| White/pink noise | Injectable RNG | Correct duration/channels; modern injectable RNG | [x] |
| Peak EQ | Validated | Bandwidth in octaves and validated settings | [x] |
| Low/high shelf | Validated | Q and validated settings | [x] |
| All/band pass | Validated | Bandwidth in octaves and validated settings | [x] |
| Low/high pass | Validated | Q and cutoff below Nyquist | [x] |
| Reusable per-sample biquad | Public `EQInstance` | Documented processor with independent state | [x] |

### Effects and DSP primitives

| Capability | C++ | Rust behavior to port or decide | Status |
|---|---|---|---|
| Delay-line primitive | Present | Fractional ms, checked ring buffer, modulation offset | [ ] |
| LFO | Partial sine-like absolute waveform | Sine/triangle/square with validated frequency | [ ] |
| Tape saturation | Present | Drive expressed in dB; finite validation | [ ] |
| Digital delay mono/stereo | Present | Signed feedback, normalised mix, additional tail | [ ] |
| Analog delay mono/stereo | Present | Configurable feedback low-pass cutoff | [ ] |
| Tape delay mono/stereo | Present | Configurable dB drive and cutoff | [ ] |
| Schroeder reverb | Present, algorithm differs | Parallel combs plus series all-pass, RT60 seconds | [ ] |
| Chorus mono/stereo | Missing | Common or independent channel settings | [ ] |
| Vibrato mono/stereo | Missing | Fully wet modulated delay | [ ] |
| Flanger mono/stereo | Missing | Modulated short delay and signed feedback | [ ] |
| Tremolo | Missing | Depth/rate/selectable waveform | [ ] |
| Autopan | Missing | Depth/rate/waveform, stereo output | [ ] |
| Phaser mono/stereo | Missing | Modulated all-pass stages, feedback, mix | [ ] |
| Modulated LPF/HPF | Missing | Frequency range, rate, waveform, Q, filter kind | [ ] |

## Confirmed or high-confidence C++ defect inventory

These items come from direct source inspection. Convert each item into a
focused failing test before changing the implementation.

### Build and portability

- [ ] `BinaryHelpers.cpp` and `Delay.h` rely on transitive declarations of
  fixed-width integer types. Public/private headers must include what they use.
- [ ] `CMAKE_EXE_LINKER_FLAGS` forces `-static` on every toolchain and target.
- [ ] The GCC conditional tests the C compiler variable and global flags are
  mutated for all consumers.
- [ ] The MSVC logic first requests `/W4` and then regex-replaces warning flags
  with `/w`, effectively disabling warnings.
- [ ] Raw integer reads/writes and unions assume a little-endian host and
  successful stream operations.

### WAV parsing, model, and serialization

- [ ] Loading overwrites `blockAlign` with bytes per sample instead of bytes
  per frame. Saving stereo files therefore writes an invalid block alignment.
- [ ] Newly created files do not initialize/calculate `byteRate`, chunk sizes,
  or all serialization invariants until late, and `validate()` does not check
  them.
- [ ] `numberOfSamples = dataChunkSize / blockAlign` only appears to work
  because the invalid overwritten `blockAlign` is bytes per sample. Frame and
  interleaved-sample concepts are conflated.
- [ ] 8-bit PCM is read/written as signed `char`; WAV 8-bit PCM is unsigned
  with 128 representing silence.
- [ ] `safeScaleDouble()` leaves its local result uninitialized for NaN.
- [ ] Fixed-size string serialization calls `write(value.c_str(), size)` even
  when `size` exceeds the string storage, which can read out of bounds. It
  must validate length and explicitly zero-pad.
- [ ] The parser assumes `fmt ` is first, handles only a narrow BEXT/JUNK/data
  sequence, and does not safely scan arbitrary unknown chunks.
- [ ] Footer seeking always adds one byte rather than adding padding only for
  an odd data size.
- [ ] Read operations do not detect truncated input before using values.
- [ ] BEXT reading skips a fixed 254 bytes rather than the declared remainder,
  footer behavior is incomplete, and saving hard-codes a 602-byte payload.
- [ ] BEXT RIFF size calculation omits the chunk header/padding, producing an
  inconsistent header.
- [ ] Saving does not check write failures, size overflow, incomplete frames,
  or whether samples were loaded.
- [ ] `setNumberOfChannels()` permits zero, does not update layout fields, and
  can make existing interleaved samples invalid.
- [ ] `setSamples()` accepts partial multichannel frames.
- [ ] `getSamples()` copies the entire buffer on every call, obscuring whether
  mutation is intended and causing unnecessary allocations.
- [ ] File/open/validation failures terminate the process from inside the data
  type, preventing safe library use and testing.

### Editing tools

- [x] Normalising an empty or silent file divides by zero and can create NaN.
- [x] Stereo reverse starts at the last frame but stops before copying the
  first frame; empty input also risks unsigned underflow.
- [x] Several loops use `size() - 1` as a bound and are unsafe for empty input.
- [x] Left/right and mid/side combine loop to `max_size - 1`, dropping the
  final mono sample and underflowing for empty inputs.
- [x] Mix uses `samples.size() <= maxSamples` as its indexing condition. That
  is true for every input and indexes beyond shorter inputs instead of adding
  silence.
- [x] Mix normalises unconditionally, including silent output.
- [x] Split converts time using `sample_rate / 1000`, losing precision for
  rates not divisible by 1000.
- [x] Split checks the boundary after appending the current frame, producing
  off-by-one segment placement.
- [x] Split allocates vectors with `new` and never deletes them.
- [x] Split does not validate sorted, unique, in-range points or avoid empty
  segments.
- [x] Join permits mismatched channel counts even though concatenating their
  interleaved buffers under one output layout is invalid.
- [x] Derived names use global string replacement of `.wav`, mishandling
  uppercase/no extension and directory components containing that substring.

### Generators and filters

- [x] All generators calculate frames with integer `lengthMs / 1000`, so
  durations below 1000 ms produce no samples and other durations truncate to
  whole seconds.
- [x] Frequency, sample rate, length, bit depth, and harmonic counts are not
  comprehensively validated before constructing or mutating output.
- [x] Direct waveforms can divide by zero for zero/too-high frequencies or
  degenerate period sizes.
- [x] Additive generators calculate harmonics beyond Nyquist and normalise
  even if output is silent/empty.
- [x] Filter constructors accept zero/non-finite Q, invalid frequency, and
  non-finite gain, which can create NaN/Inf coefficients.
- [x] Peak/all-pass/band-pass parameters are called Q even though the formula
  treats them as bandwidth in octaves; the public contract is ambiguous.

### Delay, saturation, and reverb

- [ ] A zero or sub-sample delay can allocate an empty buffer and then index
  it. Delay times larger than the maximum can also produce invalid unsigned
  position arithmetic.
- [ ] Delay setters do not validate settings or provide the strong guarantee
  that state is unchanged on error.
- [ ] Existing delays accept integer milliseconds only; the Rust API supports
  fractional milliseconds rounded to samples.
- [ ] Delay command units use 0-100 percentages while library internals use
  0-1; validation is largely CLI-only and can be bypassed by library callers.
- [ ] Delay/reverb tail input is described as a maximum output length, whereas
  Rust uses an explicit additional tail. The current default also means no
  tail beyond the original duration.
- [ ] Peak tracking for delay/reverb compares signed samples instead of
  absolute amplitude, so all-negative material produces the wrong reference.
- [ ] Effects auto-normalise their output afterward, changing the requested
  wet/dry behavior and hiding actual gain behavior.
- [ ] Stereo analog/tape feedback processing multiplexes left/right state with
  a shared call counter rather than binding a processor directly to each
  channel; this is fragile when channel routing changes.
- [ ] The LFO uses `abs(sin(...))`, resets its phase counter once per second,
  and derives frequency from an opaque input formula rather than a documented
  Hz setting.
- [ ] Reverb divides by zero at zero time and its final two stages are feedback
  comb delays rather than Schroeder all-pass filters.
- [ ] Reverb's final stages are shared by both stereo channels, causing channel
  crosstalk/state coupling.

### CLI and maintainability

- [ ] Help, parsing, file I/O, validation, DSP invocation, and user messages
  live in the same large `.cpp` files.
- [ ] Argument parsing and validation are duplicated for nearly every command.
- [ ] Help text is hand-maintained in both code and README and can drift.
- [ ] Command registration, lookup, help rendering, and dispatch are custom
  infrastructure that must be maintained in parallel with every handler.
- [ ] No black-box tests protect exit codes, stderr/stdout, overwrite behavior,
  or path derivation.

## Rust test groups to port

The Rust reference contains 175 tests. Port the contracts by group rather than
copying implementation details.

| Group | Reference coverage | C++ destination | Status |
|---|---|---|---|
| Binary I/O | Endianness, signed 24-bit, truncation | `tests/unit/wav_binary_io_test.cpp` | [ ] |
| Sample conversion | Limits, round trips, clamp, NaN | `tests/unit/sample_conversion_test.cpp` | [ ] |
| WAV model/I/O | Construction, layout, fixtures, metadata-only, BEXT | `tests/unit/wav_file_test.cpp`, integration round trips | [ ] |
| Level/gain/normalise | Sign, silence, dB, clipping policy | `tests/editing_tools_tests.cpp` | [x] |
| Frame transforms | Reverse, flip, swap | `tests/editing_tools_tests.cpp` | [x] |
| Split/combine/mix/join | Padding, promotion, validation, boundaries | `tests/editing_tools_tests.cpp` | [x] |
| Generators | Shapes, duration, channels, invalid settings | `tests/generator_tests.cpp` | [x] |
| EQ | Coefficients, state, impulse behaviour | `tests/eq_tests.cpp` | [x] |
| Delay/LFO primitives | Timing, wrap, feedback, waveforms, validation | focused DSP primitive tests | [ ] |
| Existing effects | Saturation, three delays, reverb | focused effect tests | [ ] |
| New effects | Chorus, vibrato, flanger, tremolo, autopan, phaser, modulated filter | focused effect tests | [ ] |
| CLI parsing | Every command and argument shape | parser unit tests | [ ] |
| CLI execution | Output samples, overwrite/default paths, errors | process-level integration tests | [ ] |
| Generated docs | Checked-in reference matches parser | generated-doc test | [ ] |

## Behavior decisions that must be explicit

These are not safe to resolve accidentally during implementation.

### CLI naming

Canonical C++ names use the maintainer's preferred British spelling
(`normalise`, `normalise-db`). Preserve Rust-compatible `normalize` and
`normalize-db` aliases. The later CLI replacement will decide the separate
`metadata`/`stat` and level-reporting aliases.

### Effect units

Recommended library and new CLI units:

- gain and EQ gain: dB where named as dB;
- saturation/tape drive: dB;
- feedback: signed ratio, normally `-1 < feedback < 1`;
- mix/depth/room size: normalised ratio in `[0, 1]`;
- delay and tail: milliseconds, allowing fractional delay time;
- reverb decay: RT60 seconds;
- frequency: Hz;
- peak/band/all-pass width: bandwidth in octaves;
- low/high-pass and shelves: Q.

Existing delay command names currently use percentage positionals. Do not
silently change how the same input is interpreted in a compatible release.
Use a major release, explicit flags, or an explicit legacy mode.

### Tail semantics

Use an additional tail duration. For an input of `N` frames and tail of `T`
frames, output contains `N + T` frames. This is composable and avoids the
current ambiguity where a "maximum" shorter than the input has no useful
meaning.

### Mixing and clipping

Match the Rust reference initially: average inputs, silence-pad shorter inputs,
promote mono to stereo when required, then normalise the result. Keep ordinary
gain/effect operations unnormalised and clamp only during integer encoding.
If a different mixer policy is desired, add it as an explicit option rather
than changing hidden gain behavior.

### Unknown RIFF chunks

Match the documented Rust limitation initially: skip unknown chunks safely on
read but do not reproduce them on save. BEXT is the supported preservation
exception. General unknown-chunk preservation can be a later feature with its
own memory/ordering policy.

### Compatibility comparison

Cross-language parity tests should compare decoded samples and metadata, not
exact output bytes. Encoders may legally choose different but equivalent
chunk layouts or rounding at half-LSB boundaries. Any deliberate algorithmic
difference should have a documented rationale and C++-native tests.

## Completion tracking

When a row is completed:

1. Link the implementing pull request or commit next to the checkbox.
2. Name the regression/parity tests added.
3. Record intentional differences from Rust.
4. Update generated CLI/API documentation if the public contract changed.
5. Run the affected unit tests, CLI integration tests, and sanitizer preset.

The matrix is complete only when every unchecked row is implemented,
explicitly declined with a reason, or moved to a separately approved backlog.
