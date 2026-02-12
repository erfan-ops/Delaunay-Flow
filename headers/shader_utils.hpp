#ifndef DELAUNAY_FLOW_SHADER_UTILS_HPP
#define DELAUNAY_FLOW_SHADER_UTILS_HPP

#include <string>
#include <glad/glad.h>

namespace delaunay_flow {

/** Compile and link vertex + fragment GLSL shaders; returns program id or 0 on failure. */
GLuint compileShaders(const std::string& vertexPath, const std::string& fragmentPath);

}  // namespace delaunay_flow

#endif  // DELAUNAY_FLOW_SHADER_UTILS_HPP
