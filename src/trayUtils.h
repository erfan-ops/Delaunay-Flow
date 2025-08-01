#pragma once

#include <Windows.h>

#define WM_TRAYICON (WM_USER + 20)
#define TRAY_ICON_ID 1;

// loads the icon
HICON LoadIconFromResource();

// adds the icon to system tray
void AddTrayIcon(const HWND hwnd, const HICON hIcon, const wchar_t* tooltip);

// removes the icon from system tray
void RemoveTrayIcon(const HWND hwnd);
