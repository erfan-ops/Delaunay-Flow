#include <star.hpp>
#include <cmath>

namespace delaunay_flow {

Star::MoveFunc Star::moveFunc_ = nullptr;

Star::Star(float x, float y, float speed, float angle)
    : orgx_(x), orgy_(y), x_(x), y_(y),
      speedx_(std::cosf(angle) * speed),
      speedy_(std::sinf(angle) * speed) {}

void Star::move(float dt, Rect bounds) noexcept {
    (this->*moveFunc_)(dt, bounds);
}

void Star::normalMove(float dt, Rect bounds) noexcept {
    orgx_ += speedx_ * dt;
    orgy_ += speedy_ * dt;

    float xdis = orgx_ - x_;
    float ydis = orgy_ - y_;

    x_ += xdis * dt;
    y_ += ydis * dt;

    if (orgx_ <= bounds.left) {
        speedx_ = std::abs(speedx_);
        orgx_ -= (orgx_ - bounds.left) * 2;
    } else if (orgx_ >= bounds.right) {
        speedx_ = -std::abs(speedx_);
        orgx_ -= (orgx_ - bounds.right) * 2;
    }

    if (orgy_ <= bounds.bottom) {
        speedy_ = std::abs(speedy_);
        orgy_ -= (orgy_ - bounds.bottom) * 2;
    } else if (orgy_ >= bounds.top) {
        speedy_ = -std::abs(speedy_);
        orgy_ -= (orgy_ - bounds.top) * 2;
    }
}

void Star::moveWithMouse(float dt, Rect bounds) noexcept {
    normalMove(dt, bounds);

    float mouseDistanceX = Star::mouseXNDC - getX();
    float mouseDistanceY = Star::mouseYNDC - getY();

    // --- Slight ellipse deformation ---
    constexpr float ellipseFactor = 1.015f;  // 1.0f = perfect circle

    float scaledX = mouseDistanceX * ellipseFactor;
    float scaledY = mouseDistanceY;

    float mouseDisSqr = scaledX * scaledX + scaledY * scaledY;

    if (mouseDisSqr != 0.0f && mouseDisSqr < Star::mouseKeepDistance * Star::mouseKeepDistance) {

        float ratio = Star::mouseKeepDistance / std::sqrt(mouseDisSqr);

        x_ = mouseDistanceX + x_ - (mouseDistanceX * ratio);
        y_ = mouseDistanceY + y_ - (mouseDistanceY * ratio);
    }
}

void Star::init(bool moveFromMouse) noexcept {
    if (moveFromMouse) {
        moveFunc_ = &Star::moveWithMouse;
    } else {
        moveFunc_ = &Star::normalMove;
    }
}

}  // namespace delaunay_flow
