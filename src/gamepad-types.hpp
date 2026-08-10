#pragma once

#include <string>
#include <cstddef>
#include <vector>
#include <cstdint>

namespace ogh {

inline constexpr char kSmartToggleRecordingPause[] = "smart:toggle-recording-pause";
inline constexpr char kSmartToggleRecording[] = "smart:toggle-recording";
inline constexpr char kSmartToggleRecordingPauseDisplay[] = "Smart: Toggle Pause / Resume Recording";
inline constexpr char kSmartToggleRecordingDisplay[] = "Smart: Toggle Start / Stop Recording";

struct DeviceInfo {
    std::string key;
    std::string name;
    std::string backend;
    bool connected = false;
};

struct InputEvent {
    uint64_t sequence = 0;
    std::string deviceKey;
    std::string deviceName;
    std::string control;
    bool pressed = false;
};

struct Mapping {
    std::string deviceKey = "*";
    std::string control;
    std::string hotkeyKey;
    std::string hotkeyDisplay;
};

struct HotkeyInfo {
    size_t id = static_cast<size_t>(-1);
    std::string stableKey;
    std::string name;
    std::string description;
    std::string owner;
    std::string ownerDisplay;
    std::string display;
    int registererType = 0;
};

} // namespace ogh
