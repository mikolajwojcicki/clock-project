## Why

The repository currently contains two overlapping workflow systems. OpenSpec is initialized and available, but GitHub Spec Kit remains embedded through `.specify/`, Spec Kit Cursor skills, and stale references. Keeping both creates ambiguous authoring paths and leaves obsolete framework state in the project.

## What Changes

- Remove the `.specify/` Spec Kit directory and its scripts, templates, manifests, and workflow metadata.
- Remove all `.cursor/skills/speckit-*` skills.
- Preserve project principles in an OpenSpec-compatible project document or configuration context.
- Keep and validate the existing OpenSpec configuration and Cursor command/skill integrations.
- Remove unintended Spec Kit references from active project guidance.
- Add migration verification covering files, references, OpenSpec status, and repository hygiene.

This is a tooling and documentation migration. It does not change clock product behavior or introduce a product capability.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

None.

This change sets `skip_specs: true` because it has no spec-level behavior changes.

## Impact

- Delete `.specify/`.
- Delete `.cursor/skills/speckit-*`.
- Add or update OpenSpec project context/guidance.
- Validate `.cursor/commands/opsx-*`, `.cursor/skills/openspec-*`, and `openspec/config.yaml`.
