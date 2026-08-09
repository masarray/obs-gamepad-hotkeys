param(
    [switch]$SkipBootstrap,
    [switch]$NoInstall
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
Push-Location $Root
try {
    if (-not $SkipBootstrap -and -not (Test-Path (Join-Path $Root 'cmake\common\bootstrap.cmake'))) {
        & (Join-Path $PSScriptRoot 'bootstrap-template.ps1')
    }

    cmake --preset windows-x64
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }

    cmake --build --preset windows-x64
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }

    if (-not $NoInstall) {
        & (Join-Path $PSScriptRoot 'install-local.ps1')
    }
}
finally {
    Pop-Location
}
