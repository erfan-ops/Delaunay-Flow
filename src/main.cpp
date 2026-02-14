#include <application.hpp>
#include <settings.hpp>
#include <iostream>


int main() {
    try {
        delaunay_flow::Settings::Load("settings.json");
        delaunay_flow::Application app(delaunay_flow::Settings::Instance());
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return -1;
    }
}
