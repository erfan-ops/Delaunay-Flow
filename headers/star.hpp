#pragma once

#include <types.hpp>

namespace delaunay_flow {

class Star {
public:
    Star(float x, float y, float speed, float angle);

    float getX() const noexcept { return x_; }
    float getY() const noexcept { return y_; }

    void move(float dt, Rect bounds) noexcept;

    static void init(bool moveFromMouse) noexcept;

    static float mouseXNDC;
    static float mouseYNDC;
    static float mouseKeepDistance;

private:
    void moveWithMouse(float dt, Rect bounds) noexcept;
    void normalMove(float dt, Rect bounds) noexcept;

    float orgx_;
    float orgy_;
    float x_;
    float y_;
    float speedx_;
    float speedy_;

    using MoveFunc = void (Star::*)(float, Rect bounds) noexcept;
    static MoveFunc moveFunc_;
};

}  // namespace delaunay_flow
