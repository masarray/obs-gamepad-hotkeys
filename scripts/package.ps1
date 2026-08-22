param(
    [string]$Configuration = 'RelWithDebInfo'
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$PluginName = 'obs-gamepad-hotkeys'
$BuildSpec = Get-Content (Join-Path $Root 'buildspec.json') -Raw | ConvertFrom-Json
$Version = $BuildSpec.version
$BuildRoot = Join-Path $Root 'build_x64'
$Dist = Join-Path $Root 'dist'
$Stage = Join-Path $Dist "$PluginName-$Version-windows-x64-portable"
$PluginBinStage = Join-Path $Stage 'obs-plugins\64bit'
$PluginDataStage = Join-Path $Stage "data\obs-plugins\$PluginName"
$Zip = Join-Path $Dist "$PluginName-$Version-windows-x64-portable.zip"

$dll = Get-ChildItem -Path $BuildRoot -Filter "$PluginName.dll" -Recurse -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if (-not $dll) { throw "Could not find $PluginName.dll. Build first." }

Remove-Item $Stage -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $PluginBinStage -Force | Out-Null
New-Item -ItemType Directory -Path $PluginDataStage -Force | Out-Null

Copy-Item $dll.FullName (Join-Path $PluginBinStage "$PluginName.dll") -Force
Copy-Item (Join-Path $Root 'data\*') $PluginDataStage -Recurse -Force

$installText = @"
OBS Gamepad Hotkeys $Version - Portable / Manual Install

1. Close OBS Studio.
2. Extract the CONTENTS of this ZIP directly into the OBS Studio root folder.
   The correct OBS root contains: bin, data, and obs-plugins.
3. After extraction, verify these paths exist:
   obs-plugins\64bit\obs-gamepad-hotkeys.dll
   data\obs-plugins\obs-gamepad-hotkeys\locale\en-US.ini
4. Start OBS. For portable mode, use portable_mode.txt / portable_mode or launch obs64.exe with --portable.
5. Open Tools > Gamepad Hotkeys.

Do NOT copy this ZIP's obs-plugins or data contents into OBS\bin\64bit.
Project: https://github.com/masarray/obs-gamepad-hotkeys
"@
Set-Content -Path (Join-Path $Stage 'INSTALL-PORTABLE.txt') -Value $installText -Encoding utf8

# Regression guards: this archive must be directly extractable into an OBS root.
$requiredStageFiles = @(
    (Join-Path $Stage "obs-plugins\64bit\$PluginName.dll"),
    (Join-Path $Stage "data\obs-plugins\$PluginName\locale\en-US.ini"),
    (Join-Path $Stage 'INSTALL-PORTABLE.txt')
)
foreach ($required in $requiredStageFiles) {
    if (-not (Test-Path $required -PathType Leaf)) {
        throw "Portable package staging is invalid; missing: $required"
    }
}
if (Test-Path (Join-Path $Stage "bin\64bit\$PluginName.dll")) {
    throw 'Portable package regression: plugin DLL must never be staged under OBS bin\64bit.'
}

Remove-Item $Zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $Stage '*') -DestinationPath $Zip -CompressionLevel Optimal
Write-Host "Created portable OBS-root package: $Zip"
