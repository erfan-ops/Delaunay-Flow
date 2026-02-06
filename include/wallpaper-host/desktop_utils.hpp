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
namespace desktop {

/**
 * Returns the absolute path of the current desktop wallpaper.
 *
 * The returned std::wstring is owned by the caller.
 *
 * @return absolute path (possibly empty if retrieval failed).
 */
WALLPAPER_HOST_API std::wstring GetCurrentWallpaperPath();

/**
 * Attaches the specified top-level window (hwnd) to the Windows desktop by
 * parenting it to the WorkerW window used for the wallpaper. On success, the
 * window behaves like a wallpaper window (behind icons).
 *
 * Implementation notes:
 *  - Uses the Progman message trick to force creation of a WorkerW and then
 *    searches for the WorkerW sibling which contains the SHELLDLL_DefView.
 *
 * @param hwnd Window to attach (should be a top-level, visible window).
 * @return true on success.
 */
WALLPAPER_HOST_API bool AttachWindowToDesktop(HWND hwnd);

/**
 * Attempts to restore the window to a top-level (no parent) state. Call when tearing down.
 *
 * @param hwnd Window previously attached.
 * @return true on success.
 */
WALLPAPER_HOST_API bool DetachWindowFromDesktop(HWND hwnd);

} // namespace desktop
} // namespace wallpaper
