# ADR 0001: CLI compatibility during modernization

- Status: accepted for the modernization branch
- Date: 2026-11-05
- Applies to: the first release produced from `feature/cpp-modernization`

## Context

The original C++ command line predates the Rust implementation. The Rust CLI
improves naming, validation, units, path handling, and generated help, but a
few commands reuse C++ names with incompatible argument semantics. In
particular, existing delay commands accept integer milliseconds, feedback and
mix as percentages, and an absolute maximum output length. Rust accepts
fractional milliseconds, a signed feedback ratio, a normalised mix, and an
additional tail duration.

Silently guessing which interpretation a caller intended would make scripts
unsafe. Keeping every historical semantic in the new library would also make
the refactor harder to reason about and test.

## Decision

The modernization release is an intentional CLI-breaking release. Its
commands, parameter names, and units will match `wavlab-rs` at the pinned
parity revision documented in [parity-baseline.md](../parity-baseline.md),
except that the C++ project uses the maintainer's preferred British `-ise`
spelling for canonical command and API names.

The following low-risk aliases will remain for one deprecation cycle:

| Legacy command | Canonical replacement |
|---|---|
| `stat` | `metadata` |
| `normalize` | `normalise` |
| `get-peak-db` | `metadata --full` |
| `get-rms-db` | `metadata --full` |

The Rust-compatible `normalize` and `normalize-db` spellings are also retained
as aliases. Aliases will be documented and covered by CLI tests. Their
removal will require a later, explicit compatibility decision.

Commands whose names stay the same but whose numeric semantics change will
not use value-based heuristics. The canonical syntax will use the new units.
If real users require the historical syntax, it will be exposed through an
explicit `--legacy-units` mode or a separately named compatibility command,
never inferred from a number's magnitude.

The modernized CLI will use these conventions:

- mix, depth, and room size are ratios in `[0, 1]`;
- feedback is a signed ratio with an effect-specific stable range;
- delay time and tail are milliseconds, with fractional delay values allowed;
- tail means additional output after the source, not total output length;
- reverb decay is RT60 in seconds;
- saturation and tape drive are decibels;
- command errors go to stderr and return a non-zero exit status;
- omitting the output path retains the established overwrite behavior, but
  the write must be failure-safe;
- split commands accept explicit output paths and retain derived paths as the
  default; and
- `--help` and generated Markdown come from the same command definitions.

The library API is not source- or binary-compatible with the old collection
of global types and namespaces. It is a new public boundary under namespace
`wavlab` with documented ownership, errors, and units.

## Consequences

- Existing scripts using delay, saturation, or reverb positionals need a
  migration described in the release notes.
- Compatibility is deterministic; no input changes meaning based on its
  numeric value.
- Library and CLI terminology can match the Rust reference and the tests.
- The old syntax remains recorded in
  [legacy-cli-contract.md](../legacy-cli-contract.md) for integration tests and
  migration documentation.

## Revisit point

Review this decision immediately before Phase 8 begins. Change it only with a
replacement ADR and corresponding updates to parsing tests and migration
documentation.
