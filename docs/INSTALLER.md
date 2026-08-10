# Smart Windows Installer

The release installer is built with Inno Setup and is designed for users who should not need to know OBS plugin folder layouts.

## Product experience

The installer is intentionally branded as **OBS Gamepad Hotkeys**, not as a generic setup executable. The v0.1.5 polish includes:

- Inno Setup's modern Windows 11 visual style with automatic light/dark adaptation.
- A branded welcome page that immediately explains the plugin: native gamepad control for OBS Studio.
- The repository's authored `gamepad.jpg` as the installer artwork source.
- High-quality bicubic scaling during the build while preserving the complete authored sidebar image.
- **No generated text overlay on top of `gamepad.jpg`**. Any copy already present in the artwork remains the single source of truth.
- A small wizard image generated from the same artwork with no added copy.
- A Ready page that clearly shows the detected OBS mode, OBS location, plugin target, and the default `B` / `START` controls.
- An actionable OBS-running message instead of a generic file-in-use error.
- A Finish option that launches OBS Studio and points the user toward **Tools > Gamepad Hotkeys**.

The generated wizard PNGs live only under `installer/generated/` during a build and are ignored by git. `gamepad.jpg` is a required repository asset and CI fails if the image-based branding path is removed or if generated text is reintroduced over the artwork.

## Standard OBS Studio

The installer auto-detects a normal OBS Studio installation from common install locations and Windows uninstall registry entries. For standard OBS Studio it installs the plugin to:

`C:\ProgramData\obs-studio\plugins\obs-gamepad-hotkeys\bin\64bit\obs-gamepad-hotkeys.dll`

and module data to:

`C:\ProgramData\obs-studio\plugins\obs-gamepad-hotkeys\data`

This matches the extra third-party module path used by current Windows OBS builds (and OBS 31.1.1, which is the current build dependency baseline for this project).

## OBS Studio Portable

Portable OBS deliberately does not add the ProgramData third-party plugin path. The installer therefore uses the portable OBS root layout:

- DLL: `<OBS root>\obs-plugins\64bit\obs-gamepad-hotkeys.dll`
- Data: `<OBS root>\data\obs-plugins\obs-gamepad-hotkeys\...`

When standard OBS cannot be found, the installer offers Standard or Portable mode. Portable mode validates that `<OBS root>\bin\64bit\obs64.exe` exists before installation. Portable installs intentionally do not register a machine-wide Add/Remove Programs entry; the files stay self-contained with the portable OBS tree.

For automated portable testing, setup also accepts:

`OBS-Gamepad-Hotkeys-Setup-v<version>.exe /OBSROOT="D:\Portable\obs-studio"`

The path must be a portable OBS root containing `portable_mode.txt` or `portable_mode`.

## Running OBS

The installer checks whether `obs64.exe` is running immediately before files are copied. It does not force-kill OBS. If OBS is open, the message is intentionally actionable: close OBS and retry Install.

## Local one-click build

Double-click:

- `BUILD-INSTALLER.cmd` — build the plugin + installer and select the resulting EXE in Explorer.
- `BUILD-INSTALLER-AND-RUN.cmd` — build the plugin + installer and immediately launch the installer, which is the closest local test to the end-user experience.

The build script automatically downloads a pinned Inno Setup 6.7.3 compiler from the official JRSoftware GitHub release if `ISCC.exe` is not already installed. The download is SHA-256 verified before use.

Visual Studio 2022 C++ tools and CMake are still required to compile the OBS plugin itself.

## GitHub Actions release

`.github/workflows/build-windows.yml` produces:

- `OBS-Gamepad-Hotkeys-Setup-v<version>.exe`
- `obs-gamepad-hotkeys-<version>-windows-x64.zip`
- SHA-256 files for both

Pushing a tag such as `v0.1.5` also creates the GitHub Release and attaches those files.

## Production code signing

An unsigned EXE can still trigger Windows SmartScreen / Unknown Publisher even if the installer UI itself is clean. Production releases should be Authenticode-signed with a trusted code-signing certificate.

The GitHub workflow supports these repository secrets:

- `WINDOWS_CERTIFICATE_BASE64` — Base64 encoded `.pfx`
- `WINDOWS_CERTIFICATE_PASSWORD` — PFX password

On branch/tag pushes, when the certificate is present, the workflow signs the plugin DLL first, packages it, builds the installer, signs the installer, verifies signatures with `signtool`, and regenerates SHA-256 files. Pull-request builds never receive or use the signing certificate.
