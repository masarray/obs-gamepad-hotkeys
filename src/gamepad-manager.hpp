#pragma once

#include "gamepad-types.hpp"
#include "input-backend.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace ogh {

class GamepadManager {
public:
    using EventHandler = std::function<void(const InputEvent &)>;

    GamepadManager();
    ~GamepadManager();

    bool start(HWND owner, EventHandler handler);
    void stop();

    std::vector<DeviceInfo> devices() const;
    bool latestEventAfter(uint64_t afterSequence, InputEvent &event) const;
    uint64_t latestSequence() const;

private:
    void run();

    mutable std::mutex mutex_;
    std::vector<std::unique_ptr<InputBackend>> backends_;
    std::vector<DeviceInfo> devices_;
    InputEvent latestEvent_{};
    EventHandler handler_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> sequence_{0};
};

} // namespace ogh
