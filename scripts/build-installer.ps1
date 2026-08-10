param(
    [switch]$SkipBootstrap,
    [switch]$SkipBuild,
    [switch]$Run,
    [switch]$OpenOutput
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$BuildRoot = Join-Path $Root 'build_x64'
$Dist = Join-Path $Root 'dist'
$PluginName = 'obs-gamepad-hotkeys'
$BuildSpec = Get-Content (Join-Path $Root 'buildspec.json') -Raw | ConvertFrom-Json
$Version = [string]$BuildSpec.version

if ($env:OS -ne 'Windows_NT') {
    throw 'The Windows installer can only be built on Windows.'
}

function Initialize-BuildToolchain {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw 'Visual Studio 2022 was not found. Install Visual Studio 2022 with Desktop development with C++.'
    }

    $vsPath = (& $vswhere -latest -version '[17.0,18.0)' -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
    if (-not $vsPath) {
        throw 'Visual Studio 2022 C++ tools were not found. Add the Desktop development with C++ workload in Visual Studio Installer.'
    }

    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) {
        $bundledCMake = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
        if (Test-Path $bundledCMake) {
            $env:PATH = "$(Split-Path $bundledCMake -Parent);$env:PATH"
            $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
        }
    }

    if (-not $cmakeCommand) {
        throw 'CMake was not found. Install CMake 3.28+ or add the CMake tools component in Visual Studio Installer.'
    }

    $versionLine = (& cmake --version | Select-Object -First 1)
    if ($versionLine -notmatch 'cmake version\s+(\d+)\.(\d+)\.(\d+)') {
        throw "Could not determine the CMake version: $versionLine"
    }
    $cmakeVersion = [version]"$($Matches[1]).$($Matches[2]).$($Matches[3])"
    if ($cmakeVersion -lt [version]'3.28.0') {
        throw "CMake $cmakeVersion is too old. Install CMake 3.28 or newer."
    }

    Write-Host "Visual Studio: $vsPath"
    Write-Host "CMake: $($cmakeCommand.Source) ($cmakeVersion)"
}

Initialize-BuildToolchain

Push-Location $Root
try {
    if (-not $SkipBuild) {
        if (-not $SkipBootstrap -and -not (Test-Path (Join-Path $Root 'cmake\common\bootstrap.cmake'))) {
            & (Join-Path $PSScriptRoot 'bootstrap-template.ps1')
        }

        cmake --preset windows-x64
        if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }

        cmake --build --preset windows-x64
        if ($LASTEXITCODE -ne 0) { throw 'Plugin build failed.' }
    }

    $dll = Get-ChildItem -Path $BuildRoot -Filter "$PluginName.dll" -Recurse -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
    if (-not $dll) {
        throw "Could not find $PluginName.dll under $BuildRoot. Build the plugin first."
    }

    $dataDir = Join-Path $Root 'data'
    if (-not (Test-Path $dataDir)) {
        throw "Plugin data directory not found: $dataDir"
    }

    New-Item -ItemType Directory -Path $Dist -Force | Out-Null

    # Generate branded, high-DPI PNGs every build so the source repository
    # remains text-only while the final Setup EXE still has product artwork.
    $brandingDir = Join-Path $Root 'installer\generated'
    $branding = & (Join-Path $PSScriptRoot 'new-installer-branding.ps1') -OutputDir $brandingDir
    $wizardLarge = [string]$branding.Large
    $wizardSmall = [string]$branding.Small
    if (-not (Test-Path $wizardLarge) -or -not (Test-Path $wizardSmall)) {
        throw 'Installer branding assets are missing after generation.'
    }

    $generatedInclude = Join-Path $Root 'installer\build.generated.iss'
    $escapeIss = {
        param([string]$Value)
        return $Value.Replace('"', '""')
    }
    $defines = @(
        ('#define MyAppVersion "{0}"' -f (& $escapeIss $Version)),
        ('#define PluginDll "{0}"' -f (& $escapeIss $dll.FullName)),
        ('#define PluginDataDir "{0}"' -f (& $escapeIss $dataDir)),
        ('#define InstallerOutputDir "{0}"' -f (& $escapeIss $Dist)),
        ('#define WizardLargeImage "{0}"' -f (& $escapeIss $wizardLarge)),
        ('#define WizardSmallImage "{0}"' -f (& $escapeIss $wizardSmall))
    )
    $signInstaller = [bool]($env:WINDOWS_CERTIFICATE_BASE64 -or $env:WINDOWS_CERTIFICATE_PATH)
    if ($signInstaller) {
        $defines += '#define EnableInnoSigning'
    }
    $defines | Set-Content -Path $generatedInclude -Encoding utf8

    $iscc = & (Join-Path $PSScriptRoot 'ensure-inno.ps1')
    if (-not $iscc -or -not (Test-Path $iscc)) {
        throw 'Could not resolve Inno Setup compiler (ISCC.exe).'
    }

    $iss = Join-Path $Root 'installer\obs-gamepad-hotkeys.iss'
    if ($signInstaller) {
        $signOne = Join-Path $PSScriptRoot 'sign-one-file.ps1'
        $signCommand = 'powershell.exe -NoProfile -ExecutionPolicy Bypass -File $q' + $signOne + '$q $f'
        & $iscc "/Sobs-sign=$signCommand" $iss
    }
    else {
        & $iscc $iss
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Inno Setup compilation failed with exit code $LASTEXITCODE."
    }

    $installer = Join-Path $Dist "OBS-Gamepad-Hotkeys-Setup-v$Version.exe"
    if (-not (Test-Path $installer)) {
        throw "Installer compilation reported success, but output was not found: $installer"
    }

    & (Join-Path $PSScriptRoot 'write-checksums.ps1') -Path $Dist

    Write-Host ''
    Write-Host 'Installer ready:' -ForegroundColor Green
    Write-Host $installer -ForegroundColor Cyan

    if ($OpenOutput) {
        Start-Process explorer.exe -ArgumentList "/select,`"$installer`""
    }

    if ($Run) {
        Write-Host 'Launching the installer exactly as an end user would...' -ForegroundColor Green
        Start-Process -FilePath $installer
    }
}
finally {
    Pop-Location
}
