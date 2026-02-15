#include <application.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <thread>

namespace {

static void vsyncTick(delaunay_flow::Application::GameTickDuration, delaunay_flow::Application::GameTickDuration, float&) noexcept {}

static void sleepTick(delaunay_flow::Application::GameTickDuration frameTime, delaunay_flow::Application::GameTickDuration stepInterval, float& fractionalTime) noexcept {
    if (frameTime < stepInterval) {
        const auto remaining = stepInterval - frameTime;
        const float total    = remaining.count() + fractionalTime;

        const int sleepMilliseconds = static_cast<int>(total * 1'000.0f);
        fractionalTime              = total - static_cast<float>(sleepMilliseconds) * 0.001f;

        if (sleepMilliseconds > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliseconds));
        }
    }
}

} // namespace

namespace delaunay_flow {

Application::Application()
    : settings_(Settings::Instance()), starSystem_(settings_, Rect())
{
    Settings::Load("settings.json");

    initInterpolation(settings_.backGroundColors);

    initWindow();
    initOpenGL();
    initTrayAndWallpaper();
}

int Application::run() {
    mainLoop();
    return 0;
}

void Application::initWindow() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    iWidth_  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    iHeight_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    width_       = static_cast<float>(iWidth_);
    height_      = static_cast<float>(iHeight_);
    aspectRatio_ = width_ / height_;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    if (settings_.MSAA > 0) {
        glfwWindowHint(GLFW_SAMPLES, settings_.MSAA);
    }

    window_.reset(glfwCreateWindow(iWidth_, iHeight_, "Delaunay Flow", nullptr, nullptr));
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    hwnd_ = glfwGetWin32Window(window_.get());
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, std::bit_cast<LONG_PTR>(this));
    SetWindowLongPtr(hwnd_, GWLP_WNDPROC,  std::bit_cast<LONG_PTR>(&Application::WndProc));

    starSystem_ = StarSystem(
        settings_,
        Rect(
            (-settings_.offsetBounds - 1.0f) * aspectRatio_,
            ( settings_.offsetBounds + 1.0f) * aspectRatio_,
             -settings_.offsetBounds - 1.0f,
              settings_.offsetBounds + 1.0f
        )
    );

    glfwMakeContextCurrent(window_.get());

    glfwGetCursorPos(window_.get(), &mouseX_, &mouseY_);
    mouseXNDC_ = static_cast<float>(mouseX_) / width_ * 2.0f - 1.0f;
    mouseYNDC_ = -(static_cast<float>(mouseY_) / height_ * 2.0f - 1.0f);
}

void Application::initOpenGL() {
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    Star::init(settings_.interaction.mouseInteraction);

    stepInterval_ = std::chrono::duration<float>(1.0f / static_cast<float>(settings_.targetFPS));

    if (settings_.vsync) {
        glfwSwapInterval(1);
        tickFunc_ = &vsyncTick;
    } else {
        glfwSwapInterval(0);
        tickFunc_ = &sleepTick;
    }

    renderer_.emplace(settings_, width_, height_);

    renderer_->rebuildStaticData(settings_, starSystem_, coords_, vertices_);

    wallpaper::tray::StartTrayMenuThread(hwnd_);
}

void Application::initTrayAndWallpaper() {
    trayIcon_.reset(loadIconFromResource());
    wallpaper::tray::RegisterIcon(hwnd_, trayIcon_.get(), L"Just a Simple Icon");

    originalWallpaper_ = wallpaper::desktop::GetCurrentWallpaperPath();

    wallpaper::desktop::AttachWindowToDesktop(hwnd_);
    attachedToDesktop_ = true;

    glfwShowWindow(window_.get());
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* app = std::bit_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (app) {
        app->handleMessage(msg, wParam, lParam);
    }
    return DefWindowProc(app->hwnd_, msg, wParam, lParam);
}

LRESULT Application::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case wallpaper::tray::WM_TRAYICON:
        handleTrayMessage(lParam);
        break;

    case WM_COMMAND:
        handleCommand(wParam);
        break;

    default:
        break;
    }
}

