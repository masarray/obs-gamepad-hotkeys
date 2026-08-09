cmake_minimum_required(VERSION 3.28...3.30)

# This project intentionally follows the official obsproject/obs-plugintemplate
# build system. Run scripts/bootstrap-template.ps1 once if the cmake/ and
# build-aux/ support folders are not present.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

if(NOT WIN32)
  message(FATAL_ERROR "obs-gamepad-hotkeys v0.1 currently supports Windows only")
endif()

option(ENABLE_FRONTEND_API "Use obs-frontend-api" ON)
option(ENABLE_QT "Use Qt" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    OBS::libobs
    OBS::obs-frontend-api
    Qt6::Core
    Qt6::Widgets
    dinput8
    dxguid
)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DIRECTINPUT_VERSION=0x0800 NOMINMAX WIN32_LEAN_AND_MEAN)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.cpp
    src/gamepad-types.hpp
    src/input-backend.hpp
    src/xinput-backend.hpp
    src/xinput-backend.cpp
    src/directinput-backend.hpp
    src/directinput-backend.cpp
    src/gamepad-manager.hpp
    src/gamepad-manager.cpp
    src/obs-hotkey-router.hpp
    src/obs-hotkey-router.cpp
    src/config.hpp
    src/config.cpp
    src/gamepad-dialog.hpp
    src/gamepad-dialog.cpp
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
