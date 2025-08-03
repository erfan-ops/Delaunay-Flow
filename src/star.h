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
public:
	Star(float x, float y, float speed, float angle);

	// Getters
	float getX() {return this->x;}
	float getY() {return this->y;}

	void move(const float dt, const float mouseDistanceX, const float mouseDistanceY, const float scale, const float leftBound, const float rightBound, const float topBound, const float bottomBound) noexcept;
	void moveWithMouse(const float dt, const float mouseDistanceX, const float mouseDistanceY, const float scale, const float leftBound, const float rightBound, const float topBound, const float bottomBound) noexcept;
	void normalMove(const float dt, const float mouseDistanceX, const float mouseDistanceY, const float scale, const float leftBound, const float rightBound, const float topBound, const float bottomBound) noexcept;
	static void (Star::* moveFunc)(const float, const float, const float, const float, const float, const float, const float, const float) noexcept;

};
