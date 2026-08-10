#pragma once

#include <QIcon>
#include <QPalette>
#include <QString>

namespace ogh {

QIcon lucideGamepadIcon(const QPalette &palette);
QIcon gamepadControlIcon(const QString &control, const QPalette &palette);

} // namespace ogh
