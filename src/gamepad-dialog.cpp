#include "gamepad-dialog.hpp"
#include "config.hpp"

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
    resize(820, 460);
    setModal(false);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *top = new QHBoxLayout();
    statusLabel_ = new QLabel(this);
    statusLabel_->setTextInteractionFlags(Qt::NoTextInteraction);
    top->addWidget(statusLabel_, 1);

    auto *refreshButton = new QPushButton("Refresh OBS Actions", this);
    connect(refreshButton, &QPushButton::clicked, this, [this] {
        router_.refreshHotkeys();
        rebuildTable();
    });
    top->addWidget(refreshButton);

    auto *addButton = new QPushButton("Add Mapping", this);
    connect(addButton, &QPushButton::clicked, this, [this] { addMapping(); });
    top->addWidget(addButton);
    root->addLayout(top);

    lastInputLabel_ = new QLabel("Last input: —", this);
    lastInputLabel_->setTextInteractionFlags(Qt::NoTextInteraction);
    root->addWidget(lastInputLabel_);

    table_ = new QTableWidget(this);
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({"Controller", "Gamepad input", "OBS action", ""});
    table_->verticalHeader()->setVisible(false);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    root->addWidget(table_, 1);

    auto *hint = new QLabel(
        "Default mode: B toggles Pause/Resume Recording and START toggles Start/Stop Recording. You can remove or replace either mapping.",
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
        lastInputLabel_->setText(
            QString("Last input: %1 / %2 / %3")
                .arg(QString::fromStdString(event.deviceName),
                     QString::fromStdString(event.control),
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
        table_->setItem(row, 0, new QTableWidgetItem(device));
        table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(m.control)));
        table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(m.hotkeyDisplay)));

        auto *remove = new QPushButton("Remove", table_);
        connect(remove, &QPushButton::clicked, this, [this, row] {
            removeMapping(static_cast<size_t>(row));
        });
        table_->setCellWidget(row, 3, remove);
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

void GamepadDialog::addMapping()
{
    router_.refreshHotkeys();
    const auto hotkeys = router_.hotkeys();

    QDialog dialog(this);
    dialog.setWindowTitle("Add Gamepad Mapping");
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
        inputEdit->setText(QString("%1  —  %2")
                               .arg(QString::fromStdString(event.deviceName),
                                    QString::fromStdString(event.control)));

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
