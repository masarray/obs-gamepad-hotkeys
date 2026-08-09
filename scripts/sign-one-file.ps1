param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$File
)

$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'sign-windows.ps1') -Files $File
