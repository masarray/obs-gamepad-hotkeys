param(
    [Parameter(Mandatory = $true)]
    [string[]]$Files,
    [string]$CertificateBase64 = $env:WINDOWS_CERTIFICATE_BASE64,
    [string]$CertificatePassword = $env:WINDOWS_CERTIFICATE_PASSWORD,
    [string]$CertificatePath = $env:WINDOWS_CERTIFICATE_PATH,
    [string]$TimestampUrl = 'http://timestamp.digicert.com'
)

$ErrorActionPreference = 'Stop'

if (-not $CertificatePath -and -not $CertificateBase64) {
    Write-Host 'Code signing certificate is not configured; leaving artifacts unsigned.'
    exit 0
}

$signtool = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin\*\x64\signtool.exe" -File -ErrorAction SilentlyContinue |
    Sort-Object FullName -Descending |
    Select-Object -First 1
if (-not $signtool) {
    throw 'signtool.exe was not found in the Windows SDK.'
}

$tempPfx = $null
try {
    if ($CertificatePath) {
        $pfx = $CertificatePath
    }
    else {
        $tempBase = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { $env:TEMP }
        $tempPfx = Join-Path $tempBase 'obs-gamepad-hotkeys-signing.pfx'
        [IO.File]::WriteAllBytes($tempPfx, [Convert]::FromBase64String($CertificateBase64))
        $pfx = $tempPfx
    }

    foreach ($file in $Files) {
        if (-not (Test-Path $file)) { throw "Signing target not found: $file" }
        $signArgs = @('sign', '/fd', 'SHA256', '/f', $pfx)
        if ($CertificatePassword) { $signArgs += @('/p', $CertificatePassword) }
        $signArgs += @('/tr', $TimestampUrl, '/td', 'SHA256', $file)
        & $signtool.FullName @signArgs
        if ($LASTEXITCODE -ne 0) { throw "Signing failed: $file" }
        & $signtool.FullName verify /pa /v $file
        if ($LASTEXITCODE -ne 0) { throw "Signature verification failed: $file" }
    }
}
finally {
    if ($tempPfx) { Remove-Item $tempPfx -Force -ErrorAction SilentlyContinue }
}
