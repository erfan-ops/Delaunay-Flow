#pragma once

#include <array>

namespace delaunay_flow {

using Color = std::array<float, 4>;

/** 2 * PI */
constexpr float TAU_F = 6.2831853f;

/** Vertex: position (x, y) and color (r, g, b, a). */
struct Vertex {
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;

    Vertex(float x_, float y_) : x(x_), y(y_), r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
    Vertex(float x_, float y_, float r_, float g_, float b_, float a_)
        : x(x_), y(y_), r(r_), g(g_), b(b_), a(a_) {}
    Vertex(float x_, float y_, Color color)
        : x(x_), y(y_), r(color[0]), g(color[1]), b(color[2]), a(color[3]) {}
};

}  // namespace delaunay_flow
