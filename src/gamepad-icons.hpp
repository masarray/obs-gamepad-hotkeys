#pragma once

#include <QIcon>
#include <QPalette>
#include <QString>

namespace ogh {

QIcon lucideGamepadIcon(const QPalette &palette);
QIcon lucideTrash2Icon(const QPalette &palette);
QIcon lucideRefreshCwIcon(const QPalette &palette);
QIcon lucideRotateCcwIcon(const QPalette &palette);
QIcon lucidePlusIcon(const QPalette &palette);
QIcon gamepadControlIcon(const QString &control, const QPalette &palette);

} // namespace ogh
