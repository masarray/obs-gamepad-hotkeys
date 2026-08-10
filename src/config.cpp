#include "config.hpp"

#include <obs-module.h>
#include <util/bmem.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <utility>

namespace ogh {

static QString configPath()
{
    char *path = obs_module_config_path("config.json");
    if (!path)
        return {};
    QString result = QString::fromUtf8(path);
    bfree(path);
    return result;
}

bool ConfigStore::exists()
{
    const QString path = configPath();
    return !path.isEmpty() && QFileInfo::exists(path);
}

std::vector<Mapping> ConfigStore::defaults()
{
    return {
        Mapping{"*", "B", kSmartToggleRecordingPause, kSmartToggleRecordingPauseDisplay},
        Mapping{"*", "START", kSmartToggleRecording, kSmartToggleRecordingDisplay},
    };
}

std::vector<Mapping> ConfigStore::load()
{
    std::vector<Mapping> out;
    const QString path = configPath();
    if (path.isEmpty())
        return out;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return out;

    QJsonParseError error{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
        return out;

    const QJsonArray items = doc.object().value("mappings").toArray();
    for (const QJsonValue &value : items) {
        const QJsonObject obj = value.toObject();
        Mapping m;
        m.deviceKey = obj.value("device").toString("*").toStdString();
        m.control = obj.value("control").toString().toStdString();
        m.hotkeyKey = obj.value("hotkey_key").toString().toStdString();
        m.hotkeyDisplay = obj.value("hotkey_display").toString().toStdString();
        if (!m.control.empty() && !m.hotkeyKey.empty())
            out.emplace_back(std::move(m));
    }
    return out;
}

bool ConfigStore::save(const std::vector<Mapping> &mappings)
{
    const QString path = configPath();
    if (path.isEmpty())
        return false;

    QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath()))
        return false;

    QJsonArray items;
    for (const Mapping &m : mappings) {
        QJsonObject obj;
        obj.insert("device", QString::fromStdString(m.deviceKey));
        obj.insert("control", QString::fromStdString(m.control));
        obj.insert("hotkey_key", QString::fromStdString(m.hotkeyKey));
        obj.insert("hotkey_display", QString::fromStdString(m.hotkeyDisplay));
        items.append(obj);
    }

    QJsonObject root;
    root.insert("schema", 1);
    root.insert("mappings", items);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) < 0)
        return false;
    return file.commit();
}

} // namespace ogh
