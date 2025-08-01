cmake_minimum_required(VERSION 3.30)
project(Dlaunay-Flow LANGUAGES CXX C)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# General compiler flags for both debug and release
add_compile_options(
    /W3           # Warning level 3
    /sdl          # SDL check
    /permissive-  # Conformance mode
    /fp:fast      # Fast floating point model
    /MP           # Multi-processor compilation
)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/glad.c
    src/settings.cpp
    src/star.cpp
    src/desktopUtils.cpp
    src/trayUtils.cpp
    src/utils.cpp
)

# Headers (not strictly needed for compilation, but good for IDE integration)
set(HEADERS
    src/settings.h
    src/star.h
    src/desktopUtils.h
    src/trayUtils.h
    src/resource.h
    src/utils.h
)

# Resource files
set(RESOURCES
    resource/Resource.rc
    resource/icon.ico
)

# Additional files to copy
set(EXTRA_FILES
    resource/settings.json
    shaders/vertex.glsl
    shaders/fragment.glsl
)

# Create the executable
add_executable(Dlaunay-Flow ${SOURCES} ${HEADERS} ${RESOURCES})

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "_UNICODE" "UNICODE")
endif()

# Link libraries
target_link_libraries(Dlaunay-Flow
    opengl32.lib
    triangle.lib
    glfw3_mt.lib
)

# Set library directories
target_link_directories(Dlaunay-Flow PRIVATE lib)

# Configuration-specific settings
# Release configuration
target_compile_options(Dlaunay-Flow PRIVATE $<$<CONFIG:Release>:
    /MT           # MultiThreaded runtime
    /DNDEBUG
    /D_CONSOLE
    /GL           # Whole program optimization
    /O2           # Optimize for speed
>)

target_link_options(Dlaunay-Flow PRIVATE $<$<CONFIG:Release>:
    /LTCG         # Link-time code generation
    /OPT:REF      # Optimize references
    /OPT:ICF      # Enable COMDAT folding
>)

# Copy additional files to build directory
add_custom_command(TARGET Dlaunay-Flow POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/resource/settings.json
        $<TARGET_FILE_DIR:Dlaunay-Flow>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:Dlaunay-Flow>/shaders
)
