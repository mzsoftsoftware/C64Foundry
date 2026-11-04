# Development Workflow

## Collaboration model

Development is performed collaboratively through discussion and repository-based documentation, while source changes are applied and committed by the repository owner.

The workflow uses two distinct kinds of conversation.

### 1. Decisions and work-package design

Use a dedicated chat to:

- Discuss architecture and technical alternatives.
- Inspect the current repository implementation.
- Agree responsibilities, interfaces and ownership.
- Define the scope and implementation order.
- Identify affected files and dependencies.
- Record accepted decisions and unresolved questions.

The result is a Markdown work-package document in `docs/work-packages/`.

Do not begin a broad implementation before the work package and its interfaces are sufficiently clear.

### 2. Implementation

Start a separate chat for implementation.

Provide the relevant work-package document and the current repository state. The implementation chat should:

- Inspect the actual current source files before proposing changes.
- Work only on the agreed scope.
- Provide complete files or clearly bounded changes as appropriate.
- Preserve existing behavior unless a change is explicitly part of the task.
- Avoid assuming that an earlier suggested implementation is already present.
- Respect the project's coding conventions.
- Proceed in buildable steps.

The repository owner applies the changes, builds, tests and commits them.

## Incremental implementation

- Prefer small, coherent, buildable steps.
- Compile after each agreed step.
- Run existing relevant tests where available.
- Do not introduce tests where no test infrastructure or relevant existing tests exist without first discussing that choice.
- Commit stable checkpoints regularly.
- Do not combine unrelated refactors with the current work package.

## Coding conventions

- Prefer explicit types and declarations; do not use `auto`.
- Do not use `std::` facilities in proposed project code.
- Do not use lambdas in proposed project code.
- Prefer readable, reasonably long lines over excessive line wrapping.
- Keep Qt and C++ dependencies at the lowest appropriate layer.
- Avoid unnecessary abstractions and forwarding layers.
- Keep cycle-sensitive code efficient and predictable.

These conventions should be followed unless a specific technical reason is discussed and accepted.

## Repository and implementation truth

The repository is the source of truth for the current implementation.

- Inspect current files before editing or proposing replacements.
- Distinguish implemented behavior from planned architecture.
- Do not assume the working tree or default branch is up to date without checking.
- Do not claim that a change has been applied, built or tested unless that has actually happened.
- The assistant does not directly modify, commit or push project source changes in this workflow.

## Work-package template

Each work package should include:

1. Title and status.
2. Goal and motivation.
3. Current implementation findings.
4. Agreed decisions.
5. Scope and explicit non-goals.
6. Affected components/files (verified against the current repository).
7. Implementation sequence.
8. Build/test/acceptance criteria.
9. Open questions and next step.

Update the work-package document when decisions change or implementation reveals new constraints. Once completed, mark it complete and retain it as project history if useful; create a new document for the next independent work package.
