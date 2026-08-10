# OBS Gamepad Hotkeys

Native Windows gamepad input for OBS Studio. The plugin reads controllers directly and routes button press/release events to OBS's registered hotkey callbacks — no JoyToKey, keyboard emulation, or foreground-window dependency.

> **Status:** v0.1.5 public preview. Windows x64. XInput + DirectInput. Windows builds, the manual ZIP, and the Smart Installer are validated by GitHub Actions; physical-controller behavior should still be exercised on the controllers/OBS versions targeted by each public release.

## Why this exists

Typical setup today:

`Gamepad -> JoyToKey -> synthetic keyboard shortcut -> OBS hotkey`

This plugin replaces it with:

`Gamepad -> native input backend -> mapping engine -> OBS UI task queue -> native OBS hotkey callback`

That removes the extra background application and avoids relying on a synthetic keyboard shortcut reaching OBS while a game or another application has focus.

The plugin intentionally uses **non-exclusive** controller access. It does not consume or suppress the button from the foreground game; if a game also uses the same button, both the game and OBS may react.

## v0.1 features

- XInput controller support (Xbox-compatible controllers, up to 4 slots).
- XInput DLL fallback: `xinput1_4.dll` -> `xinput1_3.dll` -> `xinput9_1_0.dll`.
- DirectInput support for generic USB gamepads/joysticks.
- DirectInput cooperative mode: background + non-exclusive.
- Press **and** release forwarding for hold-style OBS actions such as push-to-talk.
- Forced release when a DirectInput device disappears.
- Reference-counted hotkey state so multiple mapped controls do not prematurely release the same hold action.
- Automatic DirectInput hot-plug scan every 2 seconds.
- Native OBS hotkey enumeration; actions are not hard-coded.
- Source hotkeys use source UUID in persistent identity, so source renames do not retarget the mapping.
- Localized OBS description text is display-only and is not used as persistent identity.
- Compact **Tools -> Gamepad Hotkeys** configuration UI.
- **Listen** capture mode for learning a controller button.
- `Any Controller` mappings or mapping to a specific detected device.
- Persistent JSON configuration under the OBS module config path.
- Mapping capture temporarily suspends dispatch and releases active hold actions to prevent accidental scene/recording changes.
- Smart recording actions: `B` defaults to Pause/Resume Recording and `START` defaults to Start/Stop Recording.
- Theme-aware Lucide UI icons plus high-DPI controller badges for A/B/X/Y, shoulders, sticks, D-pad, START/BACK and DirectInput buttons.
- Compact Lucide `trash-2` mapping removal action, vertically and horizontally centered in its table cell.

## Input names

### XInput

`A`, `B`, `X`, `Y`, `LB`, `RB`, `BACK`, `START`, `LS`, `RS`, `DPAD_UP`, `DPAD_DOWN`, `DPAD_LEFT`, `DPAD_RIGHT`, `LT`, `RT`.

LT/RT become digital buttons at the standard XInput trigger threshold.

### DirectInput

`BUTTON_1` through `BUTTON_128`, plus the first POV hat as `DPAD_UP`, `DPAD_DOWN`, `DPAD_LEFT`, `DPAD_RIGHT`.

## Easiest install for normal users

GitHub Actions builds a branded smart Windows installer:

`OBS-Gamepad-Hotkeys-Setup-v<version>.exe`

For a normal OBS Studio installation, the installer detects OBS automatically and the user does not choose plugin folders. It installs to the Windows OBS third-party plugin location under `C:\ProgramData\obs-studio\plugins`. If standard OBS is not found, it offers a dedicated OBS Portable path and validates `bin\64bit\obs64.exe` before copying files.

The v0.1.5 installer uses a modern light/dark-adaptive Windows wizard, the repository's authored `gamepad.jpg` artwork, a clear OBS detection summary, default-control preview, and an actionable OBS-running check. The artwork is embedded image-only: the build pipeline does not draw duplicate text or branding over the image. It offers **Launch OBS Studio and open Tools > Gamepad Hotkeys** on Finish. See [docs/INSTALLER.md](docs/INSTALLER.md).

> Windows SmartScreen / Unknown Publisher is separate from installer UX. Production releases need Authenticode code signing to remove that publisher warning. The GitHub workflow supports optional certificate secrets and signs both the plugin DLL and installer when configured.

