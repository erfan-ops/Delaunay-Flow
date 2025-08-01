#pragma once

#include <string>
#include <glad/glad.h>

namespace shaderUtils {
    // compiles glsl shaders
    GLuint compileShaders(const char* vertexPath, const char* fragmentPath);
}
