#include "gamepad-manager.hpp"
#include "xinput-backend.hpp"
#include "directinput-backend.hpp"

#include <obs-module.h>

#include <chrono>
#include <utility>

namespace ogh {

GamepadManager::GamepadManager()
{
    backends_.emplace_back(std::make_unique<XInputBackend>());
    backends_.emplace_back(std::make_unique<DirectInputBackend>());
}

GamepadManager::~GamepadManager()
{
    stop();
}

bool GamepadManager::start(HWND owner, EventHandler handler)
{
    if (running_)
        return true;

    handler_ = std::move(handler);
    bool any = false;
    for (auto &backend : backends_) {
        if (backend->initialize(owner)) {
            blog(LOG_INFO, "[Gamepad Hotkeys] %s backend initialized", backend->name());
            any = true;
        } else {
            blog(LOG_WARNING, "[Gamepad Hotkeys] %s backend failed to initialize", backend->name());
        }
    }

    running_ = true;
    worker_ = std::thread(&GamepadManager::run, this);
    return any;
}

void GamepadManager::stop()
{
    if (!running_.exchange(false))
        return;

    if (worker_.joinable())
        worker_.join();

    for (auto &backend : backends_)
        backend->shutdown();

    std::scoped_lock lock(mutex_);
    devices_.clear();
    handler_ = {};
}

void GamepadManager::run()
{
    using namespace std::chrono_literals;
    while (running_) {
        std::vector<InputEvent> events;
        std::vector<DeviceInfo> currentDevices;

        for (auto &backend : backends_) {
            backend->poll(events);
            auto list = backend->devices();
            currentDevices.insert(currentDevices.end(), list.begin(), list.end());
        }

        EventHandler handlerCopy;
        {
            std::scoped_lock lock(mutex_);
            devices_ = std::move(currentDevices);
            handlerCopy = handler_;
        }

        for (InputEvent &event : events) {
            event.sequence = ++sequence_;
            {
                std::scoped_lock lock(mutex_);
                latestEvent_ = event;
            }
            if (handlerCopy)
                handlerCopy(event);
        }

        // 8 ms gives ~125 Hz input responsiveness without a busy loop.
        std::this_thread::sleep_for(8ms);
    }
}

std::vector<DeviceInfo> GamepadManager::devices() const
{
    std::scoped_lock lock(mutex_);
    return devices_;
}

bool GamepadManager::latestEventAfter(uint64_t afterSequence, InputEvent &event) const
{
    std::scoped_lock lock(mutex_);
    if (latestEvent_.sequence <= afterSequence)
        return false;
    event = latestEvent_;
    return true;
}

uint64_t GamepadManager::latestSequence() const
{
    return sequence_.load();
}

} // namespace ogh
