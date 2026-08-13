# ArZoom + OBS Gamepad Hotkeys

OBS Gamepad Hotkeys can control **ArZoom for OBS** directly from a controller.

You do **not** need JoyToKey, keyboard emulation, or an ArZoom keyboard shortcut for this integration.

## What the gamepad button does

The mapped button toggles ArZoom's native OBS action:

```text
ArZoom: Toggle Zoom In / Out
```

Press once:

```text
Zoom in + mouse follow
```

Press the same gamepad button again:

```text
Smooth zoom out to the normal view
```

ArZoom still controls the zoom amount, follow mode, movement style, safe zone, and target monitor.

## Setup

1. Install **ArZoom for OBS** and **OBS Gamepad Hotkeys**.
2. In OBS, add **ArZoom - Smart Mouse Zoom** to the Display Capture source you want to zoom.
3. Open **Tools → Gamepad Hotkeys**.
4. Click **Add Mapping**.
5. Click **Listen** and press the controller button you want to use.
6. Under **OBS action**, choose **ArZoom: Toggle Zoom In / Out**.
7. Click **Save**.
8. Put your game or application back in the foreground and press the mapped button.

The ArZoom action appears automatically when OBS Gamepad Hotkeys detects ArZoom's registered `arzoom.toggle` frontend hotkey.

## No keyboard shortcut required

ArZoom normally exposes **ArZoom — Toggle Zoom & Mouse Follow** in OBS Settings → Hotkeys. OBS Gamepad Hotkeys discovers that same native OBS hotkey and triggers its callback directly.

The control path is:

```text
Gamepad
  ↓
OBS Gamepad Hotkeys
  ↓
ArZoom native OBS hotkey callback
  ↓
Zoom in / zoom out
```

There is no synthetic keyboard event in this path.

## If the ArZoom action does not appear

- Confirm ArZoom is installed and visible in OBS.
- Restart OBS after installing either plugin.
- Open **Tools → Gamepad Hotkeys** and click **Add Mapping** again; the OBS action registry is refreshed automatically.
- If needed, click the small **Refresh OBS Actions** button in the Gamepad Hotkeys window.
- Confirm ArZoom's own hotkey row exists in **OBS Settings → Hotkeys**.

## Important

Controller access remains non-exclusive. If the game also uses the same controller button, both the game and ArZoom can react. Pick a button that fits your game/control layout.
