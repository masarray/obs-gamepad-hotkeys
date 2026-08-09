#include "directinput-backend.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <utility>

namespace ogh {

DirectInputBackend::Device::~Device()
{
    if (handle) {
        handle->Unacquire();
        handle->Release();
        handle = nullptr;
    }
}

DirectInputBackend::~DirectInputBackend()
{
    shutdown();
}

std::string DirectInputBackend::guidKey(const GUID &g)
{
    char buf[64]{};
    std::snprintf(buf, sizeof(buf),
                  "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                  static_cast<unsigned long>(g.Data1), static_cast<unsigned int>(g.Data2),
                  static_cast<unsigned int>(g.Data3),
                  static_cast<unsigned int>(g.Data4[0]), static_cast<unsigned int>(g.Data4[1]),
                  static_cast<unsigned int>(g.Data4[2]), static_cast<unsigned int>(g.Data4[3]),
                  static_cast<unsigned int>(g.Data4[4]), static_cast<unsigned int>(g.Data4[5]),
                  static_cast<unsigned int>(g.Data4[6]), static_cast<unsigned int>(g.Data4[7]));
    return std::string("dinput:") + buf;
}

std::string DirectInputBackend::utf8(const wchar_t *text)
{
    if (!text || !*text)
        return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1)
        return {};
    std::string out(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), size, nullptr, nullptr);
    out.pop_back();
    return out;
}

bool DirectInputBackend::likelyXInputDuplicate(const std::string &name)
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower.find("xinput") != std::string::npos ||
           lower.find("xbox") != std::string::npos;
}

BOOL CALLBACK DirectInputBackend::enumCallback(const DIDEVICEINSTANCEW *instance, VOID *context)
{
    auto *self = static_cast<DirectInputBackend *>(context);
    Candidate c;
    c.instance = instance->guidInstance;
    c.key = guidKey(instance->guidInstance);
    c.name = utf8(instance->tszProductName);
    if (c.name.empty())
        c.name = utf8(instance->tszInstanceName);

    // Xbox/XInput controllers are handled by the XInput backend. Skipping the
    // obvious duplicates prevents one physical press from firing twice.
    if (!likelyXInputDuplicate(c.name))
        self->enumCandidates_.emplace_back(std::move(c));
    return DIENUM_CONTINUE;
}

bool DirectInputBackend::initialize(HWND owner)
{
    owner_ = owner;
    const HRESULT hr = DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION,
                                          IID_IDirectInput8W,
                                          reinterpret_cast<void **>(&directInput_), nullptr);
    if (FAILED(hr) || !directInput_)
        return false;

    refreshDevices(nullptr);
    return true;
}

void DirectInputBackend::shutdown()
{
    devices_.clear();
    enumCandidates_.clear();
    if (directInput_) {
        directInput_->Release();
        directInput_ = nullptr;
    }
    owner_ = nullptr;
}

