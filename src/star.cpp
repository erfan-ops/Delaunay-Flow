#include <delaunay_flow/star.hpp>
#include <cmath>

namespace delaunay_flow {

Star::MoveFunc Star::moveFunc_ = nullptr;

Star::Star(float x, float y, float speed, float angle)
    : orgx_(x), orgy_(y), x_(x), y_(y),
      speedx_(std::cosf(angle) * speed),
      speedy_(std::sinf(angle) * speed) {}

void Star::move(float dt, float mouseXNDC, float mouseYNDC, float mouseDistanceSqr,
                float leftBound, float rightBound, float bottomBound, float topBound) noexcept {
    (this->*moveFunc_)(dt, mouseXNDC, mouseYNDC, mouseDistanceSqr,
                       leftBound, rightBound, bottomBound, topBound);
}

void Star::normalMove(float dt, float mouseXNDC, float mouseYNDC, float mouseDistanceSqr,
                      float leftBound, float rightBound, float bottomBound, float topBound) noexcept {
    orgx_ += speedx_ * dt;
    orgy_ += speedy_ * dt;

    float xdis = orgx_ - x_;
    float ydis = orgy_ - y_;

    x_ += xdis * dt;
    y_ += ydis * dt;

    if (orgx_ <= leftBound) {
        speedx_ = std::abs(speedx_);
    } else if (orgx_ >= rightBound) {
        speedx_ = -std::abs(speedx_);
    }

    if (orgy_ <= bottomBound) {
        speedy_ = std::abs(speedy_);
    } else if (orgy_ >= topBound) {
        speedy_ = -std::abs(speedy_);
    }
}

void Star::moveWithMouse(float dt, float mouseXNDC, float mouseYNDC, float mouseDistanceSqr,
                         float leftBound, float rightBound, float bottomBound, float topBound) noexcept {
    normalMove(dt, 0.0f, 0.0f, 0.0f, leftBound, rightBound, bottomBound, topBound);

    float mouseDistanceX = mouseXNDC - getX();
    float mouseDistanceY = mouseYNDC - getY();

    float mouseDisSqr = mouseDistanceX * mouseDistanceX + mouseDistanceY * mouseDistanceY;
    if (mouseDisSqr != 0.0f && mouseDisSqr < mouseDistanceSqr) {
        float ratio = std::sqrt(mouseDistanceSqr / mouseDisSqr);
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
