#include <application.hpp>
#include <iostream>


int main() {
    try {
        delaunay_flow::Application app;
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return -1;
    }
}
