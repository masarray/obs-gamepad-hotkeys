#include "gamepad-dialog.hpp"
#include "config.hpp"
#include "gamepad-icons.hpp"

#include <QCloseEvent>
#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

#include <cstddef>
#include <utility>

namespace ogh {

GamepadDialog::GamepadDialog(GamepadManager &manager, ObsHotkeyRouter &router, QWidget *parent)
    : QDialog(parent), manager_(manager), router_(router)
{
    setWindowTitle("Gamepad Hotkeys");
    setWindowIcon(lucideGamepadIcon(palette()));
    resize(820, 460);
    setModal(false);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *top = new QHBoxLayout();
    auto *brandIcon = new QLabel(this);
    brandIcon->setPixmap(lucideGamepadIcon(palette()).pixmap(QSize(24, 24)));
    brandIcon->setFixedSize(28, 28);
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setToolTip("OBS Gamepad Hotkeys");
    top->addWidget(brandIcon);

    statusLabel_ = new QLabel(this);
    statusLabel_->setTextInteractionFlags(Qt::NoTextInteraction);
    top->addWidget(statusLabel_, 1);

    // Refresh is intentionally an advanced/recovery action. Opening this
    // dialog and Add Mapping already refresh the OBS registry, while frontend
    // scene/profile events also refresh it automatically. Keep a compact
    // manual escape hatch for dynamically-added plugin hotkeys.
    auto *refreshButton = new QToolButton(this);
    refreshButton->setIcon(lucideRefreshCwIcon(refreshButton->palette()));
    refreshButton->setIconSize(QSize(17, 17));
    refreshButton->setFixedSize(32, 32);
    refreshButton->setAutoRaise(true);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setToolTip(
        "Refresh OBS actions\nUsually not needed — Add Mapping refreshes automatically.");
    refreshButton->setAccessibleName("Refresh OBS actions");
    connect(refreshButton, &QToolButton::clicked, this, [this] {
        router_.refreshHotkeys();
        rebuildTable();
    });
    top->addWidget(refreshButton);

    auto *restoreButton = new QPushButton("Restore Defaults", this);
    restoreButton->setIcon(lucideRotateCcwIcon(restoreButton->palette()));
    restoreButton->setIconSize(QSize(16, 16));
    restoreButton->setToolTip(
        "Restore B = Pause/Resume and START = Start/Stop. Other mappings are kept.");
    connect(restoreButton, &QPushButton::clicked, this, [this] { restoreDefaultMappings(); });
    top->addWidget(restoreButton);

    auto *addButton = new QPushButton("Add Mapping", this);
    addButton->setIcon(lucidePlusIcon(addButton->palette()));
    addButton->setIconSize(QSize(16, 16));
    connect(addButton, &QPushButton::clicked, this, [this] { addMapping(); });
    top->addWidget(addButton);
    root->addLayout(top);

    auto *lastInputRow = new QHBoxLayout();
    lastInputRow->setSpacing(6);
    lastInputBadge_ = new QLabel(this);
    lastInputBadge_->setFixedSize(50, 30);
    lastInputBadge_->setAlignment(Qt::AlignCenter);
    lastInputBadge_->hide();
    lastInputRow->addWidget(lastInputBadge_);

    lastInputLabel_ = new QLabel("Last input: —", this);
    lastInputLabel_->setTextInteractionFlags(Qt::NoTextInteraction);
    lastInputRow->addWidget(lastInputLabel_, 1);
    root->addLayout(lastInputRow);

    table_ = new QTableWidget(this);
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({"Controller", "Gamepad input", "OBS action", ""});
    table_->setIconSize(QSize(48, 28));
    table_->verticalHeader()->setVisible(false);
    table_->verticalHeader()->setDefaultSectionSize(34);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    table_->setColumnWidth(3, 44);
    root->addWidget(table_, 1);

    auto *hint = new QLabel(
        "Defaults: B toggles Pause/Resume Recording and START toggles Start/Stop Recording. Restore Defaults repairs these controls without removing your other mappings.",
        this);
    hint->setWordWrap(true);
    root->addWidget(hint);

    rebuildTable();
    updateStatus();

    timer_ = new QTimer(this);
    timer_->setInterval(150);
    connect(timer_, &QTimer::timeout, this, [this] { updateStatus(); });
    timer_->start();
}

GamepadDialog::~GamepadDialog()
{
    router_.setSuspended(false);
}

void GamepadDialog::closeEvent(QCloseEvent *event)
{
    router_.setSuspended(false);
    QDialog::closeEvent(event);
}

void GamepadDialog::updateStatus()
{
    const auto devices = manager_.devices();
    int xinput = 0;
    int dinput = 0;
    for (const auto &device : devices) {
        if (device.backend == "XInput")
            ++xinput;
        else if (device.backend == "DirectInput")
            ++dinput;
    }

    statusLabel_->setText(QString("Connected: %1  •  XInput %2  •  DirectInput %3")
                              .arg(static_cast<qulonglong>(devices.size())).arg(xinput).arg(dinput));

    InputEvent event;
    if (manager_.latestEventAfter(lastSeenSequence_, event)) {
        lastSeenSequence_ = event.sequence;
        const QString control = QString::fromStdString(event.control);
        lastInputBadge_->setPixmap(gamepadControlIcon(control, palette()).pixmap(QSize(48, 28)));
        lastInputBadge_->show();
        lastInputLabel_->setText(
            QString("Last input: %1 / %2 / %3")
                .arg(QString::fromStdString(event.deviceName),
                     control,
                     event.pressed ? "pressed" : "released"));
    }
}

void GamepadDialog::rebuildTable()
{
    const auto mappings = router_.mappings();
    table_->setRowCount(static_cast<int>(mappings.size()));

    for (int row = 0; row < static_cast<int>(mappings.size()); ++row) {
        const Mapping &m = mappings[static_cast<size_t>(row)];
        const QString device = m.deviceKey == "*" ? "Any Controller" : QString::fromStdString(m.deviceKey);
        const QString control = QString::fromStdString(m.control);

        table_->setItem(row, 0, new QTableWidgetItem(device));
        table_->setItem(row, 1, new QTableWidgetItem(gamepadControlIcon(control, table_->palette()), control));
        table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(m.hotkeyDisplay)));

