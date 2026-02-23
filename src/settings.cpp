#include <settings.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <Windows.h>


namespace {

static void showError(const std::string& msg) {
    MessageBoxA(nullptr, msg.c_str(), "Settings Load Error", MB_OK | MB_ICONERROR);
}

} // namespace

namespace delaunay_flow {

namespace {
    constexpr const char* kSettingsFilename = "settings.json";
}

Settings::Settings() {
    loadFromFile();
}

void Settings::loadFromFile() {
    try
    {
        std::ifstream file(kSettingsFilename);
        if (!file.is_open())
            throw std::runtime_error(
                "Could not open the settings file:\n" + std::string(kSettingsFilename) +
                "\n\nPlease make sure the file exists and is accessible.");

        nlohmann::json j;
        file >> j;

        // --- fps ---
        if (!j["fps"].is_number() || j["fps"] <= 0.0f)
            throw std::runtime_error(
                "Invalid value for \"fps\".\n"
                "Frames per second must be greater than 0.");
        targetFPS = j["fps"];

        // --- vsync ---
        if (!j["vsync"].is_boolean())
            throw std::runtime_error(
                "Invalid value for \"vsync\".\n"
                "This setting must be either true or false.");
        vsync = j["vsync"];

        // --- background-colors ---
        if (!j["background-colors"].is_array())
            throw std::runtime_error(
                "Invalid \"background-colors\" section.\n"
                "It must be a list of colors.");

        backGroundColors.clear();
        for (size_t i = 0; i < j["background-colors"].size(); ++i)
        {
            const auto& color = j["background-colors"][i];
            if (!color.is_array() || color.size() != 4)
                throw std::runtime_error(
                    "Invalid color in \"background-colors\".\n"
                    "Each color must contain exactly 4 numbers (R, G, B, A).");
        }
        backGroundColors = j["background-colors"].get<std::vector<Color>>();

        // --- stars ---
        auto& js = j["stars"];

        if (!js["draw"].is_boolean())
            throw std::runtime_error(
                "Invalid \"stars.draw\" value.\n"
                "This setting must be either true or false.");
        stars.draw = js["draw"];

        if (!js["segments"].is_number_integer() || js["segments"] < 3)
            throw std::runtime_error(
                "Invalid \"stars.segments\" value.\n"
                "It must be greater than 2.");
        stars.segments = js["segments"];

        if (!js["radius"].is_number() || js["radius"] <= 0.0f)
            throw std::runtime_error(
                "Invalid \"stars.radius\" value.\n"
                "It must be greater than 0.");
        stars.radius = js["radius"];

        if (!js["count"].is_number_integer() || js["count"] <= 0)
            throw std::runtime_error(
                "Invalid \"stars.count\" value.\n"
                "It must be greater than 0.");
        stars.count = js["count"];

        if (!js["min-speed"].is_number() || js["min-speed"] < 0.0f)
            throw std::runtime_error(
                "Invalid \"stars.min-speed\" value.\n"
                "It cannot be negative.");
        stars.minSpeed = js["min-speed"];

        if (!js["max-speed"].is_number() || js["max-speed"] < 0.0f)
            throw std::runtime_error(
                "Invalid \"stars.max-speed\" value.\n"
                "It cannot be negative.");
        stars.maxSpeed = js["max-speed"];

        if (!js["color"].is_array() || js["color"].size() != 4)
            throw std::runtime_error(
                "Invalid \"stars.color\" value.\n"
                "Color must contain exactly 4 numbers (R, G, B, A).");
        stars.color = js["color"].get<Color>();

        // --- edges ---
        auto& je = j["edges"];

        if (!je["draw"].is_boolean())
            throw std::runtime_error(
                "Invalid \"edges.draw\" value.\n"
                "This setting must be either true or false.");
        edges.draw = je["draw"];

        if (!je["width"].is_number() || je["width"] <= 0.0f)
            throw std::runtime_error(
                "Invalid \"edges.width\" value.\n"
                "It must be greater than 0.");
        edges.width = je["width"];

        if (!je["color"].is_array() || je["color"].size() != 4)
            throw std::runtime_error(
                "Invalid \"edges.color\" value.\n"
                "Color must contain exactly 4 numbers (R, G, B, A).");
        edges.color = je["color"].get<Color>();

        // --- interaction ---
        auto& ji = j["interaction"];

        if (!ji["mouse-interaction"].is_boolean())
            throw std::runtime_error(
                "Invalid \"interaction.mouse-interaction\" value.\n"
                "This setting must be either true or false.");
        interaction.mouseInteraction = ji["mouse-interaction"];

        if (!ji["distance-from-mouse"].is_number() ||
            ji["distance-from-mouse"] <= 0.0f)
            throw std::runtime_error(
                "Invalid \"interaction.distance-from-mouse\" value.\n"
                "It must be greater than 0.");
        interaction.distanceFromMouse = ji["distance-from-mouse"];

        // --- mouse-barrier ---
        auto& jb = j["mouse-barrier"];

        if (!jb["draw"].is_boolean())
            throw std::runtime_error(
                "Invalid \"mouse-barrier.draw\" value.\n"
                "This setting must be either true or false.");
        barrier.draw = jb["draw"];

        if (!jb["radius"].is_number() || jb["radius"] <= 0.0f)
            throw std::runtime_error(
                "Invalid \"mouse-barrier.radius\" value.\n"
                "It must be greater than 0.");
        barrier.radius = jb["radius"];

        if (!jb["color"].is_array() || jb["color"].size() != 4)
            throw std::runtime_error(
                "Invalid \"mouse-barrier.color\" value.\n"
                "Color must contain exactly 4 numbers (R, G, B, A).");
        barrier.color = jb["color"].get<Color>();

        if (!jb["blur"].is_number() || jb["blur"] <= 0.0f)
            throw std::runtime_error(
                "Invalid \"mouse-barrier.blur\" value.\n"
                "It must be greater than 0.");
        barrier.blur = jb["blur"];

        // --- offset-bounds ---
        if (!j["offset-bounds"].is_number() || j["offset-bounds"] < 0.0f)
            throw std::runtime_error(
                "Invalid \"offset-bounds\" value.\n"
                "It cannot be negative.");
        offsetBounds = j["offset-bounds"];

        // --- MSAA ---
        if (!j["MSAA"].is_number_integer() || j["MSAA"] < 0)
            throw std::runtime_error(
                "Invalid \"MSAA\" value.\n"
                "It must be 0 or a positive whole number.");
        MSAA = j["MSAA"];
    }
    catch (const nlohmann::json::parse_error&)
    {
        showError(
            "The settings file is not formatted correctly.\n\n"
            "Please check the JSON structure for missing commas or brackets.");
    }
    catch (const nlohmann::json::exception& e)
    {
        showError(std::string("There is a problem in the settings file:\n\n") + e.what());
    }
    catch (const std::exception& e)
    {
        showError(e.what());
    }
}


}  // namespace delaunay_flow
