#pragma once

#include "gamepad-types.hpp"
#include <obs.h>

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ogh {

class ObsHotkeyRouter {
public:
    ObsHotkeyRouter();

    void refreshHotkeys();
    std::vector<HotkeyInfo> hotkeys() const;

    void setMappings(std::vector<Mapping> mappings);
    std::vector<Mapping> mappings() const;

    void onInputEvent(const InputEvent &event);
    void setSuspended(bool suspended);
    bool suspended() const { return suspended_.load(); }

private:
    struct DispatchPayload {
        size_t id = static_cast<size_t>(-1);
        bool pressed = false;
    };

    struct SmartDispatchPayload {
        std::string actionKey;
    };

    static bool enumHotkey(void *data, size_t id, obs_hotkey_t *hotkey);
    static void dispatchUiTask(void *data);
    static void dispatchSmartUiTask(void *data);
    void releaseAllActive();
    static std::string ownerIdentity(obs_hotkey_t *hotkey, std::string *displayName);
    static std::string makeStableKey(int type, const std::string &owner, const char *name);

    mutable std::mutex mutex_;
    std::vector<HotkeyInfo> hotkeys_;
    std::unordered_map<std::string, size_t> runtimeIds_;
    std::unordered_map<size_t, unsigned int> activePressCounts_;
    std::unordered_map<std::string, unsigned int> activeSmartPressCounts_;
    std::vector<Mapping> mappings_;
    std::atomic<bool> suspended_{false};
};

} // namespace ogh
