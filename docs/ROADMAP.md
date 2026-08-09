# Roadmap

## P0 — v0.1 baseline

- [x] XInput backend.
- [x] DirectInput background/non-exclusive backend.
- [x] Press/release edge events.
- [x] Direct OBS hotkey registry routing.
- [x] Runtime ID re-resolution.
- [x] Source UUID identity.
- [x] Hold-action release safety.
- [x] Compact mapping UI + Listen.
- [x] JSON persistence.
- [x] Windows build/install/package scripts.
- [x] GitHub Actions build workflow.
- [ ] Compile on Windows CI and resolve any compiler/API integration errors.
- [ ] Execute P0 release test matrix.

## P1 — streamer ergonomics and compatibility

- Two-button chord capture and mapping (`LB+A`, `BACK+DPAD_UP`).
- Long-press and double-press gestures.
- DirectInput analog axis/trigger threshold mapping with hysteresis.
- Robust XInput-vs-DirectInput duplicate detection using device path/VID/PID/IG_ technique rather than product-name heuristic.
- Mapping conflict warning.
- Per-mapping debounce/cooldown for noisy generic hardware.
- Import/export profiles.
- Per OBS profile/scene-collection mapping scope.
- Diagnostics panel with backend, last event, resolved/unresolved action state, event rate.

## P2 — platform expansion

- Evaluate Windows GameInput backend.
- SDL3 backend for Linux/macOS controller support.
- Optional dock mode.
- Device GUID migration rules for reconnect/reorder cases.
- Localization.

## P3 — community release

- Signed Windows installer or OBS-compatible package format if desired.
- Automated release workflow on semantic-version tags.
- Compatibility CI against selected OBS major versions.
- Public contribution guide and issue templates.