## One-click local installer test

After extracting or cloning the repository, double-click:

```text
BUILD-INSTALLER-AND-RUN.cmd
```

It performs the complete local path:

`bootstrap -> CMake configure -> plugin build -> branded installer build -> launch installer`

So the installer you test locally is the same installer format an end user downloads from GitHub Releases. Inno Setup is bootstrapped automatically as a local build tool and its download is SHA-256 verified.

If you only want to build the installer and inspect the EXE, double-click:

```text
BUILD-INSTALLER.cmd
```

The output is placed in `dist\`.

## Fastest Windows build

Requirements:

- Windows 10/11 x64
- Visual Studio 2022 with Desktop development with C++
- Windows 10/11 SDK (10.0.20348 or newer; 10.0.22621 is used by the preset)
- CMake 3.28+
- Internet access on the first configure so the official OBS plugin-template dependency bootstrap can download OBS/Qt dependencies

For the most realistic local test, use the one-click wrapper instead of manually copying plugin files:

```text
BUILD-INSTALLER-AND-RUN.cmd
```

The first run downloads the official OBS plugin-template build support plus a pinned, hash-verified Inno Setup compiler, configures the project, builds x64, creates the installer, and launches that installer.

After installation, open OBS and use:

**Tools -> Gamepad Hotkeys**

## Manual build

```powershell
.\scripts\bootstrap-template.ps1
cmake --preset windows-x64
cmake --build --preset windows-x64
```

Developer direct-install for standard OBS (run PowerShell as Administrator):

```powershell
.\scripts\install-local.ps1
```

For OBS Portable:

```powershell
.\scripts\install-local.ps1 -PortableRoot "D:\OBS-Studio"
```

Create a distributable ZIP after building:

```powershell
.\scripts\package.ps1
```

## GitHub Actions

`.github/workflows/build-windows.yml` validates and builds the plugin, creates both the manual ZIP and Smart Installer EXE, generates SHA-256 files, and uploads them as Windows artifacts. A `v*` tag also publishes the files to a GitHub Release.

The build baseline currently follows the dependency versions/hashes in the official OBS plugin template (`OBS Studio 31.1.1` build dependency). Runtime validation against the OBS versions you intend to publish for should be part of the release test matrix.

## How to use

1. Connect the controller.
2. Start OBS.
3. Open **Tools -> Gamepad Hotkeys**.
4. Fresh installs already have `B -> Pause/Resume Recording` and `START -> Start/Stop Recording`.
5. To add another mapping, click **Add Mapping**.
6. Select `Any Controller` or a specific controller.
7. Click **Listen** and press one gamepad button.
8. Search/select an OBS action and save.
9. Put another application or game in the foreground and test the mapping.

## Architecture

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

The crucial OBS integration is intentionally small: the worker thread only reads controller state and detects edges. OBS execution is marshalled to the OBS UI task queue, then calls the registered OBS hotkey callback by runtime `obs_hotkey_id`.

Persistent config never stores that runtime ID. It stores a stable identity derived from hotkey registerer type, owning object identity, and OBS's internal hotkey name; runtime IDs are resolved again whenever the OBS hotkey registry is refreshed.

## Known v0.1 limitations

- Windows only.
- One-button mappings only. Two-button chords, long-press, and double-press are planned for P1.
- DirectInput currently maps buttons + first POV hat, not arbitrary analog axes.
- Duplicate filtering between XInput and DirectInput is still heuristic; stronger VID/PID/IG_ detection is planned for P1.
- No haptic feedback.
- No per-OBS-profile mapping layer yet; configuration is plugin-global.
- CI validates Windows compilation and installer packaging; release QA should still include physical gamepad tests and the intended OBS runtime versions.

## Reliability rules in the implementation

- No `SendInput`.
- No virtual keyboard shortcuts.
- No busy-wait loop; polling sleeps 8 ms (~125 Hz maximum poll cadence).
- UI never reads controller APIs directly.
- Controller worker never directly mutates Qt UI.
- OBS callbacks are queued to `OBS_TASK_UI`.
- Hold-state releases are forced on mapping edits/listen mode/device removal.
- Hotkey registry changes release active hold state before runtime IDs are replaced.
