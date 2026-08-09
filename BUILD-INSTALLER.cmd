@echo off
setlocal
cd /d "%~dp0"
title OBS Gamepad Hotkeys - Build Installer
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build-installer.ps1" -OpenOutput
if errorlevel 1 (
  echo.
  echo BUILD FAILED. See the message above.
  pause
  exit /b 1
)
exit /b 0
