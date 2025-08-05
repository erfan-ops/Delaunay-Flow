#pragma once

#include <string>
#include <vector>
#include <array>

using Color = std::array<float, 4>;

// settings structure
struct Settings {
	float targetFPS;
	bool vsync;
	std::vector<Color> backGroundColors;

	struct Stars {
		bool draw;
		int segments;
		float radius;
		int count;
		float minSpeed;
		float maxSpeed;
		Color color;
	} stars;

	bool drawLines;
	float lineWidth;
	Color linesColor;

	bool moveFromMouse;
	float mouseDistance;
	float speedBasedMouseDistanceMultiplier;
	bool drawMouseBarrier;
	Color mouseBarrierColor;

	float offsetBounds;

	int MSAA;
};

// Function to load settings from a JSON file
Settings loadSettings(const std::string& filename);
