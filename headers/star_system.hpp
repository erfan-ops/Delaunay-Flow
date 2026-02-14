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
    StarSystem(const Settings& settings,
               float left, float right, float bottom, float top);

    StarSystem(const StarSystem&)            = delete;
    StarSystem& operator=(const StarSystem& other) {
        if (this != &other) {
            stars_ = other.stars_;
            left_ = other.left_;
            right_ = other.right_;
            bottom_ = other.bottom_;
            top_ = other.top_;
            // settings_ is a reference, can't reassign it
            // but it will already refer to the same object
        }
        return *this;
    }

    StarSystem(StarSystem&&)            = default;
    StarSystem& operator=(StarSystem&&) = default;

    void reset();
    void update(std::chrono::duration<float> dt,
                float mouseXNDC,
                float mouseYNDC);

    [[nodiscard]] const std::vector<Star>& stars() const noexcept { return stars_; }
    [[nodiscard]] std::vector<Star>&       stars() noexcept       { return stars_; }

    [[nodiscard]] float left() const noexcept   { return left_; }
    [[nodiscard]] float right() const noexcept  { return right_; }
    [[nodiscard]] float bottom() const noexcept { return bottom_; }
    [[nodiscard]] float top() const noexcept    { return top_; }

private:
    std::vector<Star> stars_{};
    float             left_{};
    float             right_{};
    float             bottom_{};
    float             top_{};
    const Settings&   settings_;
};

} // namespace delaunay_flow
