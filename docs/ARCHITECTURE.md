# Architecture

## Data flow

```text
XInputBackend -----------\
                         +--> GamepadManager --> InputEvent edge --> ObsHotkeyRouter
DirectInputBackend ------/                                  |             |
                                                            |             v
                                                            |      obs_queue_task
                                                            |        OBS_TASK_UI
                                                            |             |
                                                            |             v
                                                            |  obs_hotkey_trigger_routed_callback
                                                            |             |
                                                            +---- UI Listen/diagnostics
                                                                          |
                                                                          v
                                                                 Native OBS hotkey callback
```

## Threading

### Worker thread

`GamepadManager` owns one polling thread.

- Poll cadence: 8 ms sleep between loops.
- XInput: edge-only state changes based on `dwPacketNumber`.
- DirectInput: `Poll()` / `GetDeviceState()` and edge comparison.
- Detected events are forwarded to `ObsHotkeyRouter::onInputEvent()`.

The worker does **not** touch Qt widgets and does **not** invoke OBS frontend actions directly.

### OBS UI thread

The router allocates a tiny dispatch payload and calls:

```cpp
obs_queue_task(OBS_TASK_UI, ...)
```

The queued task then invokes:

```cpp
obs_hotkey_trigger_routed_callback(id, pressed);
```

OBS Studio itself enables hotkey callback rerouting in its frontend initialization. Calling the routed callback after marshaling to the UI thread therefore reaches the same registered callback objects used by OBS's normal hotkey system without synthesizing a keyboard key.

## Hotkey identity

`obs_hotkey_id` is runtime state and must not be serialized.

Persistent key:

```text
registerer_type | owner_identity | internal_hotkey_name
```

Examples:

```text
frontend|OBSBasic.StartRecording
source:<source UUID>|<source hotkey name>
```

Localized descriptions are stored only as friendly display text. This prevents mappings from breaking merely because OBS UI language changes.

## Source ownership

OBS exposes hotkey registerer type and registerer pointer. Source hotkeys are registered with weak source references. During enumeration the plugin obtains a temporary strong reference, reads source UUID/name, then releases it.

Using UUID rather than source display name keeps mappings stable across source renames.

## Press/release aggregation

The router tracks a reference count per runtime OBS hotkey ID.

- First mapped input down: dispatch `pressed=true`.
- Additional mapped inputs down for the same action: increment count, do not duplicate press.
- Release while another mapped input remains held: decrement only.
- Final release: dispatch `pressed=false`.

This matters for push-to-talk/push-to-mute and other hold-style callbacks.

## Background controller input

### XInput

`XInputGetState` is polled directly and does not depend on the foreground application receiving a keyboard event.

### DirectInput

Each generic game controller is configured with:

```cpp
DISCL_BACKGROUND | DISCL_NONEXCLUSIVE
```

Background allows OBS to continue reading it while another window is foreground. Non-exclusive prevents the plugin from intentionally taking ownership away from the game.

## Hot-plug behavior

- XInput slots are checked continuously.
- DirectInput attached devices are enumerated every 2 seconds.
- Removed DirectInput devices emit forced release events for controls that were logically held.
- XInput disconnect emits release for every logically held control.

## Configuration

Qt JSON schema v1:

```json
{
  "schema": 1,
  "mappings": [
    {
      "device": "*",
      "control": "A",
      "hotkey_key": "0|frontend|OBSBasic.Screenshot",
      "hotkey_display": "Screenshot  [OBSBasic.Screenshot]"
    }
  ]
}
```

The exact numeric registerer type is generated from OBS at runtime; examples are illustrative.
