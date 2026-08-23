# Release Test Plan

A release should not be tagged until the P0 matrix passes on a real Windows machine. CI additionally enforces source integrity, build success, installer compilation, and Portable ZIP layout.

## P0 installation and module-loading tests

| Test | Expected |
|---|---|
| Standard OBS only | Setup offers Standard + Portable/custom target; Standard is preselected when detected |
| Standard OBS + Portable OBS on same PC | Setup still exposes Portable/custom target; Standard detection does not short-circuit the wizard |
| Select Portable root | Setup validates `<root>\bin\64bit\obs64.exe` |
| Install to Portable root | DLL lands at `<root>\obs-plugins\64bit\obs-gamepad-hotkeys.dll` |
| Install to Portable root | Data lands at `<root>\data\obs-plugins\obs-gamepad-hotkeys\...` |
| Finish Portable install | OBS launches with `--portable`; `Tools -> Gamepad Hotkeys` exists |
| Manual Portable ZIP | Extract directly to OBS root; `Tools -> Gamepad Hotkeys` exists after restart |
| Wrong legacy path check | No plugin DLL is shipped or documented under `<root>\bin\64bit` |
| OBS starts with plugin installed | No crash; OBS log contains `[Gamepad Hotkeys] plugin loaded` |
| Open plugin with no controller | UI opens; connected count is 0 |

## CI release-layout tests

The Windows build job must expand the final Portable ZIP and verify:

- `obs-plugins\64bit\obs-gamepad-hotkeys.dll` exists.
- `data\obs-plugins\obs-gamepad-hotkeys\locale\en-US.ini` exists.
- `INSTALL-PORTABLE.txt` exists.
- `bin\64bit\obs-gamepad-hotkeys.dll` does not exist.
- `obs-gamepad-hotkeys\bin\64bit\obs-gamepad-hotkeys.dll` does not exist.

These checks exist specifically to prevent a recurrence of the v0.1.6 package layout that could be extracted into the wrong OBS folder structure.

## P0 controller smoke tests

| Test | Expected |
|---|---|
| Connect Xbox-compatible controller before OBS | XInput device appears |
| Connect generic DirectInput controller before OBS | DirectInput device appears |
| Add mapping with Listen | Pressed button is captured once |
| Map A -> Screenshot | One screenshot per press; release causes no second screenshot |
| Put game/app fullscreen foreground | Mapping still triggers OBS |
| Exit OBS | Worker stops cleanly; no hang/crash |

## Stateful hotkey tests

| Test | Expected |
|---|---|
| Gamepad button -> Push-to-talk | Active only while button is held |
| Disconnect controller while PTT held | Forced release occurs; PTT does not remain latched |
| Enter Listen while hold action active | Action is released before capture begins |
| Remove a mapping while hold action active | Action is released |
| Two controls map to same hold action | Releasing one does not release action until both are up |

## OBS registry tests

- Scene switch hotkeys appear in action list.
- Source show/hide or source-specific hotkeys appear and are distinguishable by owner display.
- Rename a source; mapping should remain pointed to the same UUID-backed source after registry refresh.
- Change OBS language and restart; internal-name mapping should still resolve.
- Change scene collection and return; no stale runtime hotkey IDs should be invoked.
- Install another plugin that registers hotkeys; its hotkeys should appear after refresh.

## Controller compatibility matrix

At minimum test:

- Xbox Series / Xbox One controller via USB.
- Xbox-compatible controller via Bluetooth.
- Generic DirectInput USB gamepad.
- One PlayStation-family controller as exposed by Windows/driver stack.
- Two controllers connected simultaneously.

## OBS host matrix

For a public Windows release, test at least:

- Project build baseline (currently OBS 31.1.1).
- Current stable OBS release.
- Standard installed OBS.
- Official Windows ZIP running with `portable_mode.txt`.
- Official Windows ZIP launched with `--portable` and no marker file.

The project intentionally keeps the build baseline conservative unless a newer OBS API is required. A packaging-only release should not raise the minimum host version without a functional reason.

## Focus/interference tests

- OBS foreground.
- Browser foreground.
- Borderless-window game foreground.
- Exclusive/fullscreen game if supported by the game.
- Game that uses the same controller button; both game and OBS are expected to receive non-exclusive input.
- Run OBS normally and elevated; document behavior when target game runs at a different integrity level. Because this plugin reads the controller directly instead of injecting keyboard input, foreground keyboard focus should not be the mechanism of operation.

## Performance

Measure OBS idle CPU for 60 seconds:

1. Plugin absent.
2. Plugin installed, no controller.
3. One XInput controller idle.
4. One DirectInput controller idle.
5. Both backends active with frequent button presses.

Target: no measurable frame/render impact and only negligible CPU delta attributable to the 125 Hz lightweight polling thread.

## Failure injection

- Unplug/replug controller repeatedly.
- Close configuration dialog while Listen is active.
- Switch scene collections rapidly.
- Close OBS while holding mapped button.
- Delete a source whose hotkey is mapped.
- Corrupt `config.json`; plugin should load with empty mappings rather than crash.
- Install Standard target, then rerun Setup and select a different Portable root; files must land only in the selected target for that run.
- Put an old/wrong copy of the DLL under `<OBS root>\bin\64bit`; confirm the correct plugin still loads only from `obs-plugins\64bit` and document removal of the stray DLL.
