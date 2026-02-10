#include <windows.h>
#include <immintrin.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <delaunator/delaunator.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <functional>
#include <algorithm>

#include <wallpaper-host/desktop_utils.hpp>
#include <wallpaper-host/tray_utils.hpp>

#include <delaunay_flow/types.hpp>
#include <delaunay_flow/settings.hpp>
#include <delaunay_flow/star.hpp>
#include <delaunay_flow/shader_utils.hpp>
#include <delaunay_flow/color_interpolation.hpp>

#include <resource.h>

using namespace delaunay_flow;

namespace {

[[nodiscard]] static float randomUniform(float start, float end) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(start, end);
    return static_cast<float>(dist(gen));
}

[[nodiscard]] static inline size_t nextHalfedge(size_t e) noexcept {
    return (e % 3 == 2) ? e - 2 : e + 1;
}

}  // namespace

static constinit bool attachment = true;
GLFWwindow* g_window = nullptr;
static HMENU g_trayMenu = nullptr;

struct StarSystem {
    std::vector<Star> stars;

    float left, right, bottom, top;
    const Settings& settings;

    StarSystem(const Settings& s,
               float l, float r, float b, float t)
        : settings(s), left(l), right(r), bottom(b), top(t)
    {
        reset();
    }

    void reset() {
        stars.clear();
        stars.reserve(settings.stars.count);

        for (int i = 0; i < settings.stars.count; ++i) {
            float x = randomUniform(left, right);
            float y = randomUniform(bottom, top);
            float speed = randomUniform(settings.stars.minSpeed,
                                        settings.stars.maxSpeed);
            float angle = randomUniform(0, TAU_F);
            stars.emplace_back(x, y, speed, angle);
        }
    }
};

static std::atomic<bool> g_restartRequested = false;

static LRESULT WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case wallpaper::tray::WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            if (!g_trayMenu) {
                g_trayMenu = CreatePopupMenu();
                AppendMenu(g_trayMenu, MF_STRING, 1003, L"Detach");
                AppendMenu(g_trayMenu, MF_STRING, 1002, L"Restart");
                AppendMenu(g_trayMenu, MF_STRING, 1001, L"Quit");
            }

            POINT cursorPos;
            GetCursorPos(&cursorPos);
            wallpaper::tray::PostShowContextMenuAsync(hwnd, g_trayMenu, cursorPos);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1001: // Quit
            glfwSetWindowShouldClose(g_window, GLFW_TRUE);
            break;
        case 1002: // Restart
            g_restartRequested.store(true, std::memory_order_relaxed);
            break;
        case 1003: // Attach / Detach
            if (attachment) {
                wallpaper::desktop::DetachWindowFromDesktop(hwnd);
                attachment = false;
                ModifyMenu(g_trayMenu, 1003, MF_STRING, 1003, L"Attach");
            }
            else {
                wallpaper::desktop::AttachWindowToDesktop(hwnd);
                attachment = true;
                ModifyMenu(g_trayMenu, 1003, MF_STRING, 1003, L"Detach");
            }
            break;
        }
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HICON LoadIconFromResource() { return static_cast<HICON>(LoadImage( GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE)); }

