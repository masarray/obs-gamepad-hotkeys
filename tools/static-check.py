#!/usr/bin/env python3
from pathlib import Path
import json, re, sys
root = Path(__file__).resolve().parents[1]
required = [
    'CMakeLists.txt','CMakePresets.json','buildspec.json',
    'src/plugin-main.cpp','src/xinput-backend.cpp','src/directinput-backend.cpp',
    'src/gamepad-manager.cpp','src/obs-hotkey-router.cpp','src/config.cpp','src/gamepad-dialog.cpp',
    'src/gamepad-icons.hpp','src/gamepad-icons.cpp','data/licenses/LUCIDE-ISC.txt',
    '.github/workflows/build-windows.yml',
    'installer/obs-gamepad-hotkeys.iss','scripts/build-installer.ps1','scripts/ensure-inno.ps1',
    'scripts/new-installer-branding.ps1','gamepad.jpg',
    'BUILD-INSTALLER.cmd','BUILD-INSTALLER-AND-RUN.cmd'
]
for rel in required:
    if not (root / rel).exists():
        raise SystemExit(f'Missing required file: {rel}')
for rel in ('buildspec.json','CMakePresets.json'):
    json.loads((root / rel).read_text(encoding='utf-8'))
for p in sorted((root/'src').glob('*.[ch]pp')):
    s = p.read_text(encoding='utf-8')
    t = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    t = re.sub(r'//.*', '', t)
    # Strip C++ raw string literals before checking delimiters. This matters
    # for embedded SVG/XML where parentheses are data, not C++ syntax.
    t = re.sub(r'R"([A-Za-z0-9_]*)\(.*?\)\1"', '""', t, flags=re.S)
    t = re.sub(r'"(?:\\.|[^"\\])*"', '""', t)
    stack=[]; pairs={'{':'}','(':')','[':']'}
    for ch in t:
        if ch in pairs: stack.append(ch)
        elif ch in pairs.values():
            if not stack or pairs[stack.pop()] != ch:
                raise SystemExit(f'Unbalanced delimiters in {p}')
    if stack: raise SystemExit(f'Unbalanced delimiters in {p}')
all_src='\n'.join(p.read_text(encoding='utf-8') for p in (root/'src').glob('*'))
for forbidden in ('SendInput','keybd_event','RegisterHotKey'):
    if forbidden in all_src:
        raise SystemExit(f'Forbidden keyboard-injection API found: {forbidden}')
for required_api in ('obs_hotkey_trigger_routed_callback','OBS_TASK_UI','DISCL_BACKGROUND','DISCL_NONEXCLUSIVE'):
    if required_api not in all_src:
        raise SystemExit(f'Required native routing primitive missing: {required_api}')
for visual_primitive in ('lucideGamepadIcon','gamepadControlIcon','gamepad-2'):
    if visual_primitive not in all_src:
        raise SystemExit(f'Required visual primitive missing: {visual_primitive}')

installer = (root / 'installer/obs-gamepad-hotkeys.iss').read_text(encoding='utf-8')
for required_text in ('{commonappdata}\\obs-studio\\plugins', 'obs-plugins\\64bit', 'bin\\64bit\\obs64.exe', 'PrepareToInstall'):
    if required_text not in installer:
        raise SystemExit(f'Installer routing primitive missing: {required_text}')

branding = (root / 'scripts/new-installer-branding.ps1').read_text(encoding='utf-8')
for required_text in ('gamepad.jpg', 'Draw-ImageContain', 'Draw-ImageCover', 'HighQualityBicubic'):
    if required_text not in branding:
        raise SystemExit(f'Installer branding primitive missing: {required_text}')
if 'DrawString' in branding:
    raise SystemExit('Installer branding must not draw duplicate text over gamepad.jpg')

install_local = (root / 'scripts/install-local.ps1').read_text(encoding='utf-8')
if '$env:APPDATA' in install_local:
    raise SystemExit('Legacy APPDATA plugin install path is still present in install-local.ps1')

print('Static checks passed.')
