param(
    [switch]$Build
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Temp = Join-Path $env:TEMP ("obs-plugin-template-" + [guid]::NewGuid().ToString('N'))
$Zip = "$Temp.zip"
$Url = 'https://github.com/obsproject/obs-plugintemplate/archive/refs/heads/master.zip'

Write-Host 'Downloading the official OBS plugin template support files...'
Invoke-WebRequest -Uri $Url -OutFile $Zip
Expand-Archive -Path $Zip -DestinationPath $Temp -Force
$Template = Get-ChildItem -Path $Temp -Directory | Select-Object -First 1
if (-not $Template) { throw 'Could not locate extracted OBS plugin template.' }

foreach ($Folder in @('cmake', 'build-aux')) {
    $Source = Join-Path $Template.FullName $Folder
    if (Test-Path $Source) {
        Copy-Item $Source -Destination $Root -Recurse -Force
    }
}

foreach ($File in @('.clang-format', '.gersemirc')) {
    $Source = Join-Path $Template.FullName $File
    if (Test-Path $Source) {
        Copy-Item $Source -Destination (Join-Path $Root $File) -Force
    }
}

Remove-Item $Temp -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item $Zip -Force -ErrorAction SilentlyContinue

Write-Host 'OBS template support files installed.'
Write-Host 'Configure: cmake --preset windows-x64'
Write-Host 'Build:     cmake --build --preset windows-x64'

if ($Build) {
    Push-Location $Root
    try {
        cmake --preset windows-x64
        if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
        cmake --build --preset windows-x64
        if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    }
    finally {
        Pop-Location
    }
}
