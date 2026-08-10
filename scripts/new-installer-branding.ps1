param(
    [Parameter(Mandatory = $true)]
    [string]$OutputDir
)

$ErrorActionPreference = 'Stop'

if ($env:OS -ne 'Windows_NT') {
    throw 'Installer branding assets can only be generated on Windows.'
}

Add-Type -AssemblyName System.Drawing

function New-LargeWizardImage {
    param([string]$Path)

    [int]$width = 492
    [int]$height = 942
    $bitmap = [System.Drawing.Bitmap]::new($width, $height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::ClearTypeGridFit

    $background = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
        [System.Drawing.Rectangle]::new(0, 0, $width, $height),
        [System.Drawing.ColorTranslator]::FromHtml('#111827'),
        [System.Drawing.ColorTranslator]::FromHtml('#07111F'),
        90.0)
    $accent = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#60A5FA'))
    $white = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#F8FAFC'))
    $muted = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#AFC3D9'))
    $dim = [System.Drawing.SolidBrush]::new([System.Drawing.ColorTranslator]::FromHtml('#7890A8'))
    $bodyBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(42, 255, 255, 255))
    $bodyPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(220, 235, 243, 255), 4.0)
    $controlPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(235, 241, 247, 255), 5.0)
    $linePen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(55, 148, 197, 255), 1.0)
    $bluePen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#60A5FA'), 4.0)
    $redPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#F87171'), 4.0)
    $greenPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#34D399'), 4.0)
    $yellowPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#FBBF24'), 4.0)

    $badgeFont = [System.Drawing.Font]::new('Segoe UI', 18, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $titleFont = [System.Drawing.Font]::new('Segoe UI', 38, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $subtitleFont = [System.Drawing.Font]::new('Segoe UI', 18, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $smallFont = [System.Drawing.Font]::new('Segoe UI', 15, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)

    try {
        $graphics.FillRectangle($background, 0, 0, $width, $height)

        $glow = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(20, 96, 165, 250))
        try {
            $graphics.FillEllipse($glow, 8, 145, 476, 476)
        }
        finally {
            $glow.Dispose()
        }

        $graphics.DrawString('OBS', $badgeFont, $accent, 48, 48)
        $graphics.DrawString('GAMEPAD HOTKEYS', $badgeFont, $white, 98, 48)
        $graphics.DrawLine($linePen, 48, 86, 444, 86)

        # Gamepad silhouette: deliberately built from simple primitives so the
        # branding generator is reliable on Windows PowerShell and pwsh alike.
        $graphics.FillEllipse($bodyBrush, 58, 282, 176, 206)
        $graphics.FillEllipse($bodyBrush, 258, 282, 176, 206)
        $graphics.FillRectangle($bodyBrush, 128, 250, 236, 180)
        $graphics.DrawEllipse($bodyPen, 58, 282, 176, 206)
        $graphics.DrawEllipse($bodyPen, 258, 282, 176, 206)
        $graphics.DrawLine($bodyPen, 128, 250, 364, 250)
        $graphics.DrawLine($bodyPen, 128, 430, 364, 430)

        # D-pad.
        $graphics.DrawLine($controlPen, 142, 333, 202, 333)
        $graphics.DrawLine($controlPen, 172, 303, 172, 363)

        # Familiar A/B/X/Y color accents without using any vendor logo.
        $graphics.DrawEllipse($bluePen, 316, 292, 24, 24)
        $graphics.DrawEllipse($redPen, 346, 322, 24, 24)
        $graphics.DrawEllipse($greenPen, 286, 322, 24, 24)
        $graphics.DrawEllipse($yellowPen, 316, 352, 24, 24)

        # Back / Start marks.
        $graphics.DrawLine($linePen, 220, 318, 242, 318)
        $graphics.DrawLine($linePen, 254, 318, 276, 318)

        $graphics.DrawString('Control OBS.', $titleFont, $white, 48, 540)
        $graphics.DrawString('From your gamepad.', $titleFont, $white, 48, 586)
        $graphics.DrawString(
            'Native controller input for recording, scenes, audio, replay buffer and more.',
            $subtitleFont,
            $muted,
            [System.Drawing.RectangleF]::new(48, 658, 396, 96))

        $graphics.DrawString('NO JOYTOKEY', $smallFont, $accent, 48, 790)
        $graphics.DrawString('NO KEYBOARD EMULATION', $smallFont, $accent, 48, 818)
        $graphics.DrawString('BACKGROUND READY', $smallFont, $accent, 48, 846)
        $graphics.DrawString('Open source • Native OBS plugin', $smallFont, $dim, 48, 892)

        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $badgeFont.Dispose()
        $titleFont.Dispose()
        $subtitleFont.Dispose()
        $smallFont.Dispose()
        $background.Dispose()
        $accent.Dispose()
        $white.Dispose()
        $muted.Dispose()
        $dim.Dispose()
        $bodyBrush.Dispose()
        $bodyPen.Dispose()
        $controlPen.Dispose()
        $linePen.Dispose()
        $bluePen.Dispose()
        $redPen.Dispose()
        $greenPen.Dispose()
        $yellowPen.Dispose()
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

function New-SmallWizardImage {
    param([string]$Path)

    [int]$size = 256
    $bitmap = [System.Drawing.Bitmap]::new($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.Clear([System.Drawing.Color]::Transparent)

    $tileBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
        [System.Drawing.Rectangle]::new(12, 12, 232, 232),
        [System.Drawing.ColorTranslator]::FromHtml('#172033'),
        [System.Drawing.ColorTranslator]::FromHtml('#0A1424'),
        45.0)
    $tilePen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#4C8DDF'), 5.0)
    $bodyBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(34, 255, 255, 255))
    $bodyPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(225, 235, 243, 255), 4.0)
    $controlPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(235, 241, 247, 255), 4.0)
    $redPen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#F87171'), 4.0)
    $bluePen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#60A5FA'), 4.0)

    try {
        $graphics.FillEllipse($tileBrush, 12, 12, 232, 232)
        $graphics.DrawEllipse($tilePen, 12, 12, 232, 232)

        $graphics.FillEllipse($bodyBrush, 43, 106, 78, 86)
        $graphics.FillEllipse($bodyBrush, 135, 106, 78, 86)
        $graphics.FillRectangle($bodyBrush, 88, 92, 80, 70)
        $graphics.DrawEllipse($bodyPen, 43, 106, 78, 86)
        $graphics.DrawEllipse($bodyPen, 135, 106, 78, 86)
        $graphics.DrawLine($bodyPen, 88, 92, 168, 92)

        $graphics.DrawLine($controlPen, 73, 127, 101, 127)
        $graphics.DrawLine($controlPen, 87, 113, 87, 141)
        $graphics.DrawEllipse($bluePen, 158, 111, 14, 14)
        $graphics.DrawEllipse($redPen, 176, 129, 14, 14)

        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $tileBrush.Dispose()
        $tilePen.Dispose()
        $bodyBrush.Dispose()
        $bodyPen.Dispose()
        $controlPen.Dispose()
        $redPen.Dispose()
        $bluePen.Dispose()
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
$large = Join-Path $OutputDir 'wizard-large.png'
$small = Join-Path $OutputDir 'wizard-small.png'

New-LargeWizardImage -Path $large
New-SmallWizardImage -Path $small

if (-not (Test-Path $large) -or -not (Test-Path $small)) {
    throw 'Installer branding image generation did not produce the expected PNG files.'
}

Write-Host "Installer branding: $large"
Write-Host "Installer branding: $small"

[pscustomobject]@{
    Large = $large
    Small = $small
}