        // Keep destructive actions visually quiet until the user points at them.
        // The wrapper layout guarantees true horizontal + vertical centering in
        // the cell instead of letting a text button stretch to row height.
        auto *removeCell = new QWidget(table_);
        auto *removeLayout = new QHBoxLayout(removeCell);
        removeLayout->setContentsMargins(0, 0, 0, 0);
        removeLayout->setSpacing(0);
        removeLayout->setAlignment(Qt::AlignCenter);

        auto *remove = new QToolButton(removeCell);
        remove->setIcon(lucideTrash2Icon(remove->palette()));
        remove->setIconSize(QSize(17, 17));
        remove->setFixedSize(28, 28);
        remove->setAutoRaise(true);
        remove->setCursor(Qt::PointingHandCursor);
        remove->setToolTip("Remove mapping");
        remove->setAccessibleName("Remove mapping");
        connect(remove, &QToolButton::clicked, this, [this, row] {
            removeMapping(static_cast<size_t>(row));
        });

        removeLayout->addWidget(remove, 0, Qt::AlignCenter);
        table_->setCellWidget(row, 3, removeCell);
    }
}

void GamepadDialog::removeMapping(size_t index)
{
    auto mappings = router_.mappings();
    if (index >= mappings.size())
        return;
    mappings.erase(mappings.begin() + static_cast<std::ptrdiff_t>(index));
    router_.setMappings(mappings);
    ConfigStore::save(mappings);
    rebuildTable();
}

void GamepadDialog::restoreDefaultMappings()
{
    QMessageBox prompt(this);
    prompt.setWindowTitle("Restore Default Controls");
    prompt.setIcon(QMessageBox::Question);
    prompt.setText("Restore the default B and START controls?");
    prompt.setInformativeText(
        "B  →  Pause / Resume Recording\n"
        "START  →  Start / Stop Recording\n\n"
        "Other custom mappings will be kept.");
    auto *restore = prompt.addButton("Restore Defaults", QMessageBox::AcceptRole);
    prompt.addButton(QMessageBox::Cancel);
    prompt.exec();
    if (prompt.clickedButton() != restore)
        return;

    // Restore only the two factory controls. This deliberately preserves all
    // unrelated custom mappings while removing any conflicting B/START mapping
    // first, so one button can never trigger both a custom and default action.
    std::vector<Mapping> restored = ConfigStore::defaults();
    for (const Mapping &mapping : router_.mappings()) {
        if (mapping.control == "B" || mapping.control == "START")
            continue;
        restored.push_back(mapping);
    }

    router_.setMappings(restored);
    if (!ConfigStore::save(restored)) {
        QMessageBox::warning(this,
                             "Gamepad Hotkeys",
                             "The default controls were restored for this session, but the configuration could not be saved.");
    }
    rebuildTable();
}

