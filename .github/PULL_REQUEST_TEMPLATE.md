## What does this change?

<!-- Describe the user problem first, then the implementation. Keep the PR focused. -->

## User-facing behavior

<!-- What will a normal OBS user notice? Write "No user-facing change" when appropriate. -->

## Safety / compatibility

- [ ] No keyboard emulation or input injection was introduced.
- [ ] Existing user mappings/configuration are preserved or migration is documented.
- [ ] OBS UI/frontend work is kept on the appropriate OBS/UI thread.
- [ ] Standard OBS and OBS Portable installer behavior is unchanged, or installer impact is described below.
- [ ] No secrets, certificates, build output, or local machine state are committed.

## Validation

- [ ] `python tools/static-check.py`
- [ ] Windows CMake configure/build passes, or the reason it could not be run is stated below.
- [ ] Relevant runtime/controller behavior was tested, or untested hardware/runtime scope is stated below.
- [ ] Documentation/landing page was updated when the user workflow changed.

### Validation notes

<!-- OBS version, Windows version, controller used, CI run, and anything not physically validated. -->

## Release impact

- [ ] No release required.
- [ ] Suitable for the next normal release.
- [ ] Requires explicit release handling or migration notes.

<!-- Select the applicable item(s) and explain anything unusual. -->
