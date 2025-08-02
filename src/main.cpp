#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#define GLFW_EXPOSE_NATIVE_WIN32

#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <triangle/triangle.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <functional>

#include <fstream>
#include <sstream>
#include <string>

#include "star.h"
#include "desktopUtils.h"
#include "trayUtils.h"
#include "utils.h"

// 2 * PI
constexpr float TAU_F = 6.2831853f;

// generates a random number [start, end)
static float randomUniform(float start, float end) {
	// Create a random device and a random engine
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dist(start, end); // Range [start, end)

	return static_cast<float>(dist(gen)); // Generate the random number
}

// sleeps so that each frame took `stepInterval` seconds to complete
static inline void gameTick(const float& frameTime, const float& stepInterval, float& fractionalTime) {
	if (frameTime < stepInterval) {
		float totalSleepTime = (stepInterval - frameTime) + fractionalTime;
		int sleepMilliseconds = static_cast<int>(totalSleepTime * 1e+3f);
		fractionalTime = (totalSleepTime - sleepMilliseconds * 1e-3f);
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliseconds));
	}
}

// returns a color from a gradient of given colors based on the value of `t` (range: 0-1)
static Color interpolateColors(const std::vector<Color>& colors, float t) {
	if (colors.empty()) return { 0, 0, 0, 1 }; // Default: Black
	if (colors.size() == 1) return colors[0]; // Only 1 color

	// Clamp t to [0, 1]
	t = max(0.0f, min(1.0f, t));

	// Calculate segment size (N colors → N-1 segments)
	float segmentSize = 1.0f / (colors.size() - 1);

	// Find the two closest colors
	int index = static_cast<int>(t / segmentSize);
	index = min(index, static_cast<int>(colors.size()) - 2); // Prevent overflow

	// Compute local interpolation weight (0 ≤ local_t ≤ 1)
	float local_t = (t - index * segmentSize) / segmentSize;

	// Get the two colors to blend
	const auto& c1 = colors[index];
	const auto& c2 = colors[index + 1];

	// Linear interpolation (lerp) for each channel
	float r = std::get<0>(c1) + (std::get<0>(c2) - std::get<0>(c1)) * local_t;
	float g = std::get<1>(c1) + (std::get<1>(c2) - std::get<1>(c1)) * local_t;
	float b = std::get<2>(c1) + (std::get<2>(c2) - std::get<2>(c1)) * local_t;
	float a = std::get<3>(c1) + (std::get<3>(c2) - std::get<3>(c1)) * local_t;

	return { r, g, b, a };
}

// main window
GLFWwindow* window;

