# Contributing to OBS Gamepad Hotkeys

Thanks for helping improve OBS Gamepad Hotkeys. The project aims to stay small, native, reliable, and easy for non-technical OBS users to install.

## Before opening a pull request

1. Search existing issues and pull requests first.
2. For bugs, include the OBS version, Windows version, controller model, and exact reproduction steps.
3. For behavior changes, explain the user problem before proposing the implementation.
4. Keep changes focused. Avoid unrelated formatting or refactors in the same pull request.

## Development setup

Requirements:

- Windows 10/11 x64
- Visual Studio 2022 with **Desktop development with C++**
- Windows SDK
- CMake 3.28+
- Internet access for the initial OBS/Qt dependency bootstrap

Build:

```powershell
.\scripts\bootstrap-template.ps1
cmake --preset windows-x64
cmake --build --preset windows-x64
```

Or use the one-click local scripts:

```text
BUILD-INSTALLER.cmd
BUILD-INSTALLER-AND-RUN.cmd
```

## Required validation

Before requesting review:

```powershell
python tools/static-check.py
cmake --preset windows-x64
cmake --build --preset windows-x64
```

GitHub Actions must also pass the Windows Smart Installer workflow.

When changing controller behavior, OBS routing, configuration migration, installer paths, or release packaging, describe the validation performed and any hardware/runtime validation that was **not** performed.

## Project constraints

Please preserve these design rules unless a change is explicitly discussed first:

- No keyboard emulation (`SendInput`, `keybd_event`, virtual shortcuts, or similar injection).
- Controller input remains background-capable and non-exclusive.
- OBS actions are routed through OBS-native hotkey/frontend APIs.
- Worker threads must not directly mutate OBS UI state.
- Existing user mappings must not be silently overwritten.
- Installer changes must preserve standard OBS and OBS Portable support.
- Normal users should not need to manually copy DLL files.

## UI changes

The UI should remain compact and native to OBS/Qt:

- use existing Lucide icon helpers where possible;
- support dark/light OBS palettes;
- avoid oversized cards, decorative clutter, or unnecessary modal steps;
- include tooltips/accessibility names for icon-only controls;
- keep the beginner workflow obvious without exposing internal implementation details.

## Pull request checklist

A good PR explains:

- what user problem it solves;
- what changed;
- why the approach is safe;
- how it was tested;
- whether documentation or the landing page needs updating;
- whether the change affects release artifacts or configuration compatibility.

Do not commit build output, installers, certificates, signing secrets, dependency caches, or local IDE state.

## Release changes

Do not create release tags manually from feature branches. Releases are built by the repository workflow from `main` and include the Setup EXE, manual ZIP, and SHA-256 manifests.

A normal merge to `main` does **not** publish a release. Release publication is intentionally explicit through the repository's release workflow conventions.

## License

By contributing, you agree that your contribution is provided under the repository's GPL-2.0-or-later license unless clearly stated otherwise for a separately licensed third-party asset.