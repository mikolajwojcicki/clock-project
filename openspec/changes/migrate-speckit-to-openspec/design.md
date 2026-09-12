## Context

The repository has OpenSpec configuration and Cursor integrations alongside a complete Spec Kit installation. There are no existing OpenSpec capability specs, and this change has no product or runtime behavior requirements.

## Goals / Non-Goals

**Goals:**

- Make OpenSpec the only structured change-management workflow in the repository.
- Preserve project principles and useful workflow guidance without retaining Spec Kit runtime dependencies.
- Leave existing OpenSpec commands, skills, and configuration usable.
- Make removal auditable through repository-wide checks.

**Non-Goals:**

- Change firmware, hardware, web, or thesis behavior.
- Redesign OpenSpec commands or skills.
- Remove generic project documentation merely because it mentions planning or requirements.

## Decisions

### Remove Spec Kit as one complete unit

Delete `.specify/` and every `.cursor/skills/speckit-*` skill. These files are coupled through Spec Kit manifests, scripts, templates, and `.specify` path assumptions; retaining fragments would preserve an ambiguous workflow.

Alternative considered: leave the old skills as compatibility shims. Rejected because the objective is to remove Spec Kit, not hide it behind aliases.

### Preserve principles outside framework-owned state

Move the project constitution content into OpenSpec-compatible project guidance, using `openspec/config.yaml` context and/or a clearly named repository document as appropriate. Remove framework-specific command references while preserving architecture and contribution constraints.

Alternative considered: discard the constitution with `.specify/`. Rejected because it contains canonical product and engineering decisions needed by future work.

### Treat migration as tooling-only

Set `skip_specs: true` for this change. No OpenSpec capability spec is created because the migration changes authoring infrastructure, not observable clock behavior.

Alternative considered: create a synthetic workflow capability spec. Rejected because it would misrepresent tooling changes as product requirements.

### Verify absence and usability

Use repository searches to confirm no unintended `speckit`, `.specify`, or Spec Kit references remain. Run OpenSpec validation/status commands and inspect the final Git diff to confirm OpenSpec artifacts remain intact.

## Risks / Trade-offs

- [Risk] Useful project principles could be lost during deletion → Preserve and review constitution content before removing `.specify/`.
- [Risk] Hidden Spec Kit references could keep stale workflow paths alive → Search tracked and working-tree files after migration, excluding only historical Git metadata.
- [Risk] OpenSpec may reject a no-spec change → Keep `skip_specs: true` in the change metadata and run `openspec validate`.

## Migration Plan

1. Extract reusable principles from `.specify/memory/constitution.md` into OpenSpec-compatible guidance.
2. Remove Spec Kit directories and skills.
3. Update any active references to use OpenSpec commands and paths.
4. Validate OpenSpec artifacts and search for leftovers.
5. Commit the migration as a focused repository workflow change.

Rollback consists of restoring the deleted Spec Kit files from Git history and reverting guidance edits; no runtime data or external service migration is involved.
