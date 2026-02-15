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

StarSystem::StarSystem(const Settings& settings, Rect bounds)
    : bounds_(bounds)
    , settings_(settings)
{
    reset();
}

void StarSystem::reset() {
    stars_.clear();
    stars_.reserve(static_cast<std::size_t>(settings_.stars.count));

    for (int i = 0; i < settings_.stars.count; ++i) {
        const float x     = randomUniform(bounds_.left, bounds_.right);
        const float y     = randomUniform(bounds_.bottom, bounds_.top);

        const float speed = randomUniform(settings_.stars.minSpeed, settings_.stars.maxSpeed);
        const float angle = randomUniform(0.0f, TAU_F);
        
        stars_.emplace_back(x, y, speed, angle);
    }
}

void StarSystem::update(std::chrono::duration<float> dt, float mouseXNDC, float mouseYNDC) {
    const float dtSeconds = dt.count();

    Star::mouseXNDC = mouseXNDC;
    Star::mouseYNDC = mouseYNDC;
    Star::mouseKeepDistance = settings_.interaction.distanceFromMouse;

    for (Star& star : stars_) {
        star.move(dtSeconds, bounds_);
    }
}

} // namespace delaunay_flow
