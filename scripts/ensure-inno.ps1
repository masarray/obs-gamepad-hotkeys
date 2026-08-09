param()

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$ToolRoot = Join-Path $Root '.tools\inno-setup'
$LocalIscc = Join-Path $ToolRoot 'Inno Setup 6\ISCC.exe'

$candidates = @(
    $LocalIscc,
    (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
    (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe'),
    (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe')
) | Where-Object { $_ -and $_.Trim() }

foreach ($candidate in $candidates) {
    if (Test-Path $candidate) {
        $resolved = (Resolve-Path $candidate).Path
        Write-Host "Using existing Inno Setup compiler: $resolved"
        Write-Output $resolved
        exit 0
    }
}

$Version = '6.7.3'
$InstallerUrl = "https://github.com/jrsoftware/issrc/releases/download/is-6_7_3/innosetup-$Version.exe"
$ExpectedSha256 = '9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732'
$DownloadDir = Join-Path $ToolRoot 'download'
$InstallerPath = Join-Path $DownloadDir "innosetup-$Version.exe"
$InstallDir = Join-Path $ToolRoot 'Inno Setup 6'

New-Item -ItemType Directory -Path $DownloadDir -Force | Out-Null

if (-not (Test-Path $InstallerPath)) {
    Write-Host "Downloading Inno Setup $Version from the official JRSoftware GitHub release..."
    Invoke-WebRequest -Uri $InstallerUrl -OutFile $InstallerPath -UseBasicParsing
}

$actualHash = (Get-FileHash -Path $InstallerPath -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actualHash -ne $ExpectedSha256) {
    Remove-Item $InstallerPath -Force -ErrorAction SilentlyContinue
    throw "Inno Setup download hash mismatch. Expected $ExpectedSha256, got $actualHash."
}

New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
Write-Host 'Installing the local build-only copy of Inno Setup...'
$process = Start-Process -FilePath $InstallerPath -ArgumentList @(
    '/VERYSILENT',
    '/SUPPRESSMSGBOXES',
    '/NORESTART',
    '/CURRENTUSER',
    "/DIR=$InstallDir"
) -Wait -PassThru

if ($process.ExitCode -ne 0) {
    throw "Inno Setup bootstrap failed with exit code $($process.ExitCode)."
}

if (-not (Test-Path $LocalIscc)) {
    throw "Inno Setup installed but ISCC.exe was not found at $LocalIscc."
}

Write-Output (Resolve-Path $LocalIscc).Path
