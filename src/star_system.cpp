#include <star_system.hpp>

#include <iostream>


namespace {

[[nodiscard]] static float randomUniform(float start, float end) {
    static thread_local std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(start, end);
    return dist(gen);
}

} // namespace


namespace delaunay_flow {

StarSystem::StarSystem(const Settings& settings,
                       float left, float right, float bottom, float top)
    : left_(left)
    , right_(right)
    , bottom_(bottom)
    , top_(top)
    , settings_(settings)
{
    reset();
}

void StarSystem::reset() {
    stars_.clear();
    stars_.reserve(static_cast<std::size_t>(settings_.stars.count));

    for (int i = 0; i < settings_.stars.count; ++i) {
        const float x     = randomUniform(left_, right_);
        const float y     = randomUniform(bottom_, top_);
        const float speed = randomUniform(settings_.stars.minSpeed,
                                          settings_.stars.maxSpeed);
        const float angle = randomUniform(0.0f, TAU_F);
        stars_.emplace_back(x, y, speed, angle);
    }
}

void StarSystem::update(std::chrono::duration<float> dt,
                        float mouseXNDC,
                        float mouseYNDC)
{
    const float dtSeconds = dt.count();

    for (Star& star : stars_) {
        star.move(
            dtSeconds,
            mouseXNDC,
            mouseYNDC,
            settings_.interaction.distanceFromMouse,
            left_, right_, bottom_, top_
        );
    }
}

} // namespace delaunay_flow