void GamepadDialog::addMapping()
{
    router_.refreshHotkeys();
    const auto hotkeys = router_.hotkeys();

    QDialog dialog(this);
    dialog.setWindowTitle("Add Gamepad Mapping");
    dialog.setWindowIcon(lucideGamepadIcon(dialog.palette()));
    dialog.resize(640, 210);

    auto *root = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *deviceCombo = new QComboBox(&dialog);
    deviceCombo->addItem("Any Controller", "*");
    for (const auto &device : manager_.devices()) {
        deviceCombo->addItem(QString("%1  (%2)")
                                 .arg(QString::fromStdString(device.name),
                                      QString::fromStdString(device.backend)),
                             QString::fromStdString(device.key));
    }
    form->addRow("Controller", deviceCombo);

    auto *inputRow = new QWidget(&dialog);
    auto *inputLayout = new QHBoxLayout(inputRow);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(6);

    auto *capturedBadge = new QLabel(inputRow);
    capturedBadge->setFixedSize(50, 30);
    capturedBadge->setAlignment(Qt::AlignCenter);
    capturedBadge->hide();
    inputLayout->addWidget(capturedBadge);

    auto *inputEdit = new QLineEdit(inputRow);
    inputEdit->setReadOnly(true);
    inputEdit->setPlaceholderText("Press Listen, then press a gamepad button");
    auto *listenButton = new QPushButton("Listen", inputRow);
    inputLayout->addWidget(inputEdit, 1);
    inputLayout->addWidget(listenButton);
    form->addRow("Gamepad input", inputRow);

    auto *actionCombo = new QComboBox(&dialog);
    actionCombo->setEditable(true);
    actionCombo->setInsertPolicy(QComboBox::NoInsert);
    actionCombo->setMaxVisibleItems(20);

    actionCombo->addItem(kSmartToggleRecordingPauseDisplay, kSmartToggleRecordingPause);
    actionCombo->addItem(kSmartToggleRecordingDisplay, kSmartToggleRecording);
    if (!hotkeys.empty())
        actionCombo->insertSeparator(actionCombo->count());

    for (const HotkeyInfo &hotkey : hotkeys) {
        actionCombo->addItem(QString::fromStdString(hotkey.display),
                             QString::fromStdString(hotkey.stableKey));
    }
    if (auto *completer = actionCombo->completer()) {
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
    }
    form->addRow("OBS action", actionCombo);
    root->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    root->addWidget(buttons);

    bool listening = false;
    uint64_t listenAfter = 0;
    std::string capturedControl;

    QTimer captureTimer(&dialog);
    captureTimer.setInterval(20);
    connect(&captureTimer, &QTimer::timeout, &dialog, [&] {
        if (!listening)
            return;
        InputEvent event;
        if (!manager_.latestEventAfter(listenAfter, event) || !event.pressed)
            return;

        listenAfter = event.sequence;
        capturedControl = event.control;
        const QString control = QString::fromStdString(event.control);
        capturedBadge->setPixmap(gamepadControlIcon(control, dialog.palette()).pixmap(QSize(48, 28)));
        capturedBadge->show();
        inputEdit->setText(QString("%1  —  %2")
                               .arg(QString::fromStdString(event.deviceName), control));

        // If a specific device was selected but another device generated the
        // learned event, switch to that device. Any Controller stays wildcard.
        if (deviceCombo->currentData().toString() != "*") {
            const int idx = deviceCombo->findData(QString::fromStdString(event.deviceKey));
            if (idx >= 0)
                deviceCombo->setCurrentIndex(idx);
        }

        listening = false;
        listenButton->setText("Listen");
        router_.setSuspended(false);
        captureTimer.stop();
    });

    connect(listenButton, &QPushButton::clicked, &dialog, [&] {
        listening = !listening;
        if (listening) {
            listenAfter = manager_.latestSequence();
            capturedBadge->hide();
            inputEdit->clear();
            inputEdit->setPlaceholderText("Listening… press one gamepad button");
            listenButton->setText("Cancel Listen");
            router_.setSuspended(true);
            captureTimer.start();
        } else {
            listenButton->setText("Listen");
            router_.setSuspended(false);
            captureTimer.stop();
        }
    });

    connect(buttons, &QDialogButtonBox::rejected, &dialog, [&] {
        router_.setSuspended(false);
        dialog.reject();
    });

    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        router_.setSuspended(false);
        if (capturedControl.empty()) {
            QMessageBox::warning(&dialog, "Gamepad Hotkeys", "Press Listen and capture a gamepad button first.");
            return;
        }
        const int actionIndex = actionCombo->currentIndex();
        if (actionIndex < 0 || actionCombo->currentData().toString().isEmpty() ||
            actionCombo->currentText() != actionCombo->itemText(actionIndex)) {
            QMessageBox::warning(&dialog, "Gamepad Hotkeys", "Choose an OBS action from the suggestion list.");
            return;
        }
        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted) {
        router_.setSuspended(false);
        return;
    }

    Mapping mapping;
    mapping.deviceKey = deviceCombo->currentData().toString().toStdString();
    mapping.control = capturedControl;
    mapping.hotkeyKey = actionCombo->currentData().toString().toStdString();
    mapping.hotkeyDisplay = actionCombo->currentText().toStdString();

    auto mappings = router_.mappings();
    mappings.emplace_back(std::move(mapping));
    router_.setMappings(mappings);
    ConfigStore::save(mappings);
    rebuildTable();
}

} // namespace ogh
