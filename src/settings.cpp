#include "settings.h"
#include <fstream>
#include <nlohmann/json.hpp>


Settings loadSettings(const std::string& filename) {
	std::ifstream file(filename);
	nlohmann::json j;
	file >> j;

	Settings settings;

	settings.targetFPS = j["fps"];
	settings.vsync = j["vsync"];

	settings.backGroundColors = j["background-colors"].get<std::vector<Color>>();

	settings.stars.draw = j["stars"]["draw"];
	settings.stars.segments = j["stars"]["segments"];
	settings.stars.radius = j["stars"]["radius"];
	settings.stars.count = j["stars"]["count"];
	settings.stars.minSpeed = j["stars"]["min-speed"];
	settings.stars.maxSpeed = j["stars"]["max-speed"];
	settings.stars.color = j["stars"]["color"].get<Color>();

	settings.edges.draw = j["edges"]["draw"];
	settings.edges.width =j["edges"]["width"];
	settings.edges.color = j["edges"]["color"].get<Color>();

	settings.interaction.mouseInteraction = j["interaction"]["mouse-interaction"];
	settings.interaction.distanceFromMouse = j["interaction"]["distance-from-mouse"];
	settings.interaction.speedBasedMouseDistanceMultiplier = j["interaction"]["speed-based-mouse-distance-multiplier"];

	settings.barrier.draw = j["mouse-barrier"]["draw"];
	settings.barrier.radius = j["mouse-barrier"]["radius"];
	settings.barrier.color = j["mouse-barrier"]["color"].get<Color>();
	settings.barrier.blur = j["mouse-barrier"]["blur"];

	settings.offsetBounds = j["offset-bounds"];

	settings.MSAA = j["MSAA"];

	return settings;
}
