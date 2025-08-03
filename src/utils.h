#pragma once

#include <string>
#include <glad/glad.h>

namespace shaderUtils {
    // compiles glsl shaders
    GLuint compileShaders(const std::string& vertexPath, const std::string& fragmentPath);
}
