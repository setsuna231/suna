# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.9.0] - 2026-11-06
### Added
- Added `Makefile` for automated compilation.
- Isolated object files into `build/obj/`.
- Added `README.md`, `CHANGELOG.md`, and `SECURITY.md`.

## [0.8.0] - 2026-11-06
### Added
- Implemented `scribble` command to launch external editors.
- Added pre-creation logic to bypass Notepad's "create file?" prompt.

## [0.7.1] - 2026-11-06
### Fixed
- Patched `rummage` to support wildcard filename searches (e.g., `suna rummage *.c`).

## [0.7.0] - 2026-11-06
### Added
- Implemented `forage` with recursive directory traversal and content searching.

## [0.6.1] - 2026-11-06
### Added
- Implemented global glob expansion (`*`) for `nuke`, `shove`, `twin`, and `peek`.

## [0.6.0] - 2026-11-06
### Added
- Implemented `rummage` with directory listing.

## [0.5.0] - 2026-11-06
### Added
- Implemented `peek` with `--squint` line limiter and BOM stripping.

## [0.4.0] - 2026-11-06
### Added
- Implemented `twin` with directory targeting and file-only restriction.

## [0.3.0] - 2026-11-06
### Added
- Implemented `shove` with directory targeting and safety checks.

## [0.2.0] - 2026-11-06
### Added
- Implemented `poof` with file/dir creation and batch nuke support.

## [0.1.0] - 2026-11-06
### Added
- Implemented `nuke` with recycle bin, obliterate, safety checks, and dry-run.

## [0.0.0] - 2026-09-20
### Added
- Initial scaffolding, argument parser, dispatch table, and 8 subcommand stubs.