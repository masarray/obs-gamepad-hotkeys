param(
    [string]$Path = 'dist'
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Target = if ([System.IO.Path]::IsPathRooted($Path)) { $Path } else { Join-Path $Root $Path }

if (-not (Test-Path $Target)) {
    throw "Checksum target does not exist: $Target"
}

Get-ChildItem -Path $Target -File | Where-Object { $_.Extension -in '.exe', '.zip' } | ForEach-Object {
    $hash = (Get-FileHash -Path $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    $checksumPath = "$($_.FullName).sha256"
    "$hash *$($_.Name)" | Set-Content -Path $checksumPath -Encoding ascii
    Write-Host "SHA-256: $checksumPath"
}
