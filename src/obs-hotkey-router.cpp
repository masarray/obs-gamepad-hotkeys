#include "obs-hotkey-router.hpp"

#include <obs.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include <algorithm>
#include <memory>
#include <unordered_set>
#include <utility>

namespace ogh {

ObsHotkeyRouter::ObsHotkeyRouter() = default;

std::string ObsHotkeyRouter::ownerIdentity(obs_hotkey_t *hotkey, std::string *displayName)
{
    if (displayName)
        displayName->clear();

    const auto type = obs_hotkey_get_registerer_type(hotkey);
    void *registerer = obs_hotkey_get_registerer(hotkey);

    switch (type) {
    case OBS_HOTKEY_REGISTERER_FRONTEND:
        if (displayName)
            *displayName = "OBS Frontend";
        return "frontend";
    case OBS_HOTKEY_REGISTERER_SOURCE: {
        if (!registerer)
            return "source:expired";
        auto *source = obs_weak_source_get_source(static_cast<obs_weak_source_t *>(registerer));
        if (!source)
            return "source:expired";
        const char *uuid = obs_source_get_uuid(source);
        const char *name = obs_source_get_name(source);
        if (displayName)
            *displayName = name ? name : "Source";
        std::string owner = "source:";
        owner += (uuid && *uuid) ? uuid : (name ? name : "unknown");
        obs_source_release(source);
        return owner;
    }
    case OBS_HOTKEY_REGISTERER_OUTPUT: {
        if (!registerer)
            return "output:expired";
        auto *output = obs_weak_output_get_output(static_cast<obs_weak_output_t *>(registerer));
        if (!output)
            return "output:expired";
        const char *name = obs_output_get_name(output);
        if (displayName)
            *displayName = name ? name : "Output";
        std::string owner = "output:" + std::string(name ? name : "unknown");
        obs_output_release(output);
        return owner;
    }
    case OBS_HOTKEY_REGISTERER_ENCODER: {
        if (!registerer)
            return "encoder:expired";
        auto *encoder = obs_weak_encoder_get_encoder(static_cast<obs_weak_encoder_t *>(registerer));
        if (!encoder)
            return "encoder:expired";
        const char *name = obs_encoder_get_name(encoder);
        if (displayName)
            *displayName = name ? name : "Encoder";
        std::string owner = "encoder:" + std::string(name ? name : "unknown");
        obs_encoder_release(encoder);
        return owner;
    }
    case OBS_HOTKEY_REGISTERER_SERVICE: {
        if (!registerer)
            return "service:expired";
        auto *service = obs_weak_service_get_service(static_cast<obs_weak_service_t *>(registerer));
        if (!service)
            return "service:expired";
        const char *name = obs_service_get_name(service);
        if (displayName)
            *displayName = name ? name : "Service";
        std::string owner = "service:" + std::string(name ? name : "unknown");
        obs_service_release(service);
        return owner;
    }
    default:
        if (displayName)
            *displayName = "Unknown owner";
        return "unknown";
    }
}

std::string ObsHotkeyRouter::makeStableKey(int type, const std::string &owner, const char *name)
{
    // Do not include the localized description in persistent identity. OBS can
    // change UI language between launches while the internal hotkey name and
    // owning object identity remain stable.
    std::string key = std::to_string(type);
    key += '|';
    key += owner;
    key += '|';
    key += name ? name : "";
    return key;
}

bool ObsHotkeyRouter::enumHotkey(void *data, size_t id, obs_hotkey_t *hotkey)
{
    auto *items = static_cast<std::vector<HotkeyInfo> *>(data);
    const char *name = obs_hotkey_get_name(hotkey);
    const char *description = obs_hotkey_get_description(hotkey);
    const int type = static_cast<int>(obs_hotkey_get_registerer_type(hotkey));
    std::string ownerDisplay;
    const std::string owner = ownerIdentity(hotkey, &ownerDisplay);

    HotkeyInfo info;
    info.id = id;
    info.name = name ? name : "";
    info.description = description ? description : "";
    info.registererType = type;
    info.owner = owner;
    info.ownerDisplay = ownerDisplay;
    info.stableKey = makeStableKey(type, owner, name);

    if (!info.description.empty())
        info.display = info.description;
    else if (!info.name.empty())
        info.display = info.name;
    else
        info.display = "Unnamed OBS hotkey";

    if (!info.name.empty() && info.name != info.display)
        info.display += "  [" + info.name + "]";
    if (!info.ownerDisplay.empty() && info.ownerDisplay != "OBS Frontend")
        info.display += "  {" + info.ownerDisplay + "}";

    items->emplace_back(std::move(info));
    return true;
}

void ObsHotkeyRouter::refreshHotkeys()
{
    // Keep routing blocked while the runtime id table is rebuilt. Otherwise a
    // worker-thread press could land on an old id between release and swap.
    std::vector<size_t> releaseIds;
    size_t count = 0;
    {
        std::scoped_lock lock(mutex_);

        releaseIds.reserve(activePressCounts_.size());
        for (const auto &[id, activeCount] : activePressCounts_) {
            if (activeCount > 0)
                releaseIds.push_back(id);
        }
        activePressCounts_.clear();

        std::vector<HotkeyInfo> items;
        obs_enum_hotkeys(&ObsHotkeyRouter::enumHotkey, &items);

        std::sort(items.begin(), items.end(), [](const HotkeyInfo &a, const HotkeyInfo &b) {
            return a.display < b.display;
        });

        std::unordered_map<std::string, size_t> runtimeIds;
        for (const HotkeyInfo &item : items)
            runtimeIds[item.stableKey] = item.id;

        count = items.size();
        hotkeys_ = std::move(items);
        runtimeIds_ = std::move(runtimeIds);
    }

    for (size_t id : releaseIds) {
        auto *payload = new DispatchPayload{id, false};
        obs_queue_task(OBS_TASK_UI, &ObsHotkeyRouter::dispatchUiTask, payload, false);
    }

    blog(LOG_INFO, "[Gamepad Hotkeys] OBS hotkey registry refreshed (%zu entries)", count);
}

std::vector<HotkeyInfo> ObsHotkeyRouter::hotkeys() const
{
    std::scoped_lock lock(mutex_);
    return hotkeys_;
}

void ObsHotkeyRouter::releaseAllActive()
{
    std::vector<size_t> ids;
    {
        std::scoped_lock lock(mutex_);
        ids.reserve(activePressCounts_.size());
        for (const auto &[id, count] : activePressCounts_) {
            if (count > 0)
                ids.push_back(id);
        }
        activePressCounts_.clear();
        activeSmartPressCounts_.clear();
    }

    for (size_t id : ids) {
        auto *payload = new DispatchPayload{id, false};
        obs_queue_task(OBS_TASK_UI, &ObsHotkeyRouter::dispatchUiTask, payload, false);
    }
}

void ObsHotkeyRouter::setSuspended(bool suspended)
{
    const bool wasSuspended = suspended_.exchange(suspended);
    if (suspended && !wasSuspended)
        releaseAllActive();
}

void ObsHotkeyRouter::setMappings(std::vector<Mapping> mappings)
{
    // Replace mapping and clear hold state atomically relative to controller
    // events so an event cannot sneak in on the old mapping and become latched.
    std::vector<size_t> releaseIds;
    {
        std::scoped_lock lock(mutex_);
        releaseIds.reserve(activePressCounts_.size());
        for (const auto &[id, activeCount] : activePressCounts_) {
            if (activeCount > 0)
                releaseIds.push_back(id);
        }
        activePressCounts_.clear();
        activeSmartPressCounts_.clear();
        mappings_ = std::move(mappings);
    }

    for (size_t id : releaseIds) {
        auto *payload = new DispatchPayload{id, false};
        obs_queue_task(OBS_TASK_UI, &ObsHotkeyRouter::dispatchUiTask, payload, false);
    }
}

std::vector<Mapping> ObsHotkeyRouter::mappings() const
{
    std::scoped_lock lock(mutex_);
    return mappings_;
}

void ObsHotkeyRouter::dispatchUiTask(void *data)
{
    std::unique_ptr<DispatchPayload> payload(static_cast<DispatchPayload *>(data));
    if (!payload || payload->id == OBS_INVALID_HOTKEY_ID)
        return;

    // OBS Studio's frontend enables libobs hotkey callback rerouting and uses
    // this exact entry point to invoke the registered hotkey callback on the UI thread.
    obs_hotkey_trigger_routed_callback(payload->id, payload->pressed);
}

void ObsHotkeyRouter::dispatchSmartUiTask(void *data)
{
    std::unique_ptr<SmartDispatchPayload> payload(static_cast<SmartDispatchPayload *>(data));
    if (!payload)
        return;

    if (payload->actionKey == kSmartToggleRecordingPause) {
        if (!obs_frontend_recording_active()) {
            blog(LOG_DEBUG, "[Gamepad Hotkeys] Pause/resume ignored because recording is not active");
            return;
        }

        const bool paused = obs_frontend_recording_paused();
        obs_frontend_recording_pause(!paused);
        blog(LOG_INFO, "[Gamepad Hotkeys] recording %s from smart gamepad action",
             paused ? "resumed" : "paused");
        return;
    }

    if (payload->actionKey == kSmartToggleRecording) {
        if (obs_frontend_recording_active()) {
            blog(LOG_INFO, "[Gamepad Hotkeys] stopping recording from smart gamepad action");
            obs_frontend_recording_stop();
        } else {
            blog(LOG_INFO, "[Gamepad Hotkeys] starting recording from smart gamepad action");
            obs_frontend_recording_start();
        }
    }
}

void ObsHotkeyRouter::onInputEvent(const InputEvent &event)
{
    if (suspended_)
        return;

    std::vector<DispatchPayload> dispatches;
    std::vector<std::string> smartDispatches;
    {
        std::scoped_lock lock(mutex_);
        if (suspended_)
            return;

        std::unordered_set<size_t> uniqueHotkeys;
        std::unordered_set<std::string> uniqueSmartStates;

        for (const Mapping &mapping : mappings_) {
            if (mapping.control != event.control)
                continue;
            if (mapping.deviceKey != "*" && mapping.deviceKey != event.deviceKey)
                continue;

            if (mapping.hotkeyKey == kSmartToggleRecordingPause || mapping.hotkeyKey == kSmartToggleRecording) {
                // Key smart-action edge state by action + control. This lets two
                // different buttons target the same smart action independently,
                // while deduplicating duplicate backend reports of one button.
                const std::string stateKey = mapping.hotkeyKey + "|" + mapping.control;
                if (!uniqueSmartStates.insert(stateKey).second)
                    continue;

                if (event.pressed) {
                    unsigned int &count = activeSmartPressCounts_[stateKey];
                    ++count;
                    if (count == 1)
                        smartDispatches.push_back(mapping.hotkeyKey);
                } else {
                    const auto active = activeSmartPressCounts_.find(stateKey);
                    if (active == activeSmartPressCounts_.end() || active->second == 0)
                        continue;
                    --active->second;
                    if (active->second == 0)
                        activeSmartPressCounts_.erase(active);
                }
                continue;
            }

            const auto runtime = runtimeIds_.find(mapping.hotkeyKey);
            if (runtime == runtimeIds_.end() || !uniqueHotkeys.insert(runtime->second).second)
                continue;

            const size_t id = runtime->second;
            if (event.pressed) {
                unsigned int &count = activePressCounts_[id];
                ++count;
                if (count == 1)
                    dispatches.push_back({id, true});
            } else {
                const auto active = activePressCounts_.find(id);
                if (active == activePressCounts_.end() || active->second == 0)
                    continue;
                --active->second;
                if (active->second == 0) {
                    activePressCounts_.erase(active);
                    dispatches.push_back({id, false});
                }
            }
        }
    }

    for (const DispatchPayload &dispatch : dispatches) {
        auto *payload = new DispatchPayload{dispatch.id, dispatch.pressed};
        obs_queue_task(OBS_TASK_UI, &ObsHotkeyRouter::dispatchUiTask, payload, false);
    }

    for (const std::string &actionKey : smartDispatches) {
        auto *payload = new SmartDispatchPayload{actionKey};
        obs_queue_task(OBS_TASK_UI, &ObsHotkeyRouter::dispatchSmartUiTask, payload, false);
    }
}

} // namespace ogh
