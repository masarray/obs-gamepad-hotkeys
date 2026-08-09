#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include "input-backend.hpp"
#include <dinput.h>

#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace ogh {

class DirectInputBackend final : public InputBackend {
public:
    ~DirectInputBackend() override;
    const char *name() const override { return "DirectInput"; }
    bool initialize(HWND owner) override;
    void shutdown() override;
    void poll(std::vector<InputEvent> &events) override;
    std::vector<DeviceInfo> devices() const override;

private:
    struct Candidate {
        GUID instance{};
        std::string key;
        std::string name;
    };

    struct Device {
        IDirectInputDevice8W *handle = nullptr;
        GUID instance{};
        std::string key;
        std::string name;
        std::array<bool, 132> controls{};
        bool connected = true;

        ~Device();
    };

    HWND owner_ = nullptr;
    IDirectInput8W *directInput_ = nullptr;
    std::vector<std::unique_ptr<Device>> devices_;
    std::vector<Candidate> enumCandidates_;
    std::chrono::steady_clock::time_point lastEnumeration_{};

    static BOOL CALLBACK enumCallback(const DIDEVICEINSTANCEW *instance, VOID *context);
    static std::string guidKey(const GUID &guid);
    static std::string utf8(const wchar_t *text);
    static bool likelyXInputDuplicate(const std::string &name);

    void refreshDevices(std::vector<InputEvent> *events);
    std::unique_ptr<Device> createDevice(const Candidate &candidate);
    void pollDevice(Device &device, std::vector<InputEvent> &events);
    void emitDiff(Device &device, const std::array<bool, 132> &next,
                  std::vector<InputEvent> &events);
    static void emitReleaseAll(Device &device, std::vector<InputEvent> &events);
};

} // namespace ogh
