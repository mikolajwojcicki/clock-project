# NixOS primer for agents (on-demand)

Read when proposing installs, services, or system changes.

## What NixOS is (for agents)

NixOS is not “Linux with a different package manager.” The running system is a build product of Nix expressions. Packages live in the Nix store (`/nix/store/...`) and are wired into profiles via symlinks. There is no `apt`, `dnf`, `pacman`, or `brew`-style install that permanently mutates the OS.

| Traditional distro habit | NixOS reality |
| --- | --- |
| `apt install foo` / `dnf install foo` | Add to `environment.systemPackages` (or Home Manager / a flake), then rebuild |
| Edit files under `/usr` or `/etc` permanently | Prefer declarative config in the flake; many `/etc` files are generated and overwritten on rebuild |
| “Put it in `/usr/local/bin`” | Put it in a Nix derivation, `environment.systemPackages`, or a user profile |
| Assume `/bin/bash`, `/usr/bin/env`, and FHS paths everywhere | Paths often resolve through `/run/current-system/sw` or nix-shell/devShell; use `#!/usr/bin/env` carefully and prefer Nix-provided interpreters |
| Install build deps globally to “just compile” | Use `nix-shell`, `nix develop`, or a project `flake.nix` / `shell.nix` |
| Enable a service by writing a unit and `systemctl enable` | Prefer `services.*.enable = true;` (or a custom systemd module) in NixOS config, then rebuild |

## Hard rules for this machine

1. **Never suggest imperative package managers** (`apt`, `yum`, `dnf`, `pacman`, `snap`, flatpak-as-default-install path) for system software.
2. **Never tell the user to hand-edit generated system files** as the durable fix. If a change must survive reboot/rebuild, it belongs in `~/thinkpad-nixos` (or a project flake).
3. **Prefer flakes and nix-shell/devShells** for project tooling. Do not pollute the global system with one-off compile dependencies when a shell can provide them.
4. **Rebuild is the activation step.** After changing NixOS modules/packages, the change is not live until `nixos-rebuild switch` (or equivalent) succeeds.
5. **Check what Nix already provides** before inventing wrappers, vendoring binaries, or downloading install scripts meant for other distros.
6. **Secrets and Wi-Fi credentials** under `~/thinkpad-nixos/secrets/` are sensitive — do not commit, print, or paste them into chat logs casually.

## Quick decision guide

- **Need a CLI tool once for development?** → `nix-shell -p <pkg>` or a project devShell.
- **Need a tool always on PATH for the user/system?** → add to Home Manager / `environment.systemPackages`, rebuild.
- **Need a background service / hardware integration?** → NixOS module in `~/thinkpad-nixos`, rebuild.
- **Need to debug “command not found”?** → check `which`, `type`, and whether you are inside a nix-shell/devShell; do not assume Ubuntu package names map 1:1.

## Anti-patterns to avoid

- Curl-bashing install scripts that assume glibc FHS layouts or write into `/usr`.
- Instructing `chmod +x` fixes for missing dynamic loaders without considering Nix packaging.
- Assuming toolchains from the OS image are complete; pin them in the project flake/shell instead.
- Treating `/etc/nixos` as the only config location — durable system config is `~/thinkpad-nixos`.
- Giant catch-all commits that bury PCB, firmware, and LaTeX changes together without need.
- Reintroducing deleted architecture (OLED-only offline STM32 clock) without an explicit user decision.
- Pulling in Zephyr/west or an RTOS “for convenience” — conflicts with the bare-metal power-efficiency thesis focus.