std::unique_ptr<DirectInputBackend::Device>
DirectInputBackend::createDevice(const Candidate &candidate)
{
    IDirectInputDevice8W *handle = nullptr;
    if (FAILED(directInput_->CreateDevice(candidate.instance, &handle, nullptr)) || !handle)
        return nullptr;

    if (FAILED(handle->SetDataFormat(&c_dfDIJoystick2))) {
        handle->Release();
        return nullptr;
    }

    if (FAILED(handle->SetCooperativeLevel(owner_, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
        handle->Release();
        return nullptr;
    }

    // Acquire may fail temporarily; pollDevice retries when the device becomes available.
    handle->Acquire();

    auto device = std::make_unique<Device>();
    device->handle = handle;
    device->instance = candidate.instance;
    device->key = candidate.key;
    device->name = candidate.name.empty() ? "DirectInput Controller" : candidate.name;
    device->connected = true;
    return device;
}

void DirectInputBackend::refreshDevices(std::vector<InputEvent> *events)
{
    if (!directInput_)
        return;

    enumCandidates_.clear();
    directInput_->EnumDevices(DI8DEVCLASS_GAMECTRL, &DirectInputBackend::enumCallback,
                              this, DIEDFL_ATTACHEDONLY);

    // Remove devices that disappeared. Always release logically-held buttons first.
    for (auto it = devices_.begin(); it != devices_.end();) {
        const bool stillPresent = std::any_of(enumCandidates_.begin(), enumCandidates_.end(),
                                             [&](const Candidate &c) { return c.key == (*it)->key; });
        if (!stillPresent) {
            if (events)
                emitReleaseAll(*(*it), *events);
            it = devices_.erase(it);
        } else {
            ++it;
        }
    }

    // Add newly attached devices, preserving state for devices already known.
    for (const Candidate &candidate : enumCandidates_) {
        const bool exists = std::any_of(devices_.begin(), devices_.end(),
                                        [&](const auto &d) { return d->key == candidate.key; });
        if (!exists) {
            auto device = createDevice(candidate);
            if (device)
                devices_.emplace_back(std::move(device));
        }
    }

    lastEnumeration_ = std::chrono::steady_clock::now();
}

void DirectInputBackend::emitDiff(Device &device, const std::array<bool, 132> &next,
                                  std::vector<InputEvent> &events)
{
    static constexpr const char *dpadNames[4] = {
        "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"};

    for (size_t i = 0; i < next.size(); ++i) {
        if (device.controls[i] == next[i])
            continue;

        std::string control;
        if (i < 128)
            control = "BUTTON_" + std::to_string(i + 1);
        else
            control = dpadNames[i - 128];

        events.push_back({0, device.key, device.name, std::move(control), next[i]});
    }
    device.controls = next;
}

void DirectInputBackend::emitReleaseAll(Device &device, std::vector<InputEvent> &events)
{
    static constexpr const char *dpadNames[4] = {
        "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"};

    for (size_t i = 0; i < device.controls.size(); ++i) {
        if (!device.controls[i])
            continue;

        std::string control;
        if (i < 128)
            control = "BUTTON_" + std::to_string(i + 1);
        else
            control = dpadNames[i - 128];
        events.push_back({0, device.key, device.name, std::move(control), false});
    }
    device.controls.fill(false);
}

void DirectInputBackend::pollDevice(Device &device, std::vector<InputEvent> &events)
{
    HRESULT hr = device.handle->Poll();
    if (FAILED(hr)) {
        hr = device.handle->Acquire();
        for (int retry = 0; hr == DIERR_INPUTLOST && retry < 3; ++retry)
            hr = device.handle->Acquire();
        if (FAILED(hr) && hr != S_FALSE)
            return;
        device.handle->Poll();
    }

    DIJOYSTATE2 state{};
    hr = device.handle->GetDeviceState(sizeof(state), &state);
    if (FAILED(hr))
        return;

    std::array<bool, 132> next{};
    for (size_t i = 0; i < 128; ++i)
        next[i] = (state.rgbButtons[i] & 0x80) != 0;

    const DWORD pov = state.rgdwPOV[0];
    if (LOWORD(pov) != 0xFFFF) {
        const DWORD angle = pov % 36000;
        next[128] = angle >= 31500 || angle <= 4500;   // up
        next[131] = angle >= 4500 && angle <= 13500;  // right
        next[129] = angle >= 13500 && angle <= 22500; // down
        next[130] = angle >= 22500 && angle <= 31500; // left
    }

    emitDiff(device, next, events);
}

void DirectInputBackend::poll(std::vector<InputEvent> &events)
{
    if (!directInput_)
        return;

    const auto now = std::chrono::steady_clock::now();
    if (lastEnumeration_.time_since_epoch().count() == 0 ||
        now - lastEnumeration_ >= std::chrono::seconds(2))
        refreshDevices(&events);

    for (auto &device : devices_)
        pollDevice(*device, events);
}

std::vector<DeviceInfo> DirectInputBackend::devices() const
{
    std::vector<DeviceInfo> out;
    out.reserve(devices_.size());
    for (const auto &device : devices_)
        out.push_back({device->key, device->name, "DirectInput", true});
    return out;
}

} // namespace ogh
