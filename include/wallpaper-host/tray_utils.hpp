#pragma once
#include <windows.h>
#include <string>

// Export / import macros
// - When building/using a DLL, define BUILDING_WALLPAPER_HOST
// - When linking against the static library, define WALLPAPER_HOST_STATIC
#if defined(WALLPAPER_HOST_STATIC)
#  define WALLPAPER_HOST_API
#elif defined(BUILDING_WALLPAPER_HOST)
#  define WALLPAPER_HOST_API __declspec(dllexport)
#else
#  define WALLPAPER_HOST_API __declspec(dllimport)
#endif

namespace wallpaper {
namespace tray {

/** Custom message used to request that the window show a tray context menu.
 *  The library posts a pointer (MenuRequest*) in lParam when posting this message.
 */
constexpr UINT WM_TRAYICON = WM_USER + 20;
constexpr UINT WM_TRAY_SHOW_MENU = WM_USER + 21;
constexpr UINT DEFAULT_TRAY_ICON_ID = 1;

/**
 * Registers / adds a tray icon for 'hwnd'. The caller retains ownership of 'icon'.
 *
 * @param hwnd Window that receives tray notifications (WM_TRAYICON).
 * @param icon Icon handle (caller must destroy via DestroyIcon when no longer needed).
 * @param tooltip Null-terminated tooltip text (UTF-16). If empty, no tip is set.
 * @param iconId Numeric id for the icon (default DEFAULT_TRAY_ICON_ID).
 * @return true on success.
 */
WALLPAPER_HOST_API bool RegisterIcon(HWND hwnd, HICON icon, const std::wstring& tooltip, UINT iconId = DEFAULT_TRAY_ICON_ID);

/**
 * Removes the tray icon associated with 'hwnd' / iconId.
 *
 * @param hwnd Owner window.
 * @param iconId Icon id used when registering (default DEFAULT_TRAY_ICON_ID).
 * @return true on success.
 */
WALLPAPER_HOST_API bool UnregisterIcon(HWND hwnd, UINT iconId = DEFAULT_TRAY_ICON_ID);

/**
 * Posts a request to the window to display the provided popup menu at screenPos.
 * This function is non-blocking. It allocates a small request object and posts
 * WM_TRAY_SHOW_MENU to the window; the window (on the UI thread) will perform
 * TrackPopupMenu and post WM_COMMAND if an item is chosen.
 *
 * The window's message loop **must** call tray::HandleWindowMessage(...) from
 * its WindowProc to process WM_TRAY_SHOW_MENU (the library supplies the handler).
 *
 * @param hwnd      Owner window (must be the same HWND used with RegisterIcon).
 * @param menu      HMENU to display. The menu is owned by the caller and must remain valid
 *                  until the window processes WM_TRAY_SHOW_MENU. (Typical: create menu
 *                  for each popup or reuse a static menu that outlives the call.)
 * @param screenPos Position in screen coordinates where the menu should appear.
 */
WALLPAPER_HOST_API void PostShowContextMenuAsync(HWND hwnd, HMENU menu, POINT screenPos);

/**
 * Call this at the top of your WindowProc to allow the library to handle internal messages.
 *
 * Example:
 *   LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
 *       if (wallpaper::tray::HandleWindowMessage(hwnd, msg, wp, lp)) return 0;
 *       ...
 *   }
 *
 * @return true if the function handled the message (app should return 0), false otherwise.
 */
WALLPAPER_HOST_API bool HandleWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * Helper to build a simple default popup menu. Caller should destroy the menu via DestroyMenu().
 * This is convenience only — your real app will typically build a menu with AppendMenu / CreatePopupMenu.
 */
WALLPAPER_HOST_API HMENU CreateDefaultPopupMenu();

} // namespace tray
} // namespace wallpaper
