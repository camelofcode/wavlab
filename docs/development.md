# Development

## Prerequisites

The normal local build has no third-party C++ dependencies. It requires:

- a C++20 compiler;
- CMake 3.24 or later;
- Ninja for the checked-in single-configuration presets; and
- Python 3 when tests are enabled, to verify generated fixtures.

The supported CI matrix is GCC and Clang on Linux, MSVC on Windows, and Apple
Clang on macOS. Phase 1 intentionally uses only standard-library facilities so
the build does not need network access.

On Fedora, the optional sanitizer build also needs:

```console
sudo dnf install libasan libubsan
```

The managed development environment traces child processes, so the
`asan-ubsan` test preset disables LeakSanitizer only. AddressSanitizer and
UndefinedBehaviorSanitizer remain enabled. Run leak checks in an untraced
environment when they are added to the test policy.

## C++ standard policy

The current baseline is C++20 with compiler extensions disabled. C++23 is a
published ISO standard and is well supported by current GCC and Clang, but
MSVC's production `/std:c++23` toolset (14.52) is still a Release Candidate as
of 2026-11-05. Visual Studio 2022 exposes C++23 only as a preview mode.

Move the project to C++23 after all of these are true:

1. MSVC 14.52 is available in the stable Visual Studio channel;
2. the GitHub Windows runner provides that stable toolset;
3. GCC, Clang, MSVC, and Apple Clang CI all accept CMake's `cxx_std_23` feature;
4. Debug, Release, shared-library, and sanitizer builds pass; and
5. the minimum compiler policy and README are updated in the same change.

Do not select C++26/29 draft modes for production targets. Draft modes may be
used in an experimental local preset that does not change the public build
contract.

References:

- <https://www.iso.org/standard/83626.html>
- <https://gcc.gnu.org/projects/cxx-status.html>
- <https://clang.llvm.org/cxx_status>
- <https://devblogs.microsoft.com/cppblog/msvc-build-tools-version-14-52-release-candidate-now-available/>

## Presets

Configure, build, and test the ordinary Debug tree:

```console
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Available configurations:

| Preset | Purpose |
|---|---|
| `dev` | Debug build, tests, and `compile_commands.json` |
| `release` | Optimized Release build with tests |
| `ci` | Debug build with compiler warnings treated as errors |
| `ci-shared` | Warning-clean shared-library build |
| `asan-ubsan` | GCC/Clang address and undefined-behavior sanitizers |
| `windows-msvc` | Visual Studio 2022 x64 Debug build |

`CMakeUserPresets.json` is ignored by Git and can contain machine-specific
presets that inherit from the checked-in configurations.

## Targets

- `wavlab_lib` builds the library, with alias `wavlab::wavlab`.
- `wavlab_cli_core` builds the internal typed CLI definitions and handlers.
- `wavlab_cli` builds the executable with output name `wavlab`.
- `wavlab_tests` is the dependency-free library consumer smoke test.
- `wavlab_header_smoke` compiles every public header in an isolated translation
  unit to prevent accidental transitive include dependencies.
- `format` rewrites modernized C++ sources using `clang-format` when it is
  installed.
- `format-check` reports formatting differences without modifying files.

CLI11 is confined to `wavlab_cli_core`. The reusable library has no command-line
parser, terminal-output, or process-exit dependency. CMake uses an installed
CLI11 2.7.2 config package when available, with a pinned `FetchContent`
fallback for clean builds.

## Build options

| Option | Default | Effect |
|---|---|---|
| `BUILD_SHARED_LIBS` | `OFF` | Build `wavlab` as a shared rather than static library |
| `BUILD_TESTING` | `ON` at the top level | Build smoke/header tests and register CTest tests |
| `WAVLAB_WARNINGS_AS_ERRORS` | `OFF` | Promote target-scoped project warnings to errors |
| `WAVLAB_ENABLE_SANITIZERS` | `OFF` | Enable ASan and UBSan with GCC/Clang |
| `WAVLAB_ENABLE_CLANG_TIDY` | `OFF` | Run the discovered `clang-tidy` during compilation |

Options, warnings, and sanitizers are target-scoped. They are not added to
global CMake flags and do not propagate to consumers of `wavlab::wavlab`.

## Tests

The library tests remain dependency-free and use CTest executables. CLI parser
tests link only the internal CLI target and therefore do not affect library
consumers.

Current tests verify:

- an external-style executable can link `wavlab::wavlab` and construct a WAV;
- the CLI builds and provides conventional `--help` output;
- every canonical command shape parses independently of execution;
- black-box CLI tests cover output, overwrite, errors, help, and version;
- `docs/cli.md` exactly matches the live CLI11 definitions;
- all generated WAV fixtures and their manifest are reproducible; and
- every public header compiles independently as part of the normal build.

List or run tests directly:

```console
ctest --preset dev --show-only
ctest --preset dev
```

Regenerate the checked-in command reference after changing a CLI definition:

```console
./out/build/dev/wavlab _generate-cli-docs docs/cli.md
```

`wavlab.cli.reference-current` compares this file with freshly generated
output, so an omitted update also fails locally and in CI.

## Static analysis and formatting

Enable `clang-tidy` without changing the ordinary build:

```console
cmake --preset dev -DWAVLAB_ENABLE_CLANG_TIDY=ON
cmake --build --preset dev
```

Formatting is always explicit:

```console
cmake --build --preset dev --target format-check
cmake --build --preset dev --target format
```

The targets initially cover new smoke-test code. Legacy files are added as
they are migrated so this phase does not disguise logic changes inside a
repository-wide reformat. Do not combine a broad formatting change with a
behavioral change.
