// settings.cpp
#include "settings.h"
#include <fstream>
#include <nlohmann/json.hpp>

void Settings::Load(const std::string& filename) {
    Instance().loadFromFile(filename);
}

void Settings::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    nlohmann::json j;
    file >> j;

    targetFPS = j["fps"];
    vsync = j["vsync"];
    backGroundColors = j["background-colors"].get<std::vector<Color>>();

    stars.draw = j["stars"]["draw"];
    stars.segments = j["stars"]["segments"];
    stars.radius = j["stars"]["radius"];
    stars.count = j["stars"]["count"];
    stars.minSpeed = j["stars"]["min-speed"];
    stars.maxSpeed = j["stars"]["max-speed"];
    stars.color = j["stars"]["color"].get<Color>();

    edges.draw = j["edges"]["draw"];
    edges.width = j["edges"]["width"];
    edges.color = j["edges"]["color"].get<Color>();

    interaction.mouseInteraction = j["interaction"]["mouse-interaction"];
    interaction.distanceFromMouse = j["interaction"]["distance-from-mouse"];
    interaction.speedBasedMouseDistanceMultiplier =
        j["interaction"]["speed-based-mouse-distance-multiplier"];

    barrier.draw = j["mouse-barrier"]["draw"];
    barrier.radius = j["mouse-barrier"]["radius"];
    barrier.color = j["mouse-barrier"]["color"].get<Color>();
    barrier.blur = j["mouse-barrier"]["blur"];

    offsetBounds = j["offset-bounds"];
    MSAA = j["MSAA"];
}