// handles tray events
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_TRAYICON) {
		if (lParam == WM_RBUTTONUP) {
			// Create a popup menu
			HMENU menu = CreatePopupMenu();
			AppendMenu(menu, MF_STRING, 1, L"Quit");

			// Get the cursor position
			POINT cursorPos;
			GetCursorPos(&cursorPos);

			// Show the menu
			SetForegroundWindow(hwnd);
			// Example with TPM_NONOTIFY to avoid blocking
			int selection = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, cursorPos.x - 120, cursorPos.y - 22, 0, hwnd, NULL);
			DestroyMenu(menu);

			// Handle the menu selection
			if (selection == 1) {
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
		}
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/*
bool supports_adaptive_sync(GLFWwindow* window) {
    const char* extensions = NULL;
   
    extensions = (const char*)glfwGetProcAddress("wglGetExtensionsStringEXT");
    if (!extensions) {
        extensions = (const char*)glfwGetProcAddress("wglGetExtensionsStringARB");
    }
   
    const char* gl_extensions = (const char*)glGetString(GL_EXTENSIONS);
   
    if ((extensions && strstr(extensions, "_EXT_swap_control_tear")) ||
        (gl_extensions && strstr(gl_extensions, "_EXT_swap_control_tear"))) {
        return true;
    }

    glfwSwapInterval(-1);
   
    typedef int (APIENTRY * PFNWGLGETSWAPINTERVALPROC)(void);
    PFNWGLGETSWAPINTERVALPROC wglGetSwapInterval = 
        (PFNWGLGETSWAPINTERVALPROC)glfwGetProcAddress("wglGetSwapIntervalEXT");
    if (wglGetSwapInterval && wglGetSwapInterval() == -1) {
        return true;
    }

    glfwSwapInterval(-1);
    glfwSwapBuffers(window);
   
    double start = glfwGetTime();
    while (glfwGetTime() - start < 0.1) {}
   
    return false;
}
*/

// Vertex structure (2 values for x and y, 4 values for color)
struct Vertex {
    float x;
    float y;

    float r;
    float g;
    float b;
    float a;

    Vertex(float x, float y) : x(x), y(y), r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
    Vertex(float x, float y, float r, float g, float b, float a) : x(x), y(y), r(r), g(g), b(b), a(a) {}
    Vertex(float x, float y, Color color) : x(x), y(y), r(color[0]), g(color[1]), b(color[2]), a(color[3]) {}
};


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Request OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int Width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int Height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    float aspectRatio = static_cast<float>(Width) / static_cast<float>(Height);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // using multi-sample anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, settings.MSAA);

    window = glfwCreateWindow(Width, Height, "Delaunay Flow", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    HWND hwnd = glfwGetWin32Window(window);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);


    using GameTickFunc = void(*)(const float&, const float&, float&);
    GameTickFunc tickFunc;

    glfwMakeContextCurrent(window);
    
    // only use gameTick when VSync is false
    if (settings.vsync) {
        glfwSwapInterval(1);
        tickFunc = [](const float&, const float&, float&) {};
    } else {
        glfwSwapInterval(0);
        tickFunc = &gameTick;
    }
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    // enabling alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // optional move method depending on the `mouse-interaction` in settings
    if (settings.moveFromMouse) {
		Star::moveFunc = &Star::moveWithMouse;
	}
	else {
		Star::moveFunc = &Star::normalMove;
	}

    // reserve memory for stars
    void* stars_memory = operator new[](settings.stars.count * sizeof(Star));

    Star* stars = static_cast<Star*>(stars_memory);

    // calculating the offset which stars can go beyond the screen boundaries
    const float offsetBounds = settings.offsetBounds;
	const float roffsetBounds = -offsetBounds - 1.0f;
	const float woffsetBounds = offsetBounds + 1.0f;
	const float hoffsetBounds = (offsetBounds + 1.0f) * aspectRatio;

    // initiating triangulate stuff
    triangulateio in, out;
	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));

    // allocating memory for the input
	in.numberofpoints = settings.stars.count;
	in.pointlist = (double*)malloc(sizeof(double) * in.numberofpoints * 2);

    // filling up the input as well as the stars
	for (int i = 0; i < settings.stars.count; ++i) {
		float x = randomUniform(roffsetBounds, woffsetBounds);
		float y = randomUniform(roffsetBounds, hoffsetBounds) * aspectRatio;
		float speed = randomUniform(settings.stars.minSpeed, settings.stars.maxSpeed);
		float angle = randomUniform(0, TAU_F);
		stars[i] = Star(x, y, speed, angle);

		in.pointlist[i * 2] = x;
		in.pointlist[i * 2 + 1] = y;
	}

    // Constrained Delaunay Triangulation
	char* mode = (char*)"zQ";
	triangulate(mode, &in, &out, nullptr);

    // getting number of vertices needed to draw the stars
    // `0` if user doesn't want the stars to render
    // otherwise `number of stars * number of segments`
    const size_t numberOfStarVertices = settings.stars.draw ? settings.stars.count * settings.stars.segments : 0;

    // reserve memory for the vertices
    std::vector<Vertex> vertices;
    vertices.reserve((out.numberoftriangles + numberOfStarVertices) * 3);

    // filling the vertices
    for (int i = 0; i < out.numberoftriangles; i++) {
        int bIdx = i * 3;

        int ia = out.trianglelist[bIdx];
        int ib = out.trianglelist[bIdx + 1];
        int ic = out.trianglelist[bIdx + 2];

        int pa = ia * 2;
        int pb = ib * 2;
        int pc = ic * 2;

        float x1 = static_cast<float>(out.pointlist[pa]);
        float y1 = static_cast<float>(out.pointlist[pa + 1]);

        float x2 = static_cast<float>(out.pointlist[pb]);
        float y2 = static_cast<float>(out.pointlist[pb + 1]);

        float x3 = static_cast<float>(out.pointlist[pc]);
        float y3 = static_cast<float>(out.pointlist[pc + 1]);

        float cy = (y1 + y2 + y3) / 3.0f;
        cy = (cy + 1.0f) * 0.5f;

        Color color = interpolateColors(settings.backGroundColors, cy);

        vertices.emplace_back(x1, y1, color);
        vertices.emplace_back(x2, y2, color);
        vertices.emplace_back(x3, y3, color);
    }

    // custom function that inserts the `stars vertices` into the `vertices` array only if `settings.stars.draw` is `true`
    std::function<void()> insertStarsFunc;

    float *xOffsets{0}, *yOffsets{0}, *xAspectRatioCorrectionValues{0};

    if (settings.stars.draw) {
        xOffsets = new float[settings.stars.segments];
        yOffsets = new float[settings.stars.segments];
        xAspectRatioCorrectionValues = new float[settings.stars.segments];

        // pre-calculate `TAU_F / settings.stars.segments`
        float helperFactot = TAU_F / settings.stars.segments;

        for (int i = 0; i < settings.stars.segments; i++) {
            float angle = i * helperFactot;
            xOffsets[i] = settings.stars.radius*sin(angle);
            yOffsets[i] = settings.stars.radius*cos(angle);
            xAspectRatioCorrectionValues[i] = xOffsets[i] / aspectRatio;
        }

        insertStarsFunc = [&]() {
            for (int i = 0; i < settings.stars.count; i++) {
                Star& star = stars[i];
                float radius = settings.stars.radius;

                float centerX = star.getX();
                float centerY = star.getY();
                
                for (int j = 0; j < settings.stars.segments; j++) {
                    int j1 = (j+1) % settings.stars.segments;

                    float x1 = centerX + xOffsets[j];
                    float y1 = centerY + yOffsets[j];

                    float x2 = centerX + xOffsets[j1];
                    float y2 = centerY + yOffsets[j1];
                    
                    float correctedX1 = centerX + xAspectRatioCorrectionValues[j];
                    float correctedX2 = centerX + xAspectRatioCorrectionValues[j1];
                    
                    vertices.emplace_back(centerX, centerY, settings.stars.color);
                    vertices.emplace_back(correctedX1, y1, settings.stars.color);
                    vertices.emplace_back(correctedX2, y2, settings.stars.color);
                }
            }
        };
    } else {
        insertStarsFunc = []() {};
    }

    insertStarsFunc();

    // initiating VAO and VBO
    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    // VBO (dynamic since positions change)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    // vec2 for the position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // vec4 for the color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    // compiling shaders
    GLuint shaderProgram = shaderUtils::compileShaders(
        "shaders/vertex.glsl",
        "shaders/fragment.glsl"
    );
    if (shaderProgram == 0) {
        std::cerr << "Failed to compile shaders!" << std::endl;
        return -1;
    }

    // variables to store the mouse position
    double mouseX, mouseY;
    double oldMouseX, oldMouseY;

    // interval between frames (useless if vsync is on)
    const float stepInterval = 1.0f / settings.targetFPS;

    float dt{0};
	float fractionalTime{0};

    // app icon
    HICON hIcon = LoadIconFromResource();

	AddTrayIcon(hwnd, hIcon, L"Just a Simple Icon");

    // current wallpaper's path used to set the original wallpaper back when the application is closed
    wchar_t* originalWallpaper = GetCurrentWallpaper();

    // settings the window as the desktop background
    SetAsDesktop(hwnd);

    glfwShowWindow(window);

    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseX = mouseX / Width * 2.0f - 1.0f;
    mouseY = -(mouseY / Height * 2.0f - 1.0f);

    // timestamps to keep track of delta-time
    auto newF = std::chrono::high_resolution_clock::now();
	auto oldF = std::chrono::high_resolution_clock::now();

    // main loop
    while (!glfwWindowShouldClose(window)) {
        // calculating delta-time
        oldF = newF;
		newF = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<float>(newF - oldF).count();

        // getting the mouse position
        oldMouseX = mouseX;
        oldMouseY = mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseX = mouseX / Width * 2.0f - 1.0f;
        mouseY = -(mouseY / Height * 2.0f - 1.0f);

        // calculating how much the area around the mouse should expand depending on its speed
        float mdxm = static_cast<float>(std::abs(mouseX - oldMouseX));
        float mdym = static_cast<float>(std::abs(mouseY - oldMouseY));
        float mdm = std::sqrt(mdxm*mdxm + mdym*mdym);
        // float mdm = mdxm*mdxm + mdym*mdym;
        // float mdm = std::log(mdxm*mdxm + mdym*mdym);
        float scale = 1.0f + (mdm * settings.speedBasedMouseDistanceMultiplier);

        // clearing the screen
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // moving the stars and updating the triangle input
        for (int i = 0; i < settings.stars.count; i++) {
            Star& star = stars[i];
            float mouseDisX = static_cast<float>(mouseX) - star.getX();
            float mouseDisY = static_cast<float>(mouseY) - star.getY();
            star.move(dt, mouseDisX * aspectRatio, mouseDisY, scale, roffsetBounds, woffsetBounds, roffsetBounds, hoffsetBounds);

            in.pointlist[i * 2] = star.getX();
		    in.pointlist[i * 2 + 1] = star.getY();
        }

        // CDT
        triangulate(mode, &in, &out, nullptr);

        // updating the vertices array
        vertices.clear();
        vertices.reserve((out.numberoftriangles + numberOfStarVertices) * 3);

        for (int i = 0; i < out.numberoftriangles; i++) {
            int bIdx = i * 3;

            int ia = out.trianglelist[bIdx];
            int ib = out.trianglelist[bIdx + 1];
            int ic = out.trianglelist[bIdx + 2];

            int pa = ia * 2;
            int pb = ib * 2;
            int pc = ic * 2;

            float x1 = static_cast<float>(out.pointlist[pa]);
            float y1 = static_cast<float>(out.pointlist[pa + 1]);

            float x2 = static_cast<float>(out.pointlist[pb]);
            float y2 = static_cast<float>(out.pointlist[pb + 1]);

            float x3 = static_cast<float>(out.pointlist[pc]);
            float y3 = static_cast<float>(out.pointlist[pc + 1]);

            float cy = (y1 + y2 + y3) / 3.0f;
            cy = (cy + 1.0f) * 0.5f;

            // calculating the color based on the coordinate of the triangle
            Color color = interpolateColors(settings.backGroundColors, cy);

            vertices.emplace_back(x1, y1, color);
            vertices.emplace_back(x2, y2, color);
            vertices.emplace_back(x3, y3, color);
        }

        // insertings the stars vertices
        insertStarsFunc();

        // copying the data into the VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_DYNAMIC_DRAW
        );
        
        glUseProgram(shaderProgram);
        
        // actual drawing
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
        glBindVertexArray(0);

        glUseProgram(0);

        // swapping buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // freeing c allocated stuff
        if (out.pointlist)             free(out.pointlist);
        if (out.pointattributelist)    free(out.pointattributelist);
        if (out.pointmarkerlist)       free(out.pointmarkerlist);
        if (out.trianglelist)          free(out.trianglelist);
        if (out.triangleattributelist) free(out.triangleattributelist);
        if (out.trianglearealist)      free(out.trianglearealist);
        if (out.neighborlist)          free(out.neighborlist);
        if (out.segmentlist)           free(out.segmentlist);
        if (out.segmentmarkerlist)     free(out.segmentmarkerlist);
        if (out.holelist)              free(out.holelist);
        if (out.regionlist)            free(out.regionlist);
        if (out.edgelist)              free(out.edgelist);
        if (out.edgemarkerlist)        free(out.edgemarkerlist);
        if (out.normlist)              free(out.normlist);
        memset(&out, 0, sizeof(out));

        tickFunc(dt, stepInterval, fractionalTime);
    }

    // reset to the original wallpaper
    SetParent(hwnd, nullptr);
	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)originalWallpaper, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

    // remove tray icon from the system tray menu
    RemoveTrayIcon(hwnd);
    DestroyIcon(hIcon);

    // freeing memory
    free(in.pointlist);
    operator delete[](stars_memory);
    delete[] originalWallpaper;
    if (settings.stars.draw) {
        delete[] xOffsets;
        delete[] yOffsets;
        delete[] xAspectRatioCorrectionValues;
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
