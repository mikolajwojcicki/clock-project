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
- Project MCP trimmed: no whole-repo `filesystem` server by default; no secrets in tracked `mcp.json`
- `.gitignore` hardened against dump landmines

## Global OMP tweaks (live under `~/.omp`, not this git repo)

Documented here so thesis/history still records them. Re-apply after machine rebuild if needed.

| Knob | Path | Intended value | Why |
| --- | --- | --- | --- |
| ADHD rule | `~/.omp/agent/rules/adhd.mdc` | `alwaysApply: false` | Invoke with `/i-have-adhd`; restating state every turn grows context |
| Headroom archive | `~/.omp/agent/headroom.yml` | `session_archive: false` | Prefix rewriting is the same failure class as cache thrash; keep tool compression on |
| Thinking | session `/thinking` | prefer low/off for long apply loops | Medium thinking was ~8× visible text in the killer session |
| Global MCP | `~/.omp/agent/mcp.json` | keep `git` / `context-mode`; enable `pdf-mcp` only when needed | Tool schemas inflate the ~18k baseline |

Reload after edits: restart OMP, or `/reload-plugins` then `/headroom config` as appropriate.

## Optional project MCP (enable only when needed)

Tracked [`.omp/mcp.json`](../../.omp/mcp.json) stays lean. Recipes:

**Saleae Logic 2** (URL; harmless if server down):

```json
"Saleae Logic 2": { "url": "http://127.0.0.1:10530" }
```

**Overleaf** — project [`.omp/mcp.json`](../../.omp/mcp.json) starts the server **without** embedding a token.
Paste the new Git token only into the server’s local env file (gitignored):

```text
/opt/repos/overleaf-mcp-server/.env
```

```env
OVERLEAF_TOKEN=paste_new_token_here
PROJECT_ID=6a5d22821d4854c9755706fd
```

Then restart OMP. Do **not** put `olp_…` into tracked `.omp/mcp.json`.

**filesystem** — avoid scoping to `./` on this monorepo (`notes/measurements` alone is hundreds of MB). If required, point at a narrow subdirectory.

## Operator habits that matter more than config

1. Fresh OMP session after a long `/opsx-apply` / multi-skill OpenSpec chain
2. If headroom/widget shows huge `cache write` and stuck low `cache read` → compact or restart immediately
3. Do not `read` multi-MB datasheet text dumps; search/slice instead
4. Prefer `context-mode` / headroom compress for large tool outputs
