// renderer.cpp
#include <renderer.hpp>
#include <shader_utils.hpp>
#include <star_system.hpp>

#include <shaders.hpp>

#include <iostream>

namespace delaunay_flow {

Renderer::Renderer(const Settings& settings,
                   const float     screenWidth,
                   const float     screenHeight)
    : program_(compileShaders(vertex_glsl, fragment_glsl))
    , screenWidth_(screenWidth)
    , screenHeight_(screenHeight)
    , aspectRatio_(screenWidth / screenHeight)
{
    if (program_.id() == 0U) {
        throw std::runtime_error("Failed to compile shaders");
    }

    initState(settings, screenWidth, screenHeight);
}

void Renderer::initState(const Settings& settings,
                         const float     screenWidth,
                         const float     screenHeight)
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao_.bind();
    vbo_.bind();

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), nullptr
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        std::bit_cast<void*>(offsetof(Vertex, r))
    );
    glEnableVertexAttribArray(1);

    vao_.unbind();

    aspectRatioLocation_       = glGetUniformLocation(program_.id(), "aspectRatio");
    mousePosLocation_          = glGetUniformLocation(program_.id(), "mousePos");
    mouseBarrierRadiusLocation_ = glGetUniformLocation(program_.id(), "mouseBarrierRadius");
    displayBoundsLocation_     = glGetUniformLocation(program_.id(), "displayBounds");
    mouseBarrierColorLocation_ = glGetUniformLocation(program_.id(), "mouseBarrierColor");
    mouseBarrierBlurLocation_  = glGetUniformLocation(program_.id(), "mouseBarrierBlur");

    glUseProgram(program_.id());
    glUniform1f(aspectRatioLocation_, aspectRatio_);

    const float mouseDistNDC = settings.barrier.radius * screenHeight / 2.0f;
    glUniform1f(mouseBarrierRadiusLocation_, mouseDistNDC);
    glUniform2f(displayBoundsLocation_, screenWidth, screenHeight);
    glUniform1f(mouseBarrierBlurLocation_, settings.barrier.blur);

    if (settings.barrier.draw) {
        glUniform4f(
            mouseBarrierColorLocation_,
            settings.barrier.color[0],
            settings.barrier.color[1],
            settings.barrier.color[2],
            settings.barrier.color[3]
        );
    } else {
        glUniform4f(mouseBarrierColorLocation_, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    glUseProgram(0);

    if (settings.stars.draw) {
        const std::size_t segCount =
            static_cast<std::size_t>(settings.stars.segments);
        xOffsets_.resize(segCount);
        yOffsets_.resize(segCount);
        xAspectRatioCorrectionValues_.resize(segCount);

        const float helperFactor =
            TAU_F / static_cast<float>(settings.stars.segments);

        for (int i = 0; i < settings.stars.segments; ++i) {
            const float angle = static_cast<float>(i) * helperFactor;
            const std::size_t idx = static_cast<std::size_t>(i);

            const float xOffset = settings.stars.radius * std::sin(angle);
            const float yOffset = settings.stars.radius * std::cos(angle);

            xOffsets_[idx]                    = xOffset;
            yOffsets_[idx]                    = yOffset;
            xAspectRatioCorrectionValues_[idx] = xOffset;
        }
    }

    halfEdgeWidth_ = settings.edges.width * 0.5f;

    const size_t numberOfStarVertices =
        settings.stars.draw ? static_cast<size_t>(settings.stars.count) * settings.stars.segments * 3 : 0;
    const size_t numberOfLineVertices =
        settings.edges.draw ? static_cast<size_t>(settings.stars.count) * 18 - 36 : 0;
    const size_t numberOfTriangleVertices = static_cast<size_t>(settings.stars.count) * 6 - 15;

    verticesCount = numberOfTriangleVertices + numberOfStarVertices + numberOfLineVertices;
}

void Renderer::rebuildStaticData(const Settings&    settings,
                                 const StarSystem&  starSystem,
                                 std::vector<double>& coords,
                                 std::vector<Vertex>& vertices)
{
    const int   starsCount             = settings.stars.count;
    const bool  drawStars              = settings.stars.draw;
    const bool  drawEdges              = settings.edges.draw;
    const auto  starCountULL           = static_cast<std::size_t>(starsCount);

    const std::size_t starCoordCount   = 2U * starCountULL;
    coords.resize(starCoordCount);

    for (std::size_t i = 0; i < starSystem.stars().size(); ++i) {
        const std::size_t idx = 2U * i;
        coords[idx]           = starSystem.stars()[i].getX();
        coords[idx + 1U]      = starSystem.stars()[i].getY();
    }

    const std::size_t numberOfStarVertices =
        drawStars ? starCountULL * static_cast<std::size_t>(settings.stars.segments) * 3U
                  : 0U;

    const std::size_t numberOfLineVertices = drawEdges ? starCountULL * 18U - 36U : 0U;

    const std::size_t numberOfTriangleVertices = starCountULL * 6U - 15U;

    const std::size_t reserveCount =
        numberOfTriangleVertices
        + numberOfStarVertices
        + numberOfLineVertices;

    vertices.clear();
    vertices.reserve(reserveCount);
}

void Renderer::updateFrameGeometry(const Settings&         settings,
                                   const StarSystem&       starSystem,
                                   const std::vector<double>& coords,
                                   std::vector<Vertex>&    vertices,
                                   delaunator::Delaunator& delaunator)
{
    vertices.clear();
    insertTriangles(delaunator, vertices);
    insertLines(settings, delaunator, vertices);
    insertStars(settings, starSystem, vertices);
}

void Renderer::uploadVertices(const std::vector<Vertex>& vertices) noexcept {
    vbo_.setData(vertices, GL_DYNAMIC_DRAW);
}

void Renderer::render(const float mouseX, const float mouseY) const noexcept {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_.id());
    glUniform2f(mousePosLocation_, mouseX, mouseY);

    vao_.bind();

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verticesCount)); // verticesCount supplied by caller or cached
    vao_.unbind();

    glUseProgram(0);
}