void Application::handleTrayMessage(LPARAM lParam) {
    if (lParam != WM_RBUTTONUP) {
        return;
    }

    if (!trayMenu_.get()) {
        HMENU menu = CreatePopupMenu();
        trayMenu_.reset(menu);

        AppendMenu(trayMenu_.get(), MF_STRING, toUint(MenuId::ToggleAttach), L"Detach");
        AppendMenu(trayMenu_.get(), MF_STRING, toUint(MenuId::Restart),      L"Restart");
        AppendMenu(trayMenu_.get(), MF_STRING, toUint(MenuId::Quit),         L"Quit");
    }

    POINT cursorPos{};
    GetCursorPos(&cursorPos);
    wallpaper::tray::PostShowContextMenuAsync(hwnd_, trayMenu_.get(), cursorPos);
}

void Application::handleCommand(WPARAM wParam) {
    const auto id = static_cast<MenuId>(LOWORD(wParam));
    switch (id) {
    case MenuId::Quit:
        glfwSetWindowShouldClose(window_.get(), GLFW_TRUE);
        break;

    case MenuId::Restart:
        restartRequested_.store(true, std::memory_order_relaxed);
        break;

    case MenuId::ToggleAttach:
        if (attachedToDesktop_) {
            wallpaper::desktop::DetachWindowFromDesktop(hwnd_);
            SystemParametersInfo(
                SPI_SETDESKWALLPAPER,
                0,
                const_cast<wchar_t*>(originalWallpaper_.c_str()),
                SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
            );
            attachedToDesktop_ = false;
            ModifyMenu(trayMenu_.get(), toUint(MenuId::ToggleAttach), MF_STRING,
                       toUint(MenuId::ToggleAttach), L"Attach");
        } else {
            glfwSetWindowPos(window_.get(), 0, 0);
            wallpaper::desktop::AttachWindowToDesktop(hwnd_);
            attachedToDesktop_ = true;
            ModifyMenu(trayMenu_.get(), toUint(MenuId::ToggleAttach), MF_STRING,
                       toUint(MenuId::ToggleAttach), L"Detach");
        }
        break;
    }
}

HICON Application::loadIconFromResource() {
    return std::bit_cast<HICON>(
        LoadImage(
            GetModuleHandle(nullptr),
            MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON,
            0,
            0,
            LR_DEFAULTSIZE
        )
    );
}

void Application::mainLoop() {
    auto previous = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window_.get())) {
        const auto now = std::chrono::high_resolution_clock::now();
        const GameTickDuration dt = now - previous;
        previous                  = now;

        glfwGetCursorPos(window_.get(), &mouseX_, &mouseY_);
        mouseXNDC_ = (static_cast<float>(mouseX_) / width_ * 2.0f - 1.0f) * aspectRatio_;
        mouseYNDC_ = -(static_cast<float>(mouseY_) / height_ * 2.0f - 1.0f);

        if (restartRequested_.exchange(false)) {
            starSystem_.reset();
            renderer_->rebuildStaticData(settings_, starSystem_, coords_, vertices_);
        }

        starSystem_.update(dt, mouseXNDC_, mouseYNDC_);

        for (std::size_t i = 0; i < starSystem_.stars().size(); ++i) {
            const std::size_t idx = 2U * i;
            coords_[idx]          = static_cast<double>(starSystem_.stars()[i].getX());
            coords_[idx + 1U]     = static_cast<double>(starSystem_.stars()[i].getY());
        }

        delaunator::Delaunator delaunator(coords_);

        renderer_->updateFrameGeometry(settings_, starSystem_, coords_, vertices_, delaunator);
        renderer_->uploadVertices(vertices_);
        renderer_->render(static_cast<float>(mouseX_), static_cast<float>(mouseY_));

        glfwSwapBuffers(window_.get());
        glfwPollEvents();

        if (tickFunc_ != nullptr) {
            tickFunc_(dt, stepInterval_, fractionalTime_);
        }
    }

    if (attachedToDesktop_) {
        wallpaper::desktop::DetachWindowFromDesktop(hwnd_);
    }

    SystemParametersInfo(
        SPI_SETDESKWALLPAPER,
        0,
        const_cast<wchar_t*>(originalWallpaper_.c_str()),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    );

    wallpaper::tray::UnregisterIcon(hwnd_);
}

} // namespace delaunay_flow