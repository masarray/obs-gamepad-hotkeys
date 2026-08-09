#pragma once

#include "gamepad-manager.hpp"
#include "obs-hotkey-router.hpp"

#include <QDialog>

class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

namespace ogh {

class GamepadDialog final : public QDialog {
public:
    GamepadDialog(GamepadManager &manager, ObsHotkeyRouter &router, QWidget *parent = nullptr);
    ~GamepadDialog() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void rebuildTable();
    void updateStatus();
    void addMapping();
    void removeMapping(size_t index);

    GamepadManager &manager_;
    ObsHotkeyRouter &router_;
    QLabel *statusLabel_ = nullptr;
    QLabel *lastInputLabel_ = nullptr;
    QTableWidget *table_ = nullptr;
    QTimer *timer_ = nullptr;
    uint64_t lastSeenSequence_ = 0;
};

} // namespace ogh
