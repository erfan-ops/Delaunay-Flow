#pragma once

#include <string>
#include <glad/glad.h>

namespace delaunay_flow {

/** Compile and link vertex + fragment GLSL shaders; returns program id or 0 on failure. */
GLuint compileShaders(const char* vertexCode, const char* fragmentCode);

}  // namespace delaunay_flow
