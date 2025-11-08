#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include "desktopUtils.h"

// Callback function for EnumWindows
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    // Retrieve the vector of HWNDs passed as lParam
    std::vector<HWND>* workerwWindows = reinterpret_cast<std::vector<HWND>*>(lParam);

    // Get the class name of the window
    wchar_t className[256];
    if (GetClassName(hwnd, className, sizeof(className))) {
        // Check if the class name is "WorkerW"
        if (std::wstring(className) == L"WorkerW") {
            workerwWindows->push_back(hwnd);
        }
    }
    return TRUE; // Continue enumeration
}

// Function to create and retrieve the WorkerW layer
static HWND CreateWorkerWLayer() {
    // Get the handle to the Progman window
    HWND progman = FindWindow(L"Progman", nullptr);
    if (!progman) {
        return nullptr;
    }

    // Send a message to Progman to spawn a WorkerW window
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    // Vector to store WorkerW windows
    std::vector<HWND> workerwWindows;

    // Enumerate all top-level windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&workerwWindows));

    // If we found WorkerW windows, return the last one
    if (!workerwWindows.empty()) {
        return workerwWindows.back();
    }

    return nullptr;
}

// Gets the location of the currently using wallpaper
wchar_t* GetCurrentWallpaper() {
    const wchar_t* regKey = L"Control Panel\\Desktop";
    const wchar_t* regValue = L"Wallpaper";
    HKEY hKey;

    // Open the registry key
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, regKey, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Failed to open registry key, error code: " + std::to_string(result));
    }

    // Query the size of the value
    DWORD valueSize = 0;
    result = RegQueryValueEx(hKey, regValue, nullptr, nullptr, nullptr, &valueSize);
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        throw std::runtime_error("Failed to query registry value size, error code: " + std::to_string(result));
    }

    // Allocate buffer for the value
    wchar_t* valueBuffer = new wchar_t[valueSize / sizeof(wchar_t)];
    result = RegQueryValueEx(hKey, regValue, nullptr, nullptr, reinterpret_cast<LPBYTE>(valueBuffer), &valueSize);
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        delete[] valueBuffer; // Free allocated memory in case of failure
        throw std::runtime_error("Failed to query registry value, error code: " + std::to_string(result));
    }

    // Close the registry key
    RegCloseKey(hKey);

    return valueBuffer;
}

void SetAsDesktop(HWND hwnd) {
    // Call the function to create or retrieve the WorkerW layer
    HWND workerw = CreateWorkerWLayer();
    if (!workerw) {
        return;
    }

    // Set the parent of the target window to WorkerW
    SetParent(hwnd, workerw);
}
