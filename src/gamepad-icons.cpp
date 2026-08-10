#include "gamepad-icons.hpp"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QSvgRenderer>

namespace ogh {
namespace {

constexpr qreal kDevicePixelRatio = 2.0;
constexpr int kControlIconWidth = 48;
constexpr int kControlIconHeight = 28;
constexpr int kLucideGamepadSize = 24;
constexpr int kLucideActionSize = 18;

QPixmap highDpiPixmap(int width, int height)
{
    QPixmap pixmap(qRound(width * kDevicePixelRatio), qRound(height * kDevicePixelRatio));
    pixmap.setDevicePixelRatio(kDevicePixelRatio);
    pixmap.fill(Qt::transparent);
    return pixmap;
}

QIcon renderLucide(const char *svgTemplate, const QPalette &palette, int size)
{
    const QColor stroke = palette.color(QPalette::WindowText);
    const QByteArray svg = QByteArray(svgTemplate).replace("%1", stroke.name(QColor::HexRgb).toUtf8());
    QSvgRenderer renderer(svg);

    QPixmap pixmap = highDpiPixmap(size, size);
    QPainter painter(&pixmap);
    renderer.render(&painter, QRectF(0, 0, size, size));
    return QIcon(pixmap);
}

QColor xboxFaceColor(const QString &control)
{
    if (control == "A")
        return QColor("#2E8B57");
    if (control == "B")
        return QColor("#C94C4C");
    if (control == "X")
        return QColor("#3E78B2");
    if (control == "Y")
        return QColor("#C79A2B");
    return {};
}

QString compactControlLabel(const QString &control)
{
    if (control == "DPAD_UP")
        return QString::fromUtf8("↑");
    if (control == "DPAD_DOWN")
        return QString::fromUtf8("↓");
    if (control == "DPAD_LEFT")
        return QString::fromUtf8("←");
    if (control == "DPAD_RIGHT")
        return QString::fromUtf8("→");
    if (control.startsWith("BUTTON_"))
        return "B" + control.mid(7);
    return control;
}

void drawTextBadge(QPainter &painter, const QRectF &rect, const QString &text, const QColor &fill,
                   const QColor &border, const QColor &textColor, qreal radius, int pixelSize)
{
    painter.setPen(QPen(border, 1.0));
    painter.setBrush(fill);
    painter.drawRoundedRect(rect, radius, radius);

    QFont font = QApplication::font();
    font.setPixelSize(pixelSize);
    font.setWeight(QFont::DemiBold);
    painter.setFont(font);
    painter.setPen(textColor);
    painter.drawText(rect, Qt::AlignCenter, text);
}

} // namespace

QIcon lucideGamepadIcon(const QPalette &palette)
{
    // Geometry copied from Lucide's `gamepad-2` icon (ISC licensed).
    // The distributed license notice is in data/licenses/LUCIDE-ISC.txt.
    static const char svgTemplate[] = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"
 fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
 <line x1="6" x2="10" y1="11" y2="11"/>
 <line x1="8" x2="8" y1="9" y2="13"/>
 <line x1="15" x2="15.01" y1="12" y2="12"/>
 <line x1="18" x2="18.01" y1="10" y2="10"/>
 <path d="M17.32 5H6.68a4 4 0 0 0-3.978 3.59c-.006.052-.01.101-.017.152C2.604 9.416 2 14.456 2 16a3 3 0 0 0 3 3c1 0 1.5-.5 2-1l1.414-1.414A2 2 0 0 1 9.828 16h4.344a2 2 0 0 1 1.414.586L17 18c.5.5 1 1 2 1a3 3 0 0 0 3-3c0-1.545-.604-6.584-.685-7.258-.007-.05-.011-.1-.017-.151A4 4 0 0 0 17.32 5z"/>
</svg>
)SVG";
    return renderLucide(svgTemplate, palette, kLucideGamepadSize);
}

QIcon lucideTrash2Icon(const QPalette &palette)
{
    // Lucide `trash-2` (ISC licensed).
    static const char svgTemplate[] = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"
 fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
 <path d="M3 6h18"/>
 <path d="M8 6V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/>
 <path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6"/>
 <path d="M10 11v6"/>
 <path d="M14 11v6"/>
</svg>
)SVG";
    return renderLucide(svgTemplate, palette, kLucideActionSize);
}

QIcon lucideRefreshCwIcon(const QPalette &palette)
{
    // Lucide `refresh-cw` (ISC licensed).
    static const char svgTemplate[] = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"
 fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
 <path d="M3 12a9 9 0 0 1 9-9 9.75 9.75 0 0 1 6.74 2.74L21 8"/>
 <path d="M21 3v5h-5"/>
 <path d="M21 12a9 9 0 0 1-9 9 9.75 9.75 0 0 1-6.74-2.74L3 16"/>
 <path d="M8 16H3v5"/>
</svg>
)SVG";
    return renderLucide(svgTemplate, palette, kLucideActionSize);
}

QIcon lucidePlusIcon(const QPalette &palette)
{
    // Lucide `plus` (ISC licensed).
    static const char svgTemplate[] = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"
 fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
 <path d="M5 12h14"/>
 <path d="M12 5v14"/>
</svg>
)SVG";
    return renderLucide(svgTemplate, palette, kLucideActionSize);
}

QIcon gamepadControlIcon(const QString &control, const QPalette &palette)
{
    QPixmap pixmap = highDpiPixmap(kControlIconWidth, kControlIconHeight);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QColor faceColor = xboxFaceColor(control);
    const QColor neutralFill = palette.color(QPalette::Button);
    const QColor neutralBorder = palette.color(QPalette::Mid);
    const QColor neutralText = palette.color(QPalette::ButtonText);
    const QString label = compactControlLabel(control);

    if (faceColor.isValid()) {
        const QRectF circle(12, 2, 24, 24);
        drawTextBadge(painter, circle, control, faceColor, faceColor.darker(125), Qt::white, 12.0, 12);
        return QIcon(pixmap);
    }

    if (control == "LS" || control == "RS") {
        const QRectF circle(12, 2, 24, 24);
        drawTextBadge(painter, circle, control, neutralFill, neutralBorder, neutralText, 12.0, 9);
        return QIcon(pixmap);
    }

    if (control.startsWith("DPAD_")) {
        const QRectF square(11, 2, 26, 24);
        drawTextBadge(painter, square, label, neutralFill, neutralBorder, neutralText, 7.0, 15);
        return QIcon(pixmap);
    }

    int width = 34;
    int fontSize = 9;
    if (control == "START" || control == "BACK") {
        width = 44;
        fontSize = 8;
    } else if (control.startsWith("BUTTON_")) {
        width = 38;
    }

    const QRectF pill((kControlIconWidth - width) / 2.0, 4, width, 20);
    drawTextBadge(painter, pill, label, neutralFill, neutralBorder, neutralText, 10.0, fontSize);
    return QIcon(pixmap);
}

} // namespace ogh
