#ifndef DELAUNAY_FLOW_SETTINGS_HPP
#define DELAUNAY_FLOW_SETTINGS_HPP

#include "types.hpp"
#include <string>
#include <vector>

namespace delaunay_flow {

class Settings {
public:
    static Settings& Instance() noexcept {
        static Settings instance;
        return instance;
    }

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

private:
    Settings();
    void loadFromFile();

public:
    float targetFPS = 0.0f;
    bool vsync = false;
    std::vector<Color> backGroundColors;

    struct Stars {
        bool draw = false;
        int segments = 0;
        float radius = 0.0f;
        int count = 0;
        float minSpeed = 0.0f;
        float maxSpeed = 0.0f;
        Color color{};
    } stars;

    struct Edges {
        bool draw = false;
        float width = 0.0f;
        Color color{};
    } edges;

    struct Interaction {
        bool mouseInteraction = false;
        float distanceFromMouse = 0.0f;
    } interaction;

    struct Barrier {
        bool draw = false;
        float radius = 0.0f;
        Color color{};
        float blur = 0.0f;
    } barrier;

    float offsetBounds = 0.0f;
    int MSAA = 1;
};

}  // namespace delaunay_flow

#endif  // DELAUNAY_FLOW_SETTINGS_HPP
