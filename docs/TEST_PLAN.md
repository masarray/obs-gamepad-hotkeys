# Release Test Plan

A release should not be tagged until this matrix passes on a real Windows machine.

## P0 smoke tests

| Test | Expected |
|---|---|
| OBS starts with plugin installed | No crash; `Tools -> Gamepad Hotkeys` exists |
| Open plugin with no controller | UI opens; connected count is 0 |
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
