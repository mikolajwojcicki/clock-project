## 1. Preserve Project Guidance

- [x] 1.1 Extract canonical project principles from `.specify/memory/constitution.md` into OpenSpec-compatible guidance; verify all locked architecture and workflow constraints remain represented.
- [x] 1.2 Remove Spec Kit-specific command, path, and framework references from preserved guidance; verify active guidance contains no unintended `speckit` or `.specify` references.

## 2. Remove Spec Kit

- [x] 2.1 Delete the `.specify/` directory and all Spec Kit scripts, templates, manifests, and workflow metadata; verify no `.specify` directory remains.
- [x] 2.2 Delete all `.cursor/skills/speckit-*` skills; verify only OpenSpec workflow skills remain under `.cursor/skills/`.

## 3. Verify OpenSpec Migration

- [x] 3.1 Validate OpenSpec configuration and change artifacts with `openspec validate --change migrate-speckit-to-openspec`.
- [x] 3.2 Search repository files for unintended `speckit`, `spec-kit`, and `.specify` references; verify only migration history or explicitly historical text remains.
- [x] 3.3 Verify OpenSpec status, commands, and skills remain discoverable and usable.
- [x] 3.4 Review final Git diff and repository status; verify migration contains no firmware, hardware, web, or thesis behavior changes.
