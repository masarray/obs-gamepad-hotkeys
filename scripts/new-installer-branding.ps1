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

    [int]$width = 492
    [int]$height = 942
    $bitmap = [System.Drawing.Bitmap]::new($width, $height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::ClearTypeGridFit

    $background = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
        [System.Drawing.Rectangle]::new(0, 0, $width, $height),
        [System.Drawing.ColorTranslator]::FromHtml('#08111F'),
        [System.Drawing.ColorTranslator]::FromHtml('#020712'),
        90.0)
    $accent = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#60A5FA'))
    $white = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#F8FAFC'))
    $muted = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#C2D0DF'))
    $dim = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#8DA0B5'))
    $linePen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(90, 96, 165, 250), 1.0)

    $brandFont = [System.Drawing.Font]::new('Segoe UI', 18, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $titleFont = [System.Drawing.Font]::new('Segoe UI', 38, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $subtitleFont = [System.Drawing.Font]::new('Segoe UI', 17, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $benefitFont = [System.Drawing.Font]::new('Segoe UI', 15, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $footerFont = [System.Drawing.Font]::new('Segoe UI', 14, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)

    try {
        $graphics.FillRectangle($background, 0, 0, $width, $height)

        # Real repository artwork. Keep the source image undistorted and crop it
        # with a cover fit so it remains cinematic in the tall Inno sidebar.
        Draw-ImageCover -Graphics $graphics -Image $HeroImage -X 0 -Y 88 -Width $width -Height 528

        # Darken the top for the product mark and fade the photo into the lower
        # copy area so text remains readable at every installer scaling level.
        $topShade = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(118, 2, 7, 18))
        $heroShade = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(18, 2, 7, 18))
        $fade = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
            [System.Drawing.Rectangle]::new(0, 430, $width, 240),
            [System.Drawing.Color]::FromArgb(0, 2, 7, 18),
            [System.Drawing.Color]::FromArgb(255, 2, 7, 18),
            90.0)
        try {
            $graphics.FillRectangle($heroShade, 0, 88, $width, 430)
            $graphics.FillRectangle($topShade, 0, 0, $width, 112)
            $graphics.FillRectangle($fade, 0, 430, $width, 240)
        }
        finally {
            $topShade.Dispose()
            $heroShade.Dispose()
            $fade.Dispose()
        }

        $graphics.DrawString('OBS', $brandFont, $accent, 38, 34)
        $graphics.DrawString('GAMEPAD HOTKEYS', $brandFont, $white, 88, 34)
        $graphics.DrawLine($linePen, 38, 75, 454, 75)

        $graphics.DrawString('Control OBS.', $titleFont, $white, 38, 610)
        $graphics.DrawString('From your gamepad.', $titleFont, $white, 38, 654)
        $graphics.DrawString(
            'Native controller input for recording, scenes, audio, replay buffer and more.',
            $subtitleFont,
            $muted,
            [System.Drawing.RectangleF]::new(38, 716, 416, 72))

        $bulletPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#60A5FA'), 2.0)
        try {
            $benefits = @(
                @{ Y = 808; Text = 'NO JOYTOKEY' },
                @{ Y = 838; Text = 'NO KEYBOARD EMULATION' },
                @{ Y = 868; Text = 'BACKGROUND READY' }
            )
            foreach ($benefit in $benefits) {
                [float]$y = [float]$benefit.Y
                $graphics.DrawEllipse($bulletPen, 39, $y + 3, 14, 14)
                $graphics.DrawLine($bulletPen, 43, $y + 10, 47, $y + 14)
                $graphics.DrawLine($bulletPen, 47, $y + 14, 51, $y + 7)
                $graphics.DrawString([string]$benefit.Text, $benefitFont, $accent, 64, $y)
            }
        }
        finally {
            $bulletPen.Dispose()
        }

        $graphics.DrawString('Open source • Native OBS plugin', $footerFont, $dim, 38, 912)

        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $brandFont.Dispose()
        $titleFont.Dispose()
        $subtitleFont.Dispose()
        $benefitFont.Dispose()
        $footerFont.Dispose()
        $background.Dispose()
        $accent.Dispose()
        $white.Dispose()
        $muted.Dispose()
        $dim.Dispose()
        $linePen.Dispose()
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
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality

    $borderPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#60A5FA'), 4.0)
    try {
        Draw-ImageCover -Graphics $graphics -Image $HeroImage -X 0 -Y 0 -Width $size -Height $size

        $shade = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(35, 2, 7, 18))
        try {
            $graphics.FillRectangle($shade, 0, 0, $size, $size)
        }
        finally {
            $shade.Dispose()
        }

        $graphics.DrawRectangle($borderPen, 2, 2, 251, 251)
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $borderPen.Dispose()
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
Write-Host "Installer branding: $large"
Write-Host "Installer branding: $small"

[pscustomobject]@{
    Large = $large
    Small = $small
}