int main() {
    Settings::Load("settings.json");
    Settings& settings = Settings::Instance();

    initInterpolation(settings.backGroundColors);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int iWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int iHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    const float width = static_cast<float>(iWidth);
    const float height = static_cast<float>(iHeight);
    float aspectRatio = width / height;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    if (settings.MSAA > 0) {
        glfwWindowHint(GLFW_SAMPLES, settings.MSAA);
    }

    g_window = glfwCreateWindow(iWidth, iHeight, "Delaunay Flow", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    HWND hwnd = glfwGetWin32Window(g_window);
    wallpaper::tray::StartTrayMenuThread(hwnd);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));

    using GameTickFunc = void (*)(float, float, float&);
    GameTickFunc tickFunc;

    glfwMakeContextCurrent(g_window);

    if (settings.vsync) {
        glfwSwapInterval(1);
        tickFunc = [](float, float, float&) {};
    } else {
        glfwSwapInterval(0);
        tickFunc = [](float frameTime, float stepInterval, float& fractionalTime) {
            if (frameTime < stepInterval) {
                float totalSleepTime = (stepInterval - frameTime) + fractionalTime;
                int sleepMilliseconds = static_cast<int>(totalSleepTime * 1e+3f);
                fractionalTime = totalSleepTime - sleepMilliseconds * 1e-3f;
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliseconds));
            }
        };
    }

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Star::init(settings.interaction.mouseInteraction);

    const int starsCount = settings.stars.count;

    const float offsetBounds = settings.offsetBounds;
    const float leftBound = (-offsetBounds - 1.0f) * aspectRatio;
    const float rightBound = (offsetBounds + 1.0f) * aspectRatio;
    const float bottomBound = -offsetBounds - 1.0f;
    const float topBound = offsetBounds + 1.0f;

    // std::vector<Star> stars;
    StarSystem starSystem(settings, leftBound, rightBound, bottomBound, topBound);


    std::vector<double> coords;
    coords.resize(static_cast<size_t>(starsCount) << 1);

    for (size_t i = 0; i < starSystem.stars.size(); ++i) {
        size_t i2 = i << 1;
        coords[i2]     = starSystem.stars[i].getX();
        coords[i2 + 1] = starSystem.stars[i].getY();
    }

    delaunator::Delaunator delaunator(coords);

    const size_t numberOfStarVertices =
        settings.stars.draw ? static_cast<size_t>(starsCount) * settings.stars.segments * 3 : 0;
    const size_t numberOfLineVertices =
        settings.edges.draw ? static_cast<size_t>(settings.stars.count) * 18 - 36 : 0;
    const size_t numberOfTriangleVertices = static_cast<size_t>(settings.stars.count) * 6 - 15;

    size_t reserve_count = numberOfTriangleVertices + numberOfStarVertices + numberOfLineVertices;
    std::vector<Vertex> vertices;
    vertices.reserve(reserve_count);

    std::function<void(delaunator::Delaunator&)> insertTrianglesFunc = [&](delaunator::Delaunator& d) {
        for (size_t i = 0; i < d.triangles.size(); i += 3) {
            size_t aIdx = d.triangles[i] << 1;
            size_t bIdx = d.triangles[i + 1] << 1;
            size_t cIdx = d.triangles[i + 2] << 1;

            float x1 = static_cast<float>(d.coords[aIdx]);
            float y1 = static_cast<float>(d.coords[aIdx + 1]);
            float x2 = static_cast<float>(d.coords[bIdx]);
            float y2 = static_cast<float>(d.coords[bIdx + 1]);
            float x3 = static_cast<float>(d.coords[cIdx]);
            float y3 = static_cast<float>(d.coords[cIdx + 1]);

            float cy = (y1 + y2 + y3) / 3.0f;
            cy = (cy + 1.0f) * 0.5f;

            Color color = interpolate(cy);

            vertices.emplace_back(x1, y1, color);
            vertices.emplace_back(x2, y2, color);
            vertices.emplace_back(x3, y3, color);
        }
    };

    std::function<void()> insertStarsFunc;
    std::unique_ptr<float[]> xOffsets;
    std::unique_ptr<float[]> yOffsets;
    std::unique_ptr<float[]> xAspectRatioCorrectionValues;

    if (settings.stars.draw) {
        const size_t segCount = static_cast<size_t>(settings.stars.segments);
        xOffsets = std::make_unique<float[]>(segCount);
        yOffsets = std::make_unique<float[]>(segCount);
        xAspectRatioCorrectionValues = std::make_unique<float[]>(segCount);

        float helperFactor = TAU_F / settings.stars.segments;

        for (int i = 0; i < settings.stars.segments; i++) {
            float angle = i * helperFactor;
            size_t ii = static_cast<size_t>(i);
            xOffsets[ii] = settings.stars.radius * std::sin(angle);
            yOffsets[ii] = settings.stars.radius * std::cos(angle);
            xAspectRatioCorrectionValues[ii] = xOffsets[ii];
        }

        insertStarsFunc = [&]() {
            const int segs = settings.stars.segments;
            for (auto& star : starSystem.stars) {
                float centerX = star.getX();
                float centerY = star.getY();

                for (int j = 0; j < segs; j++) {
                    int j1 = (j + 1) % segs;
                    size_t jj = static_cast<size_t>(j);
                    size_t jj1 = static_cast<size_t>(j1);

                    float y1 = centerY + yOffsets[jj];
                    float y2 = centerY + yOffsets[jj1];
                    float correctedX1 = centerX + xAspectRatioCorrectionValues[jj];
                    float correctedX2 = centerX + xAspectRatioCorrectionValues[jj1];

                    vertices.emplace_back(centerX, centerY, settings.stars.color);
                    vertices.emplace_back(correctedX1, y1, settings.stars.color);
                    vertices.emplace_back(correctedX2, y2, settings.stars.color);
                }
            }
        };
    } else {
        insertStarsFunc = []() {};
    }

    std::function<void(delaunator::Delaunator&)> insertLinesFunc;
    float halfWidth = settings.edges.width * 0.5f;

    if (settings.edges.draw) {
        insertLinesFunc = [&](delaunator::Delaunator& d) {
            for (size_t i = 0; i < d.halfedges.size(); ++i) {
                size_t j = d.halfedges[i];
                if (j != delaunator::INVALID_INDEX && i < j) {
                    size_t ia = d.triangles[i] << 1;
                    size_t ib = d.triangles[nextHalfedge(i)] << 1;

                    float x1 = static_cast<float>(d.coords[ia]);
                    float y1 = static_cast<float>(d.coords[ia + 1]);
                    float x2 = static_cast<float>(d.coords[ib]);
                    float y2 = static_cast<float>(d.coords[ib + 1]);

                    float dx = x2 - x1;
                    float dy = y2 - y1;
                    float lengthSqr = dx * dx + dy * dy;

                    if (lengthSqr != 0.0f) {
                        float length = std::sqrt(lengthSqr);
                        float nx = -dy / length;
                        float ny = dx / length;

                        float rx1 = x1 + nx * halfWidth;
                        float ry1 = y1 + ny * halfWidth;
                        float rx2 = x1 - nx * halfWidth;
                        float ry2 = y1 - ny * halfWidth;
                        float rx3 = x2 - nx * halfWidth;
                        float ry3 = y2 - ny * halfWidth;
                        float rx4 = x2 + nx * halfWidth;
                        float ry4 = y2 + ny * halfWidth;

                        vertices.emplace_back(rx1, ry1, settings.edges.color);
                        vertices.emplace_back(rx2, ry2, settings.edges.color);
                        vertices.emplace_back(rx3, ry3, settings.edges.color);
                        vertices.emplace_back(rx4, ry4, settings.edges.color);
                        vertices.emplace_back(rx3, ry3, settings.edges.color);
                        vertices.emplace_back(rx1, ry1, settings.edges.color);
                    }
                }
            }
        };
    } else {
        insertLinesFunc = [](delaunator::Delaunator&) {};
    }

    insertTrianglesFunc(delaunator);
    insertLinesFunc(delaunator);
    insertStarsFunc();

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         reinterpret_cast<void*>(offsetof(Vertex, r)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    GLuint shaderProgram = compileShaders("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Failed to compile shaders!\n";
        return -1;
    }

    GLint aspectRatioLocation = glGetUniformLocation(shaderProgram, "aspectRatio");
    GLint mousePosLocation = glGetUniformLocation(shaderProgram, "mousePos");
    GLint mouseBarrierRadiusLocation = glGetUniformLocation(shaderProgram, "mouseBarrierRadius");
    GLint displayBoundsLocation = glGetUniformLocation(shaderProgram, "displayBounds");
    GLint mouseBarrierColorLocation = glGetUniformLocation(shaderProgram, "mouseBarrierColor");
    GLint mouseBarrierBlurLocation = glGetUniformLocation(shaderProgram, "mouseBarrierBlur");

    glUseProgram(shaderProgram);
    glUniform1f(aspectRatioLocation, aspectRatio);
    float mouseDistNDC = settings.barrier.radius * height / 2.0f;
    glUniform1f(mouseBarrierRadiusLocation, mouseDistNDC);
    glUniform2f(displayBoundsLocation, width, height);
    glUniform1f(mouseBarrierBlurLocation, settings.barrier.blur);

    if (settings.barrier.draw) {
        glUniform4f(mouseBarrierColorLocation,
                   settings.barrier.color[0], settings.barrier.color[1],
                   settings.barrier.color[2], settings.barrier.color[3]);
    } else {
        glUniform4f(mouseBarrierColorLocation, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    glUseProgram(0);

    double mouseX, mouseY;
    float mouseXNDC, mouseYNDC;
    float oldMouseXNDC, oldMouseYNDC;

    const float stepInterval = 1.0f / settings.targetFPS;
    float dt = 0.0f;
    float fractionalTime = 0.0f;

    HICON hIcon = LoadIconFromResource();
    wallpaper::tray::RegisterIcon(hwnd, hIcon, L"Just a Simple Icon");

    std::wstring originalWallpaper = wallpaper::desktop::GetCurrentWallpaperPath();

    const float distanceFromMouseSqr =
        settings.interaction.distanceFromMouse * settings.interaction.distanceFromMouse;

    wallpaper::desktop::AttachWindowToDesktop(hwnd);
    glfwShowWindow(g_window);

    glfwGetCursorPos(g_window, &mouseX, &mouseY);
    mouseXNDC = static_cast<float>(mouseX) / width * 2.0f - 1.0f;
    mouseYNDC = -(static_cast<float>(mouseY) / height * 2.0f - 1.0f);

    auto newF = std::chrono::high_resolution_clock::now();
    auto oldF = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(g_window)) {
        oldF = newF;
        newF = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float>(newF - oldF).count();

        oldMouseXNDC = mouseXNDC;
        oldMouseYNDC = mouseYNDC;
        glfwGetCursorPos(g_window, &mouseX, &mouseY);
        mouseXNDC = (static_cast<float>(mouseX) / width * 2.0f - 1.0f) * aspectRatio;
        mouseYNDC = -(static_cast<float>(mouseY) / height * 2.0f - 1.0f);

        float mdxm = std::abs(mouseXNDC - oldMouseXNDC);
        float mdym = std::abs(mouseYNDC - oldMouseYNDC);
        float mdm = std::sqrt(mdxm * mdxm + mdym * mdym);
        float scale = 1.0f + (mdm * settings.interaction.speedBasedMouseDistanceMultiplier);
        float mouseBarrierDistSqr = distanceFromMouseSqr * scale * scale;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (g_restartRequested.exchange(false)) {
            starSystem.reset();

            // rebuild coords immediately
            for (size_t i = 0; i < starSystem.stars.size(); ++i) {
                size_t i2 = i << 1;
                coords[i2]     = starSystem.stars[i].getX();
                coords[i2 + 1] = starSystem.stars[i].getY();
            }
        }

        for (size_t i = 0; i < starSystem.stars.size(); i++) {
            Star& star = starSystem.stars[i];
            star.move(dt, mouseXNDC, mouseYNDC, mouseBarrierDistSqr,
                     leftBound, rightBound, bottomBound, topBound);

            size_t i2 = i << 1;
            coords[i2] = static_cast<double>(star.getX());
            coords[i2 + 1] = static_cast<double>(star.getY());
        }

        delaunator::Delaunator delaunator(coords);

        vertices.clear();
        insertTrianglesFunc(delaunator);
        insertLinesFunc(delaunator);
        insertStarsFunc();

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                     vertices.data(),
                     GL_DYNAMIC_DRAW);

        glUseProgram(shaderProgram);
        glUniform2f(mousePosLocation, static_cast<float>(mouseX), static_cast<float>(mouseY));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(g_window);
        glfwPollEvents();

        tickFunc(dt, stepInterval, fractionalTime);
    }

    if (attachment) {
        wallpaper::desktop::DetachWindowFromDesktop(hwnd);
    }
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, static_cast<PVOID>(originalWallpaper.data()),
                         SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

    if (g_trayMenu) DestroyMenu(g_trayMenu);

    wallpaper::tray::UnregisterIcon(hwnd);
    DestroyIcon(hIcon);

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(g_window);
    glfwTerminate();
    return 0;
}
