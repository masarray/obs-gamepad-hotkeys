# Product Requirements — OBS Gamepad Hotkeys

## Problem

OBS users who want a gamepad as a control surface commonly insert a keyboard-emulation utility between controller and OBS. That creates another process to launch/manage and routes control through synthetic keyboard events whose behavior can vary with focus, privileges, or application-specific input handling.

## Product goal

Make a connected gamepad act as a native OBS control surface while OBS is in the background, without keyboard emulation and without requiring a separate runtime application.

## Primary user stories

1. As a streamer, I can bind a controller button directly to any hotkey registered in OBS.
2. As a gamer, the binding still works while the game is foreground because OBS reads the controller directly.
3. As a PTT user, press/release state is accurate and cannot remain stuck after controller disconnect.
4. As a plugin user, hotkeys registered by third-party OBS plugins appear automatically without this project hard-coding them.
5. As a user with a generic controller, I am not limited to Xbox/XInput hardware.

## Functional requirements

### P0

- Enumerate XInput controllers.
- Enumerate DirectInput game controllers.
- Background/non-exclusive DirectInput cooperative level.
- Detect button down/up edges.
- Map controller input to stable OBS hotkey identity.
- Resolve stable identity to current runtime `obs_hotkey_id`.
- Dispatch callbacks on OBS UI task queue.
- Persistent mappings.
- Listen-to-bind UI.
- Hot-plug handling and forced releases.
- Automatic OBS hotkey registry refresh on relevant frontend lifecycle events.

### P1

- Chords.
- Long/double press.
- Axis thresholds.
- Strong duplicate-device detection.
- Import/export mapping profiles.

## Non-functional requirements

- No keyboard/mouse injection.
- No administrator requirement for ordinary use.
- No exclusive controller capture.
- Negligible CPU impact at idle.
- No work on OBS render/audio real-time threads.
- Clean shutdown without leaked worker thread.
- Open source under GPL-2.0-or-later.

## Success criteria for v0.1

- A real controller button changes an OBS scene or triggers screenshot while a different app/game has focus.
- Hold actions correctly follow press/release.
- Disconnect while held does not latch the action.
- Generic DirectInput device works without JoyToKey.
- At least one third-party OBS hotkey can be selected without code changes.
- GitHub Actions produces an installable Windows x64 ZIP.

## Out of scope for v0.1

- Remapping controller input for the game itself.
- Suppressing the button from the game.
- Virtual controller creation.
- Macro scripting.
- Haptics.
- Linux/macOS.
