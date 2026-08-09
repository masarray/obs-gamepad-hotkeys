param(
    [string]$Configuration = 'RelWithDebInfo',
    [string]$PortableRoot,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$BuildRoot = Join-Path $Root 'build_x64'
$PluginName = 'obs-gamepad-hotkeys'

$obs = Get-Process obs64 -ErrorAction SilentlyContinue
if ($obs -and -not $Force) {
    throw 'OBS Studio is open. Close OBS Studio before installing the plugin.'
}

$dll = Get-ChildItem -Path $BuildRoot -Filter "$PluginName.dll" -Recurse -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if (-not $dll) {
    throw "Could not find $PluginName.dll under $BuildRoot. Build the plugin first."
}

if ($PortableRoot) {
    $PortableRoot = (Resolve-Path $PortableRoot).Path
    $obsExe = Join-Path $PortableRoot 'bin\64bit\obs64.exe'
    if (-not (Test-Path $obsExe)) {
        throw "PortableRoot is not an OBS root folder: $PortableRoot"
    }
    $TargetBin = Join-Path $PortableRoot 'obs-plugins\64bit'
    $TargetData = Join-Path $PortableRoot "data\obs-plugins\$PluginName"
}
else {
    $TargetRoot = Join-Path $env:ProgramData "obs-studio\plugins\$PluginName"
    $TargetBin = Join-Path $TargetRoot 'bin\64bit'
    $TargetData = Join-Path $TargetRoot 'data'

    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Standard OBS plugin installation uses C:\ProgramData and requires Administrator rights. Use BUILD-INSTALLER-AND-RUN.cmd for the normal end-user install flow.'
    }
}

New-Item -ItemType Directory -Path $TargetBin -Force | Out-Null
New-Item -ItemType Directory -Path $TargetData -Force | Out-Null
Copy-Item $dll.FullName (Join-Path $TargetBin $dll.Name) -Force

$DataSource = Join-Path $Root 'data'
if (Test-Path $DataSource) {
    Copy-Item (Join-Path $DataSource '*') $TargetData -Recurse -Force
}

Write-Host "Installed DLL to: $TargetBin"
Write-Host 'Start OBS Studio, then open Tools -> Gamepad Hotkeys.'
