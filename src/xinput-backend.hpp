#pragma once

#include "input-backend.hpp"
#include <Xinput.h>
#include <array>

namespace ogh {

class XInputBackend final : public InputBackend {
public:
    const char *name() const override { return "XInput"; }
    bool initialize(HWND owner) override;
    void shutdown() override;
    void poll(std::vector<InputEvent> &events) override;
    std::vector<DeviceInfo> devices() const override;

private:
    struct SlotState {
        bool connected = false;
        DWORD packet = 0;
        std::array<bool, 16> controls{};
    };

    using XInputGetStateFn = DWORD(WINAPI *)(DWORD, XINPUT_STATE *);
    HMODULE module_ = nullptr;
    XInputGetStateFn getState_ = nullptr;
    std::array<SlotState, XUSER_MAX_COUNT> slots_{};

    static std::array<bool, 16> decode(const XINPUT_STATE &state);
    void emitDiff(DWORD slot, const std::array<bool, 16> &next,
                  std::vector<InputEvent> &events);
    void emitReleaseAll(DWORD slot, std::vector<InputEvent> &events);
};

} // namespace ogh
