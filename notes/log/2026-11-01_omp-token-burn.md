# 2026-11-01 — OMP / Claude token burn forensics

- **Date:** 2026-11-01
- **Done:** Measured why a single `clock-project` OMP session nearly exhausted a 5h Claude window in about an hour; applied repo + documented global mitigations.
- **Failed:** N/A (investigation). Prior session failure mode was prompt-cache thrash, not a single giant file read.
- **Fix:** Slim always-on `AGENTS.md`; move deep refs to `notes/agent-reference/`; disable always-on handoff scanning; add `token-hygiene` rule; trim project MCP; disable ADHD always-apply + headroom `session_archive` globally; harden `.gitignore`.
- **Why this part or library:** Thesis work is OMP-heavy; rate-window burn blocks firmware/hardware progress. Dense git history should record both the failure analysis and the config response.
- **Evidence:** `~/.omp/stats.db` session `…/2026-11-01T12-31-42-747Z_01a19718…` (~20.5M tokens, ~95% `cache_write`); details in [`notes/agent-reference/omp-token-hygiene.md`](../agent-reference/omp-token-hygiene.md).
