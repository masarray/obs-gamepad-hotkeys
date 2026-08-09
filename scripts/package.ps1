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
$Stage = Join-Path $Dist "$PluginName-$Version-windows-x64"
$PluginStage = Join-Path $Stage $PluginName
$Zip = Join-Path $Dist "$PluginName-$Version-windows-x64.zip"

$dll = Get-ChildItem -Path $BuildRoot -Filter "$PluginName.dll" -Recurse -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if (-not $dll) { throw "Could not find $PluginName.dll. Build first." }

Remove-Item $Stage -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path (Join-Path $PluginStage 'bin\64bit') -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $PluginStage 'data') -Force | Out-Null
Copy-Item $dll.FullName (Join-Path $PluginStage 'bin\64bit') -Force
Copy-Item (Join-Path $Root 'data\*') (Join-Path $PluginStage 'data') -Recurse -Force

Remove-Item $Zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $Stage '*') -DestinationPath $Zip -CompressionLevel Optimal
Write-Host "Created: $Zip"
