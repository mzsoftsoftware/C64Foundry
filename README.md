# C64Foundry Documentation

This directory contains the project's technical architecture documentation and development work packages.

## Architecture

- [Architecture overview](architecture/ARCHITECTURE.md) — high-level application and emulator structure.
- [IEC architecture](architecture/IEC.md) — bus, central protocol, generic device routing, Foundry Drive/disk ownership and event flow.
- [Emulation execution](architecture/EMULATION.md) — controller, worker, factory, IEC system and scheduler responsibilities.

## Development

- [Development workflow](DEVELOPMENT.md) — collaboration, implementation and documentation workflow.

## Current work

- [IEC architecture refactoring](work-packages/IEC-REFACTOR.md) — current implementation status, agreed decisions, remaining sequence and next step.

## Historical work packages

- [IECSystem](work-packages/IEC-SYSTEM.md) — completed work package retained as implementation history. The current architecture and remaining work are documented in `architecture/IEC.md` and `work-packages/IEC-REFACTOR.md`.

## Keeping these documents current

Architecture documents describe stable responsibilities and agreed design decisions. Work-package documents describe implementation progress, scope and the next actionable step.

When an architectural decision changes, update the relevant architecture document and the current work-package document. Do not leave contradictory historical proposals in active documentation. Completed work packages may be retained as history, but must be clearly marked as completed and must point to the current source of truth.
