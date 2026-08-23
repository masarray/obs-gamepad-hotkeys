# Smart Windows Installer

The release installer is built with Inno Setup and is designed so end users do not need to understand OBS plugin folder layouts.

## Product experience

The installer is intentionally branded as **OBS Gamepad Hotkeys**, not as a generic setup executable.

- Inno Setup modern Windows 11 visual style with automatic light/dark adaptation.
- Branded welcome page explaining native gamepad control for OBS Studio.
- Repository-authored `gamepad.jpg` as the installer artwork source.
- High-quality bicubic scaling while preserving the complete authored image.
- No generated text overlay on top of `gamepad.jpg`.
- Ready page showing selected OBS mode, OBS location, plugin target, and default `B` / `START` controls.
- Actionable OBS-running message instead of a generic file-in-use error.
- Finish option that launches OBS Studio and points the user toward **Tools > Gamepad Hotkeys**.

The generated wizard PNGs live only under `installer/generated/` during a build and are ignored by git.

## Target-selection contract

v0.1.7 makes target selection explicit and multi-instance aware.

A detected Standard OBS installation is useful information, but **must never prevent the user from selecting another Portable/custom OBS tree**. The installer therefore always offers:

1. **Standard OBS Studio** — install as a system-wide third-party plugin.
2. **OBS Studio Portable / custom OBS folder** — install into a selected OBS root.

This is important on PCs where Standard OBS and one or more Portable OBS instances coexist.

The optional `/OBSROOT="..."` command-line argument intentionally selects a custom/portable OBS root without displaying the target-selection pages. The supplied folder must contain `bin\64bit\obs64.exe`.

## Standard OBS Studio

For Standard OBS Studio the installer writes:

`C:\ProgramData\obs-studio\plugins\obs-gamepad-hotkeys\bin\64bit\obs-gamepad-hotkeys.dll`

and module data to:

`C:\ProgramData\obs-studio\plugins\obs-gamepad-hotkeys\data`

This is the preferred third-party plugin structure documented by OBS for Windows.

Standard installs register an uninstall entry.

## OBS Studio Portable / custom root

For a selected OBS root the installer writes:

- DLL: `<OBS root>\obs-plugins\64bit\obs-gamepad-hotkeys.dll`
- Data: `<OBS root>\data\obs-plugins\obs-gamepad-hotkeys\...`

The installer validates that `<OBS root>\bin\64bit\obs64.exe` exists before copying files.

Portable/custom-root installs intentionally do not register a machine-wide Add/Remove Programs entry; the plugin files remain self-contained with that OBS tree.

When Setup launches a selected Portable target after installation it passes `--portable`, matching OBS's documented portable-mode launch mechanism. Users may also keep using an existing `portable_mode` / `portable_mode.txt` marker.

## Portable/manual ZIP contract

The release ZIP is named:

`obs-gamepad-hotkeys-<version>-windows-x64-portable.zip`

It is **not** a ProgramData plugin bundle. It is an OBS-root overlay intended to be extracted directly into the root of an OBS installation.

Archive layout:

```text
obs-plugins/
  64bit/
    obs-gamepad-hotkeys.dll

data/
  obs-plugins/
    obs-gamepad-hotkeys/
      locale/
        en-US.ini
      licenses/
        ...

INSTALL-PORTABLE.txt
```

After extraction, `obs-gamepad-hotkeys.dll` must **not** exist under `<OBS root>\bin\64bit`.

`scripts/package.ps1` includes staging guards, and GitHub Actions independently expands the final ZIP and verifies the required and forbidden paths before the artifact can be released.

## Automated portable installation

Setup accepts:

`OBS-Gamepad-Hotkeys-Setup-v<version>.exe /OBSROOT="D:\Portable\obs-studio"`

The root only needs to be a valid OBS tree containing `bin\64bit\obs64.exe`. A portable marker is not required because Setup launches that selected target with `--portable`.

## Running OBS

The installer checks whether `obs64.exe` is running immediately before files are copied. It does not force-kill OBS. If OBS is open, the user is asked to close it and retry Install.

## Local one-click build

Double-click:

- `BUILD-INSTALLER.cmd` — build the plugin + installer and select the resulting EXE in Explorer.
- `BUILD-INSTALLER-AND-RUN.cmd` — build the plugin + installer and immediately launch it.

The build script automatically downloads the pinned Inno Setup compiler when `ISCC.exe` is unavailable and verifies the download SHA-256 before use.

Visual Studio 2022 C++ tools and CMake are still required to compile the OBS plugin itself.

## GitHub Actions release

`.github/workflows/build-windows.yml` produces:

- `OBS-Gamepad-Hotkeys-Setup-v<version>.exe`
- `obs-gamepad-hotkeys-<version>-windows-x64-portable.zip`
- SHA-256 files for both

Before upload, CI expands the Portable ZIP and asserts:

- `obs-plugins\64bit\obs-gamepad-hotkeys.dll` exists.
- `data\obs-plugins\obs-gamepad-hotkeys\locale\en-US.ini` exists.
- `INSTALL-PORTABLE.txt` exists.
- `bin\64bit\obs-gamepad-hotkeys.dll` does not exist.
- legacy nested `obs-gamepad-hotkeys\bin\64bit\...` layout does not exist.

A tag push, or a main-branch release commit containing `[release]`, publishes the GitHub Release and attaches those artifacts.

## Production code signing

An unsigned EXE can still trigger Windows SmartScreen / Unknown Publisher even if the installer UI is clean. Production releases should be Authenticode-signed with a trusted code-signing certificate.

The GitHub workflow supports these repository secrets:

- `WINDOWS_CERTIFICATE_BASE64` — Base64 encoded `.pfx`
- `WINDOWS_CERTIFICATE_PASSWORD` — PFX password

On branch/tag pushes, when the certificate is present, the workflow signs the plugin DLL first, packages it, builds the installer, signs the installer, verifies signatures with `signtool`, and regenerates SHA-256 files. Pull-request builds never receive or use the signing certificate.
