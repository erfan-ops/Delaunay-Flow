#include "star.h"
#include <cmath>


void (Star::* Star::moveFunc)(const float, const float, const float, const float, const float, const float, const float, const float) noexcept = nullptr;

Star::Star(float x, float y, float speed, float angle)
    : x(x), y(y), orgx(x), orgy(y), speedx(std::cosf(angle)*speed), speedy(std::sinf(angle)*speed) {}

void Star::normalMove(
    const float dt,
    const float mouseXNDC,
    const float mouseYNDC,
    const float mouseDistanceSqr,
    const float leftBound,
    const float rightBound,
    const float bottomBound,
    const float topBound
) noexcept
{
    this->orgx += this->speedx * dt;
    this->orgy += this->speedy * dt;

    float xdis = this->orgx - this->x;
    float ydis = this->orgy - this->y;

    this->x += xdis * dt;
    this->y += ydis * dt;

    if (this->orgx <= leftBound) {
        this->speedx = std::abs(this->speedx);
    }
    else if (this->orgx >= rightBound) {
        this->speedx = -std::abs(this->speedx);
    }

    if (this->orgy <= bottomBound) {
        this->speedy = std::abs(this->speedy);
    }
    else if (this->orgy >= topBound) {
        this->speedy = -std::abs(this->speedy);
    }
}

void Star::moveWithMouse(
    const float dt,
    const float mouseXNDC,
    const float mouseYNDC,
    const float mouseDistanceSqr,
    const float leftBound,
    const float rightBound,
    const float bottomBound,
    const float topBound
) noexcept
{
    this->normalMove(dt, 0.0f, 0.0f, 0.0f, leftBound, rightBound, bottomBound, topBound);

    float mouseDistanceX = mouseXNDC - this->getX();
    float mouseDistanceY = mouseYNDC - this->getY();

    float mouseDisSqr = mouseDistanceX * mouseDistanceX + mouseDistanceY * mouseDistanceY;
    if (mouseDisSqr && mouseDisSqr < mouseDistanceSqr) {
        float ratio = std::sqrt(mouseDistanceSqr / mouseDisSqr);
        this->x = mouseDistanceX + this->x - (mouseDistanceX * ratio);
        this->y = mouseDistanceY + this->y - (mouseDistanceY * ratio);
    }
}

void Star::init(const bool moveFromMouse) noexcept {
    if (moveFromMouse) {
		Star::moveFunc = &Star::moveWithMouse;
	}
	else {
		Star::moveFunc = &Star::normalMove;
	}
}
