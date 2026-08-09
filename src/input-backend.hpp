#pragma once

#include "gamepad-types.hpp"
#include <windows.h>
#include <vector>

namespace ogh {

class InputBackend {
public:
    virtual ~InputBackend() = default;
    virtual const char *name() const = 0;
    virtual bool initialize(HWND owner) = 0;
    virtual void shutdown() = 0;
    virtual void poll(std::vector<InputEvent> &events) = 0;
    virtual std::vector<DeviceInfo> devices() const = 0;
};

} // namespace ogh
