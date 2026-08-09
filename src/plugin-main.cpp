#include "config.hpp"
#include "gamepad-dialog.hpp"
#include "gamepad-manager.hpp"
#include "obs-hotkey-router.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QAction>
#include <QWidget>

#include <memory>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-gamepad-hotkeys", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
    return "Native gamepad-to-OBS hotkey router for Windows (XInput + DirectInput).";
}

namespace {

std::unique_ptr<ogh::GamepadManager> g_manager;
std::unique_ptr<ogh::ObsHotkeyRouter> g_router;
ogh::GamepadDialog *g_dialog = nullptr;
QAction *g_toolsAction = nullptr;

void ensureInputStarted()
{
    if (!g_manager || !g_router)
        return;

    HWND hwnd = static_cast<HWND>(obs_frontend_get_main_window_handle());
    if (!hwnd) {
        blog(LOG_WARNING, "[Gamepad Hotkeys] OBS main window handle is not ready yet");
        return;
    }

    g_manager->start(hwnd, [](const ogh::InputEvent &event) {
        if (g_router)
            g_router->onInputEvent(event);
    });
}

void showDialog()
{
    if (!g_manager || !g_router)
        return;

    ensureInputStarted();
    g_router->refreshHotkeys();

    if (!g_dialog) {
        auto *parent = static_cast<QWidget *>(obs_frontend_get_main_window());
        g_dialog = new ogh::GamepadDialog(*g_manager, *g_router, parent);
        g_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        QObject::connect(g_dialog, &QObject::destroyed, [] { g_dialog = nullptr; });
    }

    g_dialog->show();
    g_dialog->raise();
    g_dialog->activateWindow();
}

void frontendEvent(enum obs_frontend_event event, void *)
{
    if (!g_router)
        return;

    switch (event) {
    case OBS_FRONTEND_EVENT_FINISHED_LOADING:
        g_router->refreshHotkeys();
        ensureInputStarted();
        break;
    case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
    case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
    case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED:
    case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
        g_router->refreshHotkeys();
        break;
    case OBS_FRONTEND_EVENT_EXIT:
        if (g_manager)
            g_manager->stop();
        break;
    default:
        break;
    }
}

} // namespace

bool obs_module_load(void)
{
#ifndef _WIN32
    blog(LOG_ERROR, "[Gamepad Hotkeys] v0.1 supports Windows only");
    return false;
#else
    g_router = std::make_unique<ogh::ObsHotkeyRouter>();
    g_router->setMappings(ogh::ConfigStore::load());
    g_manager = std::make_unique<ogh::GamepadManager>();

    obs_frontend_add_event_callback(frontendEvent, nullptr);

    g_toolsAction = static_cast<QAction *>(obs_frontend_add_tools_menu_qaction("Gamepad Hotkeys"));
    if (g_toolsAction)
        QObject::connect(g_toolsAction, &QAction::triggered, [] { showDialog(); });

    blog(LOG_INFO, "[Gamepad Hotkeys] plugin loaded");
    return true;
#endif
}

void obs_module_unload(void)
{
#ifdef _WIN32
    obs_frontend_remove_event_callback(frontendEvent, nullptr);

    if (g_dialog) {
        // Delete synchronously while manager/router references are still alive.
        delete g_dialog;
        g_dialog = nullptr;
    }

    if (g_manager)
        g_manager->stop();

    g_manager.reset();
    g_router.reset();
    g_toolsAction = nullptr;

    blog(LOG_INFO, "[Gamepad Hotkeys] plugin unloaded");
#endif
}
