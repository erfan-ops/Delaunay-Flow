#pragma once

#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <delaunator/delaunator.hpp>

#include <settings.hpp>
#include <types.hpp>
#include <star_system.hpp>
#include <renderer.hpp>
#include <raii.hpp>
#include <wallpaper-host/desktop_utils.hpp>
#include <wallpaper-host/tray_utils.hpp>
#include <resource.h>

namespace delaunay_flow {

enum class MenuId : UINT {
    Quit         = 1001U,
    Restart      = 1002U,
    ToggleAttach = 1003U
};

constexpr UINT toUint(MenuId id) noexcept {
    return static_cast<UINT>(id);
}

class Application {
public:
    Application(Settings& s);
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    int run();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    using GameTickDuration = std::chrono::duration<float>;

private:
    using GameTickFunc     = void (*)(GameTickDuration, GameTickDuration, float&) noexcept;

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void    handleTrayMessage(HWND hwnd, LPARAM lParam);
    void    handleCommand(HWND hwnd, WPARAM wParam);

    void initWindow();
    void initOpenGL();
    void initTrayAndWallpaper();
    void mainLoop();

    [[nodiscard]] static HICON loadIconFromResource();

private:
    GlfwContext   glfwContext_{};
    GlfwWindowPtr window_{};
    HWND          hwnd_{nullptr};

    Settings&  settings_;
    StarSystem starSystem_;

    std::vector<double> coords_;
    std::vector<Vertex> vertices_;
    std::optional<Renderer> renderer_;

    std::wstring originalWallpaper_;
    WinMenu      trayMenu_{};
    WinIcon      trayIcon_{};
    bool         attachedToDesktop_{true};

    std::atomic<bool> restartRequested_{false};

    GameTickFunc      tickFunc_{nullptr};
    GameTickDuration  stepInterval_{};
    float             fractionalTime_{0.0f};

    double mouseX_{};
    double mouseY_{};
    float  mouseXNDC_{};
    float  mouseYNDC_{};

    int    iWidth_{};
    int    iHeight_{};
    float  width_{};
    float  height_{};
    float  aspectRatio_{};
};

} // namespace delaunay_flow
