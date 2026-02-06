#ifndef DELAUNAY_FLOW_STAR_HPP
#define DELAUNAY_FLOW_STAR_HPP

namespace delaunay_flow {

class Star {
public:
    Star(float x, float y, float speed, float angle);

    float getX() const noexcept { return x_; }
    float getY() const noexcept { return y_; }

    void move(float dt, float mouseXNDC, float mouseYNDC, float mouseDistanceSqr,
              float leftBound, float rightBound, float bottomBound, float topBound) noexcept;

    static void init(bool moveFromMouse) noexcept;

private:
    void moveWithMouse(float dt, float mouseXNDC, float mouseYNDC, float mouseDistanceSqr,
                       float leftBound, float rightBound, float bottomBound, float topBound) noexcept;
    void normalMove(float dt, float mouseXNDC, float mouseYNDC, float mouseDistanceSqr,
                    float leftBound, float rightBound, float bottomBound, float topBound) noexcept;

    float orgx_;
    float orgy_;
    float x_;
    float y_;
    float speedx_;
    float speedy_;

    using MoveFunc = void (Star::*)(float, float, float, float, float, float, float, float) noexcept;
    static MoveFunc moveFunc_;
};

}  // namespace delaunay_flow

#endif  // DELAUNAY_FLOW_STAR_HPP
