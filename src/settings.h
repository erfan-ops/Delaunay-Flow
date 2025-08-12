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

	struct Edges {
		bool draw;
		float width;
		Color color;
	} edges;

	struct Interaction {
		bool mouseInteraction;
		float distanceFromMouse;
		float speedBasedMouseDistanceMultiplier;
	} interaction;

	struct Barrier {
		bool draw;
		float radius;
		Color color;
		float blur;
	} barrier;
	
	float offsetBounds;

	int MSAA;
};

// Function to load settings from a JSON file
Settings loadSettings(const std::string& filename);
