#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>

#include <memory>
#include <vector>

namespace delaunay_flow {

class GLProgram {
public:
    GLProgram(GLuint id);
    ~GLProgram();

    GLProgram(const GLProgram&) = delete;
    GLProgram& operator=(const GLProgram&) = delete;

    GLProgram(GLProgram&& other) noexcept;
    GLProgram& operator=(GLProgram&& other) noexcept;

    GLuint id() const noexcept;

private:
    GLuint id_{};
};

struct GlfwContext {
    GlfwContext();
    
    GlfwContext(const GlfwContext&)            = delete;
    GlfwContext& operator=(const GlfwContext&) = delete;

    GlfwContext(GlfwContext&&) noexcept            = default;
    GlfwContext& operator=(GlfwContext&&) noexcept = default;

    ~GlfwContext() noexcept;
};

struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) const noexcept;
};

using GlfwWindowPtr = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>;

class VertexArray {
public:
    VertexArray();
    ~VertexArray() noexcept;

    VertexArray(const VertexArray&)            = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void bind() const noexcept;
    void unbind() const noexcept;

    [[nodiscard]] GLuint id() const noexcept;

private:
    void reset() noexcept;

    GLuint id_{0U};
};

class ArrayBuffer {
public:
    ArrayBuffer();
    ~ArrayBuffer() noexcept;

    ArrayBuffer(const ArrayBuffer&)            = delete;
    ArrayBuffer& operator=(const ArrayBuffer&) = delete;

    ArrayBuffer(ArrayBuffer&& other) noexcept;
    ArrayBuffer& operator=(ArrayBuffer&& other) noexcept;

    void bind() const noexcept;
    void unbind() const noexcept;

    template <typename VertexT>
    void setData(const std::vector<VertexT>& vertices, GLenum usage) noexcept;

    [[nodiscard]] GLuint id() const noexcept;

private:
    void reset() noexcept;

    GLuint id_{0U};
};

class WinIcon {
public:
    WinIcon() = default;
    explicit WinIcon(HICON icon) noexcept;

    WinIcon(const WinIcon&)            = delete;
    WinIcon& operator=(const WinIcon&) = delete;

    WinIcon(WinIcon&& other) noexcept;
    WinIcon& operator=(WinIcon&& other) noexcept;

    ~WinIcon() noexcept;

    [[nodiscard]] HICON get() const noexcept;
    [[nodiscard]] HICON release() noexcept;
    void reset(HICON newIcon = nullptr) noexcept;

private:
    HICON icon_{nullptr};
};

class WinMenu {
public:
    WinMenu() = default;
    explicit WinMenu(HMENU menu) noexcept;

    WinMenu(const WinMenu&)            = delete;
    WinMenu& operator=(const WinMenu&) = delete;

    WinMenu(WinMenu&& other) noexcept;
    WinMenu& operator=(WinMenu&& other) noexcept;

    ~WinMenu() noexcept;

    [[nodiscard]] HMENU get() const noexcept;
    [[nodiscard]] HMENU release() noexcept;
    void reset(HMENU newMenu = nullptr) noexcept;

private:
    HMENU menu_{nullptr};
};

// Template method implementation must be in header
template <typename VertexT>
void ArrayBuffer::setData(const std::vector<VertexT>& vertices, GLenum usage) noexcept {
    bind();
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexT)),
        vertices.data(),
        usage
    );
}

} // namespace delaunay_flow