void Renderer::insertTriangles(delaunator::Delaunator& d,
                               std::vector<Vertex>&    vertices) const
{
    for (std::size_t i = 0; i < d.triangles.size(); i += 3U) {
        const std::size_t aIdx = 2U * d.triangles[i];
        const std::size_t bIdx = 2U * d.triangles[i + 1U];
        const std::size_t cIdx = 2U * d.triangles[i + 2U];

        const float x1 = static_cast<float>(d.coords[aIdx]);
        const float y1 = static_cast<float>(d.coords[aIdx + 1U]);
        const float x2 = static_cast<float>(d.coords[bIdx]);
        const float y2 = static_cast<float>(d.coords[bIdx + 1U]);
        const float x3 = static_cast<float>(d.coords[cIdx]);
        const float y3 = static_cast<float>(d.coords[cIdx + 1U]);

        float cy = (y1 + y2 + y3) / 3.0f;
        cy       = (cy + 1.0f) * 0.5f;

        const Color color = interpolate(cy);

        vertices.emplace_back(x1, y1, color);
        vertices.emplace_back(x2, y2, color);
        vertices.emplace_back(x3, y3, color);
    }
}

void Renderer::insertStars(const Settings&   settings,
                           const StarSystem& starSystem,
                           std::vector<Vertex>& vertices) const
{
    if (!settings.stars.draw) {
        return;
    }

    const int segs = settings.stars.segments;
    for (const Star& star : starSystem.stars()) {
        const float centerX = star.getX();
        const float centerY = star.getY();

        for (int j = 0; j < segs; ++j) {
            const int j1         = (j + 1) % segs;
            const std::size_t jj  = static_cast<std::size_t>(j);
            const std::size_t jj1 = static_cast<std::size_t>(j1);

            const float y1 = centerY + yOffsets_[jj];
            const float y2 = centerY + yOffsets_[jj1];
            const float correctedX1 = centerX + xAspectRatioCorrectionValues_[jj];
            const float correctedX2 = centerX + xAspectRatioCorrectionValues_[jj1];

            vertices.emplace_back(centerX,     centerY, settings.stars.color);
            vertices.emplace_back(correctedX1, y1,      settings.stars.color);
            vertices.emplace_back(correctedX2, y2,      settings.stars.color);
        }
    }
}

void Renderer::insertLines(const Settings&   settings,
                           delaunator::Delaunator& d,
                           std::vector<Vertex>&    vertices) const
{
    if (!settings.edges.draw) {
        return;
    }

    for (std::size_t i = 0; i < d.halfedges.size(); ++i) {
        const std::size_t j = d.halfedges[i];
        if (j != delaunator::INVALID_INDEX && i < j) {
            const std::size_t ia = 2U * d.triangles[i];
            const std::size_t ib = 2U * d.triangles[nextHalfedge(i)];

            const float x1 = static_cast<float>(d.coords[ia]);
            const float y1 = static_cast<float>(d.coords[ia + 1U]);
            const float x2 = static_cast<float>(d.coords[ib]);
            const float y2 = static_cast<float>(d.coords[ib + 1U]);

            const float dx        = x2 - x1;
            const float dy        = y2 - y1;
            const float lengthSqr = dx * dx + dy * dy;

            if (lengthSqr != 0.0f) {
                const float length = std::sqrt(lengthSqr);
                const float nx     = -dy / length;
                const float ny     = dx / length;

                const float rx1 = x1 + nx * halfEdgeWidth_;
                const float ry1 = y1 + ny * halfEdgeWidth_;
                const float rx2 = x1 - nx * halfEdgeWidth_;
                const float ry2 = y1 - ny * halfEdgeWidth_;
                const float rx3 = x2 - nx * halfEdgeWidth_;
                const float ry3 = y2 - ny * halfEdgeWidth_;
                const float rx4 = x2 + nx * halfEdgeWidth_;
                const float ry4 = y2 + ny * halfEdgeWidth_;

                vertices.emplace_back(rx1, ry1, settings.edges.color);
                vertices.emplace_back(rx2, ry2, settings.edges.color);
                vertices.emplace_back(rx3, ry3, settings.edges.color);
                vertices.emplace_back(rx4, ry4, settings.edges.color);
                vertices.emplace_back(rx3, ry3, settings.edges.color);
                vertices.emplace_back(rx1, ry1, settings.edges.color);
            }
        }
    }
}

std::size_t Renderer::nextHalfedge(const std::size_t e) noexcept {
    return (e % 3U == 2U) ? (e - 2U) : (e + 1U);
}

} // namespace delaunay_flow
