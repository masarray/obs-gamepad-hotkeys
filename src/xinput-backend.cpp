#include "xinput-backend.hpp"

#include <array>
#include <string>

namespace ogh {

bool XInputBackend::initialize(HWND)
{
    static constexpr std::array<const wchar_t *, 3> candidates = {
        L"xinput1_4.dll", L"xinput1_3.dll", L"xinput9_1_0.dll"};

    for (const wchar_t *dll : candidates) {
        module_ = LoadLibraryW(dll);
        if (!module_)
            continue;
        getState_ = reinterpret_cast<XInputGetStateFn>(GetProcAddress(module_, "XInputGetState"));
        if (getState_)
            return true;
        FreeLibrary(module_);
        module_ = nullptr;
    }
    return false;
}

void XInputBackend::shutdown()
{
    for (auto &slot : slots_)
        slot = {};
    getState_ = nullptr;
    if (module_) {
        FreeLibrary(module_);
        module_ = nullptr;
    }
}

std::array<bool, 16> XInputBackend::decode(const XINPUT_STATE &s)
{
    const WORD b = s.Gamepad.wButtons;
    return {
        (b & XINPUT_GAMEPAD_A) != 0,
        (b & XINPUT_GAMEPAD_B) != 0,
        (b & XINPUT_GAMEPAD_X) != 0,
        (b & XINPUT_GAMEPAD_Y) != 0,
        (b & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0,
        (b & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0,
        (b & XINPUT_GAMEPAD_BACK) != 0,
        (b & XINPUT_GAMEPAD_START) != 0,
        (b & XINPUT_GAMEPAD_LEFT_THUMB) != 0,
        (b & XINPUT_GAMEPAD_RIGHT_THUMB) != 0,
        (b & XINPUT_GAMEPAD_DPAD_UP) != 0,
        (b & XINPUT_GAMEPAD_DPAD_DOWN) != 0,
        (b & XINPUT_GAMEPAD_DPAD_LEFT) != 0,
        (b & XINPUT_GAMEPAD_DPAD_RIGHT) != 0,
        s.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
        s.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
    };
}

void XInputBackend::emitDiff(DWORD slot, const std::array<bool, 16> &next,
                             std::vector<InputEvent> &events)
{
    static constexpr const char *controlNames[16] = {
        "A", "B", "X", "Y", "LB", "RB", "BACK", "START",
        "LS", "RS", "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT", "LT", "RT"};

    SlotState &old = slots_[slot];
    const std::string key = "xinput:" + std::to_string(slot);
    const std::string deviceName = "XInput Controller " + std::to_string(slot + 1);

    for (size_t i = 0; i < next.size(); ++i) {
        if (old.controls[i] != next[i])
            events.push_back({0, key, deviceName, controlNames[i], next[i]});
    }
    old.controls = next;
}

void XInputBackend::emitReleaseAll(DWORD slot, std::vector<InputEvent> &events)
{
    static constexpr const char *controlNames[16] = {
        "A", "B", "X", "Y", "LB", "RB", "BACK", "START",
        "LS", "RS", "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT", "LT", "RT"};

    SlotState &old = slots_[slot];
    const std::string key = "xinput:" + std::to_string(slot);
    const std::string deviceName = "XInput Controller " + std::to_string(slot + 1);
    for (size_t i = 0; i < old.controls.size(); ++i) {
        if (old.controls[i])
            events.push_back({0, key, deviceName, controlNames[i], false});
    }
    old.controls.fill(false);
}

void XInputBackend::poll(std::vector<InputEvent> &events)
{
    if (!getState_)
        return;

    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
        XINPUT_STATE state{};
        const DWORD result = getState_(i, &state);
        SlotState &slot = slots_[i];

        if (result != ERROR_SUCCESS) {
            if (slot.connected)
                emitReleaseAll(i, events);
            slot.connected = false;
            slot.packet = 0;
            continue;
        }

        if (!slot.connected) {
            slot.connected = true;
            slot.packet = state.dwPacketNumber;
            emitDiff(i, decode(state), events);
            continue;
        }

        if (slot.packet == state.dwPacketNumber)
            continue;
        slot.packet = state.dwPacketNumber;
        emitDiff(i, decode(state), events);
    }
}

std::vector<DeviceInfo> XInputBackend::devices() const
{
    std::vector<DeviceInfo> out;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
        if (!slots_[i].connected)
            continue;
        out.push_back({"xinput:" + std::to_string(i),
                       "XInput Controller " + std::to_string(i + 1), "XInput", true});
    }
    return out;
}

} // namespace ogh
