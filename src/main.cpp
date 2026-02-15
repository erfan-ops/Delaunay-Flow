#include <application.hpp>
#include <iostream>


int main() {
    try {
        delaunay_flow::Application app;
        return app.run();
    } catch (const std::exception& ex) {
        MessageBoxA(nullptr, ex.what(), "Fatal error:", MB_OK | MB_ICONERROR);
        return -1;
    }
}
