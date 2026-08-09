# Licensing and distribution notes

This is an engineering note, not legal advice.

## Project license

The repository uses GPL-2.0 and includes the license text in `LICENSE`.

The plugin links against OBS/libobs and uses the OBS frontend API. Keeping the plugin source public under GPL-2.0 is a straightforward distribution posture for an OBS plugin derived from and linked with the GPL-licensed OBS ecosystem.

## Windows controller APIs

The implementation calls Windows system APIs:

- XInput loaded dynamically from Windows DLLs.
- DirectInput 8 via Windows SDK headers/import libraries.

The project does not redistribute Microsoft's controller DLLs or any controller driver.

## Third-party utilities

The plugin does not bundle, launch, or depend on JoyToKey.

## Repository hygiene

Before public release:

- Keep the full corresponding source for every released binary/tag.
- Preserve OBS/open-source notices where code is copied rather than independently implemented.
- Do not copy source from a deleted third-party gamepad plugin unless its license and provenance are known and compatible.
- Maintain a `NOTICE`/attribution file if future dependencies require one.
