#include "trayUtils.h"
#include "resource.h"


HICON LoadIconFromResource() {
    return (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
}

void AddTrayIcon(const HWND hwnd, const HICON hIcon, const wchar_t* tooltip) {
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcsncpy_s(nid.szTip, _countof(nid.szTip) - 1, tooltip, _TRUNCATE);
    Shell_NotifyIconW(NIM_ADD, &nid);
}

void RemoveTrayIcon(const HWND hwnd) {
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}
