#include "raii.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <stdexcept>
#include <utility>

namespace delaunay_flow {

// GLProgram implementation
GLProgram::GLProgram(GLuint id) : id_(id) {}

GLProgram::~GLProgram() { 
    if (id_) glDeleteProgram(id_); 
}

GLProgram::GLProgram(GLProgram&& other) noexcept 
    : id_(std::exchange(other.id_, 0)) {}

GLProgram& GLProgram::operator=(GLProgram&& other) noexcept {
    if (this != &other) {
        glDeleteProgram(id_);
        id_ = std::exchange(other.id_, 0);
    }
    return *this;
}

GLuint GLProgram::id() const noexcept { 
    return id_; 
}

// GlfwContext implementation
GlfwContext::GlfwContext() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
}

GlfwContext::~GlfwContext() noexcept {
    glfwTerminate();
}

// GlfwWindowDeleter implementation
void GlfwWindowDeleter::operator()(GLFWwindow* window) const noexcept {
    if (window != nullptr) {
        glfwDestroyWindow(window);
    }
}

// Window implementation
Window::Window(int msaa) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    if (msaa > 0) {
        glfwWindowHint(GLFW_SAMPLES, msaa);
    }

    widthPx_  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    heightPx_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    window_.reset(glfwCreateWindow(widthPx_, heightPx_, "Delaunay Flow", nullptr, nullptr));
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_.get());

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

GLFWwindow* Window::get() const noexcept {
    return window_.get();
}

HWND Window::hwnd() const noexcept {
    return glfwGetWin32Window(window_.get());
}

float Window::width() const noexcept {
    return static_cast<float>(widthPx_);
}

float Window::height() const noexcept {
    return static_cast<float>(heightPx_);
}

int Window::widthPx() const noexcept {
    return widthPx_;
}

int Window::heightPx() const noexcept {
    return heightPx_;
}

// VertexArray implementation
VertexArray::VertexArray() {
    glGenVertexArrays(1, &id_);
    if (id_ == 0U) {
        throw std::runtime_error("Failed to create VAO");
    }
}

VertexArray::~VertexArray() noexcept {
    reset();
}

VertexArray::VertexArray(VertexArray&& other) noexcept 
    : id_(other.id_) {
    other.id_ = 0U;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this != &other) {
        reset();
        id_       = other.id_;
        other.id_ = 0U;
    }
    return *this;
}

void VertexArray::bind() const noexcept { 
    glBindVertexArray(id_); 
}

void VertexArray::unbind() const noexcept { 
    glBindVertexArray(0U); 
}

GLuint VertexArray::id() const noexcept { 
    return id_; 
}

void VertexArray::reset() noexcept {
    if (id_ != 0U) {
        glDeleteVertexArrays(1, &id_);
        id_ = 0U;
    }
}

// ArrayBuffer implementation
ArrayBuffer::ArrayBuffer() {
    glGenBuffers(1, &id_);
    if (id_ == 0U) {
        throw std::runtime_error("Failed to create VBO");
    }
}

ArrayBuffer::~ArrayBuffer() noexcept {
    reset();
}

ArrayBuffer::ArrayBuffer(ArrayBuffer&& other) noexcept 
    : id_(other.id_) {
    other.id_ = 0U;
}

ArrayBuffer& ArrayBuffer::operator=(ArrayBuffer&& other) noexcept {
    if (this != &other) {
        reset();
        id_       = other.id_;
        other.id_ = 0U;
    }
    return *this;
}

void ArrayBuffer::bind() const noexcept { 
    glBindBuffer(GL_ARRAY_BUFFER, id_); 
}

void ArrayBuffer::unbind() const noexcept { 
    glBindBuffer(GL_ARRAY_BUFFER, 0U); 
}

GLuint ArrayBuffer::id() const noexcept { 
    return id_; 
}

void ArrayBuffer::reset() noexcept {
    if (id_ != 0U) {
        glDeleteBuffers(1, &id_);
        id_ = 0U;
    }
}

// WinIcon implementation
WinIcon::WinIcon(HICON icon) noexcept : icon_(icon) {}

WinIcon::WinIcon(WinIcon&& other) noexcept : icon_(other.icon_) {
    other.icon_ = nullptr;
}

WinIcon& WinIcon::operator=(WinIcon&& other) noexcept {
    if (this != &other) {
        reset();
        icon_     = other.icon_;
        other.icon_ = nullptr;
    }
    return *this;
}

WinIcon::~WinIcon() noexcept {
    reset();
}

HICON WinIcon::get() const noexcept { 
    return icon_; 
}

HICON WinIcon::release() noexcept {
    HICON tmp = icon_;
    icon_     = nullptr;
    return tmp;
}

void WinIcon::reset(HICON newIcon) noexcept {
    if (icon_ != nullptr) {
        DestroyIcon(icon_);
    }
    icon_ = newIcon;
}

// WinMenu implementation
WinMenu::WinMenu(HMENU menu) noexcept : menu_(menu) {}

WinMenu::WinMenu(WinMenu&& other) noexcept : menu_(other.menu_) {
    other.menu_ = nullptr;
}

WinMenu& WinMenu::operator=(WinMenu&& other) noexcept {
    if (this != &other) {
        reset();
        menu_       = other.menu_;
        other.menu_ = nullptr;
    }
    return *this;
}

WinMenu::~WinMenu() noexcept {
    reset();
}

HMENU WinMenu::get() const noexcept { 
    return menu_; 
}

HMENU WinMenu::release() noexcept {
    HMENU tmp = menu_;
    menu_     = nullptr;
    return tmp;
}

void WinMenu::reset(HMENU newMenu) noexcept {
    if (menu_ != nullptr) {
        DestroyMenu(menu_);
    }
    menu_ = newMenu;
}

} // namespace delaunay_flow
