param(
    [Parameter(Mandatory = $true)]
    [string]$OutputDir
)

$ErrorActionPreference = 'Stop'

if ($env:OS -ne 'Windows_NT') {
    throw 'Installer branding assets can only be generated on Windows.'
}

Add-Type -AssemblyName System.Drawing

function New-RoundedRectanglePath {
    param(
        [float]$X,
        [float]$Y,
        [float]$Width,
        [float]$Height,
        [float]$Radius
    )

    $path = [System.Drawing.Drawing2D.GraphicsPath]::new()
    $diameter = $Radius * 2.0
    $path.AddArc($X, $Y, $diameter, $diameter, 180, 90)
    $path.AddArc($X + $Width - $diameter, $Y, $diameter, $diameter, 270, 90)
    $path.AddArc($X + $Width - $diameter, $Y + $Height - $diameter, $diameter, $diameter, 0, 90)
    $path.AddArc($X, $Y + $Height - $diameter, $diameter, $diameter, 90, 90)
    $path.CloseFigure()
    return $path
}

function Draw-Gamepad {
    param(
        [System.Drawing.Graphics]$Graphics,
        [float]$X,
        [float]$Y,
        [float]$Width,
        [float]$Height,
        [float]$Scale = 1.0
    )

    $bodyPath = New-RoundedRectanglePath -X $X -Y $Y -Width $Width -Height ($Height * 0.72) -Radius ($Height * 0.22)
    $bodyBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(42, 255, 255, 255))
    $outline = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(220, 235, 243, 255), 4.0 * $Scale)
    $outline.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round

    $gripBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(32, 255, 255, 255))
    $gripPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(175, 219, 234, 255), 3.0 * $Scale)

    try {
        $Graphics.FillPath($bodyBrush, $bodyPath)
        $Graphics.DrawPath($outline, $bodyPath)

        $gripW = $Width * 0.29
        $gripH = $Height * 0.47
        $Graphics.FillEllipse($gripBrush, $X + $Width * 0.03, $Y + $Height * 0.43, $gripW, $gripH)
        $Graphics.FillEllipse($gripBrush, $X + $Width * 0.68, $Y + $Height * 0.43, $gripW, $gripH)
        $Graphics.DrawArc($gripPen, $X + $Width * 0.03, $Y + $Height * 0.43, $gripW, $gripH, 80, 200)
        $Graphics.DrawArc($gripPen, $X + $Width * 0.68, $Y + $Height * 0.43, $gripW, $gripH, -100, 200)

        $controlPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(235, 241, 247, 255), 5.0 * $Scale)
        try {
            $dpadX = $X + $Width * 0.24
            $dpadY = $Y + $Height * 0.32
            $arm = $Height * 0.10
            $Graphics.DrawLine($controlPen, $dpadX - $arm, $dpadY, $dpadX + $arm, $dpadY)
            $Graphics.DrawLine($controlPen, $dpadX, $dpadY - $arm, $dpadX, $dpadY + $arm)

            $buttonRadius = $Height * 0.045
            $buttonCenters = @(
                @($X + $Width * 0.72, $Y + $Height * 0.25, '#60A5FA'),
                @($X + $Width * 0.79, $Y + $Height * 0.32, '#F87171'),
                @($X + $Width * 0.65, $Y + $Height * 0.32, '#34D399'),
                @($X + $Width * 0.72, $Y + $Height * 0.39, '#FBBF24')
            )
            foreach ($button in $buttonCenters) {
                $pen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml($button[2]), 4.0 * $Scale)
                try {
                    $Graphics.DrawEllipse(
                        $pen,
                        [float]$button[0] - $buttonRadius,
                        [float]$button[1] - $buttonRadius,
                        $buttonRadius * 2,
                        $buttonRadius * 2)
                }
                finally {
                    $pen.Dispose()
                }
            }

            $centerPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(155, 219, 234, 255), 3.0 * $Scale)
            try {
                $Graphics.DrawLine($centerPen, $X + $Width * 0.44, $Y + $Height * 0.25, $X + $Width * 0.49, $Y + $Height * 0.25)
                $Graphics.DrawLine($centerPen, $X + $Width * 0.53, $Y + $Height * 0.25, $X + $Width * 0.58, $Y + $Height * 0.25)
            }
            finally {
                $centerPen.Dispose()
            }
        }
        finally {
            $controlPen.Dispose()
        }
    }
    finally {
        $bodyPath.Dispose()
        $bodyBrush.Dispose()
        $outline.Dispose()
        $gripBrush.Dispose()
        $gripPen.Dispose()
    }
}

function New-LargeWizardImage {
    param([string]$Path)

    $width = 492
    $height = 942
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
    $linePen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(55, 148, 197, 255), 1.0)

    $badgeFont = [System.Drawing.Font]::new('Segoe UI', 18, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $titleFont = [System.Drawing.Font]::new('Segoe UI', 38, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $subtitleFont = [System.Drawing.Font]::new('Segoe UI', 18, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $smallFont = [System.Drawing.Font]::new('Segoe UI', 15, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)

    try {
        $graphics.FillRectangle($background, 0, 0, $width, $height)

        # Soft accent glow without depending on raster artwork checked into git.
        for ($i = 7; $i -ge 1; $i--) {
            $alpha = 8 + (7 - $i) * 2
            $glowBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb($alpha, 96, 165, 250))
            try {
                $diameter = 400 + ($i * 28)
                $graphics.FillEllipse($glowBrush, ($width - $diameter) / 2, 190 - ($i * 10), $diameter, $diameter)
            }
            finally {
                $glowBrush.Dispose()
            }
        }

        $graphics.DrawString('OBS', $badgeFont, $accent, 48, 48)
        $graphics.DrawString('GAMEPAD HOTKEYS', $badgeFont, $white, 98, 48)
        $graphics.DrawLine($linePen, 48, 86, 444, 86)

        Draw-Gamepad -Graphics $graphics -X 56 -Y 190 -Width 380 -Height 270 -Scale 1.15

        $graphics.DrawString('Control OBS.', $titleFont, $white, 48, 540)
        $graphics.DrawString('From your gamepad.', $titleFont, $white, 48, 586)
        $graphics.DrawString('Native controller input for recording, scenes, audio, replay buffer and more.', $subtitleFont, $muted, [System.Drawing.RectangleF]::new(48, 658, 396, 96))

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
        $linePen.Dispose()
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

function New-SmallWizardImage {
    param([string]$Path)

    $size = 256
    $bitmap = [System.Drawing.Bitmap]::new($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.Clear([System.Drawing.Color]::Transparent)

    $tilePath = New-RoundedRectanglePath -X 12 -Y 12 -Width 232 -Height 232 -Radius 52
    $tileBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
        [System.Drawing.Rectangle]::new(12, 12, 232, 232),
        [System.Drawing.ColorTranslator]::FromHtml('#172033'),
        [System.Drawing.ColorTranslator]::FromHtml('#0A1424'),
        45.0)
    $tilePen = [System.Drawing.Pen]::new([System.Drawing.ColorTranslator]::FromHtml('#4C8DDF'), 5.0)

    try {
        $graphics.FillPath($tileBrush, $tilePath)
        $graphics.DrawPath($tilePen, $tilePath)
        Draw-Gamepad -Graphics $graphics -X 40 -Y 68 -Width 176 -Height 128 -Scale 0.75
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $tilePath.Dispose()
        $tileBrush.Dispose()
        $tilePen.Dispose()
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
