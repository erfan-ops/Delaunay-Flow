#pragma once

#include <random>
#include <vector>
#include <chrono>

#include <types.hpp>
#include <settings.hpp>
#include <star.hpp>

namespace delaunay_flow {

class StarSystem {
public:
    StarSystem(const Settings& settings, Rect bounds);

    StarSystem(const StarSystem&) = delete;
    StarSystem& operator=(const StarSystem& other) {
        if (this != &other) {
            stars_ = other.stars_;
            bounds_ = other.bounds_;
        }
        return *this;
    }

    StarSystem(StarSystem&&)            = default;
    StarSystem& operator=(StarSystem&&) = default;

    void reset();
    void update(std::chrono::duration<float> dt, float mouseXNDC, float mouseYNDC);

    [[nodiscard]] const std::vector<Star>& stars() const noexcept { return stars_; }
    [[nodiscard]] std::vector<Star>&       stars() noexcept       { return stars_; }

    [[nodiscard]] float left() const noexcept   { return bounds_.left; }
    [[nodiscard]] float right() const noexcept  { return bounds_.right; }
    [[nodiscard]] float bottom() const noexcept { return bounds_.bottom; }
    [[nodiscard]] float top() const noexcept    { return bounds_.top; }

private:
    std::vector<Star> stars_{};
    Rect              bounds_;
    const Settings&   settings_;
};

} // namespace delaunay_flow
