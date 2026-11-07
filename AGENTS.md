# WavLab C++ repository guidance

Use the repository skills under `.agents/skills/` whenever their descriptions
match the task:

- `wavlab-cpp-development` for library, CLI, test, build, or documentation
  changes.
- `wavlab-cpp-release` for version bumps, release preparation, tags, and
  GitHub Releases.

Keep the library/CLI boundary intact: public API belongs in `include/wavlab`,
private implementation in `src/lib`, and command-line concerns in `src/cli`.
Generated build products belong under `out/` and must not be committed.

Use British English in new prose and user-facing C++ names. Preserve existing
compatibility aliases and public spellings unless the task explicitly changes
the compatibility contract.

The `main` branch is protected. Make changes on a focused branch, open a pull
request, and require `Required C++ checks` to pass before merging. Prefer the
ordinary `dev` preset locally; leave the expensive compiler/platform matrix to
GitHub Actions.
