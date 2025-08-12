#pragma once

#include <Windows.h>
#include <array>

#include "settings.h"

const Settings settings = loadSettings("settings.json");

class Star {
private:
    float orgx;
    float orgy;
    float x;
	float y;
	float speedx;
	float speedy;

	void moveWithMouse(const float dt, const float mouseXNDC, const float mouseYNDC, const float scale, const float leftBound, const float rightBound, const float bottomBound, const float topBound) noexcept;
	void normalMove(const float dt, const float mouseXNDC, const float mouseYNDC, const float scale, const float leftBound, const float rightBound, const float bottomBound, const float topBound) noexcept;
	static void (Star::* moveFunc)(const float, const float, const float, const float, const float, const float, const float, const float) noexcept;

public:
	Star(float x, float y, float speed, float angle);

	// Getters
	float getX() const noexcept { return this->x; }
	float getY() const noexcept { return this->y; }

	void move(const float dt, const float mouseXNDC, const float mouseYNDC, const float scale, const float leftBound, const float rightBound, const float bottomBound, const float topBound) noexcept;

	static void init(const bool moveFromMouse) noexcept;
};
