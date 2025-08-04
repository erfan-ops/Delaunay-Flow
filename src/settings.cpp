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

	settings.stars.draw = j["stars"]["draw-stars"];
	settings.stars.segments = j["stars"]["segments"];
	settings.stars.radius = j["stars"]["radius"];
	settings.stars.count = j["stars"]["count"];
	settings.stars.minSpeed = j["stars"]["min-speed"];
	settings.stars.maxSpeed = j["stars"]["max-speed"];
	settings.stars.color = j["stars"]["color"].get<Color>();

	settings.drawLines = j["draw-lines"];
	settings.lineWidth = j["line-width"];
	settings.linesColor = j["lines-color"].get<Color>();

	settings.moveFromMouse = j["mouse-interaction"];
	settings.mouseDistance = j["keep-distance-from-mouse"];
	settings.speedBasedMouseDistanceMultiplier = j["dynamic-mouse-distance-factor"];
	settings.offsetBounds = j["offset-bounds"];

	settings.MSAA = j["MSAA"];

	return settings;
}
