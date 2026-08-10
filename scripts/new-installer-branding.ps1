param(
    [Parameter(Mandatory = $true)]
    [string]$OutputDir,
    [string]$SourceImage
)

$ErrorActionPreference = 'Stop'

if ($env:OS -ne 'Windows_NT') {
    throw 'Installer branding assets can only be generated on Windows.'
}

Add-Type -AssemblyName System.Drawing

$Root = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($SourceImage)) {
    $SourceImage = Join-Path $Root 'gamepad.jpg'
}
$SourceImage = [System.IO.Path]::GetFullPath($SourceImage)

if (-not (Test-Path $SourceImage -PathType Leaf)) {
    throw "Installer hero image was not found: $SourceImage"
}

function Set-HighQualityGraphics {
    param([System.Drawing.Graphics]$Graphics)

    $Graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $Graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $Graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $Graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
}

function Draw-ImageContain {
    param(
        [System.Drawing.Graphics]$Graphics,
        [System.Drawing.Image]$Image,
        [float]$X,
        [float]$Y,
        [float]$Width,
        [float]$Height
    )

    [float]$scale = [Math]::Min($Width / [float]$Image.Width, $Height / [float]$Image.Height)
    [float]$drawWidth = [float]$Image.Width * $scale
    [float]$drawHeight = [float]$Image.Height * $scale
    [float]$drawX = $X + (($Width - $drawWidth) / 2.0)
    [float]$drawY = $Y + (($Height - $drawHeight) / 2.0)

    $destinationRect = [System.Drawing.RectangleF]::new($drawX, $drawY, $drawWidth, $drawHeight)
    $sourceRect = [System.Drawing.RectangleF]::new(0.0, 0.0, [float]$Image.Width, [float]$Image.Height)
    $Graphics.DrawImage($Image, $destinationRect, $sourceRect, [System.Drawing.GraphicsUnit]::Pixel)
}

function Draw-ImageCover {
    param(
        [System.Drawing.Graphics]$Graphics,
        [System.Drawing.Image]$Image,
        [float]$X,
        [float]$Y,
        [float]$Width,
        [float]$Height
    )

    [float]$sourceWidth = $Image.Width
    [float]$sourceHeight = $Image.Height
    [float]$destinationAspect = $Width / $Height
    [float]$sourceAspect = $sourceWidth / $sourceHeight

    [float]$cropX = 0.0
    [float]$cropY = 0.0
    [float]$cropWidth = $sourceWidth
    [float]$cropHeight = $sourceHeight

    if ($sourceAspect -gt $destinationAspect) {
        $cropWidth = $sourceHeight * $destinationAspect
        $cropX = ($sourceWidth - $cropWidth) / 2.0
    }
    elseif ($sourceAspect -lt $destinationAspect) {
        $cropHeight = $sourceWidth / $destinationAspect
        $cropY = ($sourceHeight - $cropHeight) / 2.0
    }

    $destinationRect = [System.Drawing.RectangleF]::new($X, $Y, $Width, $Height)
    $sourceRect = [System.Drawing.RectangleF]::new($cropX, $cropY, $cropWidth, $cropHeight)
    $Graphics.DrawImage($Image, $destinationRect, $sourceRect, [System.Drawing.GraphicsUnit]::Pixel)
}

function New-LargeWizardImage {
    param(
        [string]$Path,
        [System.Drawing.Image]$HeroImage
    )

    # 3x the classic Inno Setup 164x314 wizard image. The repository artwork
    # already contains the complete visual design and copy, so do not draw any
    # additional text, badges, fades, or overlays on top of it.
    [int]$width = 492
    [int]$height = 942
    $bitmap = [System.Drawing.Bitmap]::new($width, $height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    Set-HighQualityGraphics -Graphics $graphics

    $background = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#020712'))
    try {
        $graphics.FillRectangle($background, 0, 0, $width, $height)
        # Preserve the entire authored artwork. Contain-fit avoids clipping any
        # text that is already baked into gamepad.jpg.
        Draw-ImageContain -Graphics $graphics -Image $HeroImage -X 0 -Y 0 -Width $width -Height $height
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $background.Dispose()
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

function New-SmallWizardImage {
    param(
        [string]$Path,
        [System.Drawing.Image]$HeroImage
    )

    [int]$size = 256
    $bitmap = [System.Drawing.Bitmap]::new($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    Set-HighQualityGraphics -Graphics $graphics

    try {
        # Small wizard art is image-only as well. A centered cover crop keeps
        # the gamepad visual recognizable without adding duplicate copy.
        Draw-ImageCover -Graphics $graphics -Image $HeroImage -X 0 -Y 0 -Width $size -Height $size
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
$large = Join-Path $OutputDir 'wizard-large.png'
$small = Join-Path $OutputDir 'wizard-small.png'

$hero = [System.Drawing.Image]::FromFile($SourceImage)
try {
    New-LargeWizardImage -Path $large -HeroImage $hero
    New-SmallWizardImage -Path $small -HeroImage $hero
}
finally {
    $hero.Dispose()
}

if (-not (Test-Path $large) -or -not (Test-Path $small)) {
    throw 'Installer branding image generation did not produce the expected PNG files.'
}

Write-Host "Installer hero source: $SourceImage"
Write-Host 'Installer hero mode: image-only (no generated text overlay)'
Write-Host "Installer branding: $large"
Write-Host "Installer branding: $small"

[pscustomobject]@{
    Large = $large
    Small = $small
}
