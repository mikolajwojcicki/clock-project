# OMP / Claude token hygiene (on-demand)

## What burned the 5h window (2026-11-01)

Main session under `clock-project` (`01a19718…`, OpenSpec BLE apply, ~13:32–18:52):

| Metric | Value |
| --- | --- |
| Total tokens | ~20.5M |
| First-hour tokens | ~11.8M |
| Dominant cost | **~95% `cache_write`** (~$57 of ~$60 API-equivalent) |
| Sticky baseline `cache_read` after thrash | ~17,986 (system + tools + always-on rules) |

**Mechanism:** long medium-thinking agentic loop → OMP `thinking_dropped` / `prefix_binding_mismatch` rewrites prior turns → Anthropic prompt cache only hits the small system prefix → **every turn re-writes 50–110k tokens as `cache_write`**.

Secondary same-day burn: `scratch/battery-and-charger` (~6.2M) from fat `web_fetch` / `grep` / `read` dumps.

## Project mitigations (tracked in this repo)

- Slimmer always-on [`AGENTS.md`](../../AGENTS.md); deep material under `notes/agent-reference/`
- `handoff` rule no longer `alwaysApply` (invoke when needed)
- Short always-on [`.omp/rules/token-hygiene.mdc`](../../.omp/rules/token-hygiene.mdc)
- Project MCP keeps optional servers with `"enabled": false` (OMP does not connect them → no tool-schema baseline cost)
- No secrets in tracked `mcp.json`; `.gitignore` hardened against dump landmines

## Global OMP tweaks (live under `~/.omp`, not this git repo)

Documented here so thesis/history still records them. Re-apply after machine rebuild if needed.

| Knob | Path | Intended value | Why |
| --- | --- | --- | --- |
| ADHD rule | `~/.omp/agent/rules/adhd.mdc` | `alwaysApply: false` | Invoke with `/i-have-adhd`; restating state every turn grows context |
| Headroom archive | `~/.omp/agent/headroom.yml` | `session_archive: false` | Prefix rewriting is the same failure class as cache thrash; keep tool compression on |
| Thinking | session `/thinking` | prefer low/off for long apply loops | Medium thinking was ~8× visible text in the killer session |
| Global MCP | `~/.omp/agent/mcp.json` | `git` + `context-mode` on; `pdf-mcp` present with `enabled: false` | Opt-in PDF tools without deleting the recipe |

Reload after edits: `/mcp reload`, or restart OMP. Headroom: `/reload-plugins` then `/headroom config` as needed.

## MCP on/off workflow (official OMP pattern)

Per [omp MCP docs](https://omp.sh/docs/mcp): `"enabled": false` means **do not connect**; `/mcp enable` / `/mcp disable` flip that without deleting config.

| Default | Scope | Server | How to use when needed |
| --- | --- | --- | --- |
| on | project | Saleae Logic 2 | needs Logic 2 listening on `:10530` |
| on | project | overleaf | token only in `/opt/repos/overleaf-mcp-server/.env` |
| **off** | project | filesystem | `/mcp enable filesystem` → work → `/mcp disable filesystem` (scoped to `clock-project-latex/`, not repo root) |
| on | user | git, context-mode | always-on lean tools |
| **off** | user | pdf-mcp | `/mcp enable pdf-mcp` → work → `/mcp disable pdf-mcp` |

Check state: `/mcp list`. After editing JSON by hand: `/mcp reload`. Prefer a **fresh session** when enabling fat servers.

**Overleaf token** (never in tracked `mcp.json`):

```text
/opt/repos/overleaf-mcp-server/.env
```

```env
OVERLEAF_TOKEN=paste_new_token_here
PROJECT_ID=6a5d22821d4854c9755706fd
```

If you need filesystem on another subtree, edit the abs path under `filesystem.args` in [`.omp/mcp.json`](../../.omp/mcp.json), keep it narrow, then `/mcp reload`.

## Operator habits that matter more than config

1. Fresh OMP session after a long `/opsx-apply` / multi-skill OpenSpec chain
2. If headroom/widget shows huge `cache write` and stuck low `cache read` → compact or restart immediately
3. Do not `read` multi-MB datasheet text dumps; search/slice instead
4. Prefer `context-mode` / headroom compress for large tool outputs
