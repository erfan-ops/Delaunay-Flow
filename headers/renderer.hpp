#pragma once

#include <vector>

#include <glad/glad.h>

#include <types.hpp>
#include <settings.hpp>
#include <color_interpolation.hpp>
#include <raii.hpp>
#include <star_system.hpp>

#include <delaunator/delaunator.hpp>

namespace delaunay_flow {

class Renderer {
public:
    Renderer(const Settings& settings,
             float screenWidth,
             float screenHeight);

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&&)            = default;
    Renderer& operator=(Renderer&&) = default;

    void rebuildStaticData(const Settings&              settings,
                           const StarSystem&            starSystem,
                           std::vector<double>&         coords,
                           std::vector<Vertex>&         vertices);

    void updateFrameGeometry(const Settings&    settings,
                             const StarSystem&  starSystem,
                             const std::vector<double>& coords,
                             std::vector<Vertex>&      vertices,
                             delaunator::Delaunator&   delaunator);

    void uploadVertices(const std::vector<Vertex>& vertices) noexcept;

    void render(float mouseX, float mouseY) const noexcept;

private:
    void initState(const Settings& settings,
                   float screenWidth,
                   float screenHeight);

    void insertTriangles(delaunator::Delaunator& d,
                         std::vector<Vertex>&    vertices) const;

    void insertStars(const Settings&   settings,
                     const StarSystem& starSystem,
                     std::vector<Vertex>& vertices) const;

    void insertLines(const Settings&   settings,
                     delaunator::Delaunator& d,
                     std::vector<Vertex>&    vertices) const;

    [[nodiscard]] static std::size_t nextHalfedge(std::size_t e) noexcept;

private:
    VertexArray vao_{};
    ArrayBuffer vbo_{};
    GLProgram  program_;

    GLint aspectRatioLocation_{-1};
    GLint mousePosLocation_{-1};
    GLint mouseBarrierRadiusLocation_{-1};
    GLint displayBoundsLocation_{-1};
    GLint mouseBarrierColorLocation_{-1};
    GLint mouseBarrierBlurLocation_{-1};

    float screenWidth_{};
    float screenHeight_{};
    float aspectRatio_{};

    // Cached precomputed star offsets when stars are drawn
    std::vector<float> xOffsets_;
    std::vector<float> yOffsets_;
    std::vector<float> xAspectRatioCorrectionValues_;

    float halfEdgeWidth_{};

    size_t verticesCount;
};

} // namespace delaunay_flow