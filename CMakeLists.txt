cmake_minimum_required(VERSION 3.30)
project(Delaunay-Flow LANGUAGES CXX C)

# ============================================================
# Compiler settings
# ============================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)  # Enables /GL + /LTCG automatically

# ============================================================
# Global compile options
# ============================================================
add_compile_options(
    /W3             # Reasonable warning level
    /sdl            # Additional security checks
    /permissive-    # Strict standard conformance
    /fp:fast        # Fast floating point math
    /arch:AVX2      # Use AVX2 SIMD instructions (change to AVX512 if supported)
    /Oi             # Intrinsics
    /Ot             # Favor fast code
    /GT             # Fiber-safe TLS (safe for multithreading)
    /GL             # Whole program optimization
    /MP             # Multi-processor compilation
)

# Application sources - everything else
set(APP_SOURCES
    src/main.cpp
    src/settings.cpp
    src/star.cpp
    src/color_interpolation.cpp
    src/shader_utils.cpp
    include/glad/glad.c
)

set(HEADERS
    src/resource.h
)

set(RESOURCES
    resource/Resource.rc
    resource/icon.ico
)

set(EXTRA_FILES
    resource/settings.json
    shaders/vertex.glsl
    shaders/fragment.glsl
)

# ============================================================
# Executable target
# ============================================================
add_executable(Delaunay-Flow ${APP_SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(Delaunay-Flow PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_options(${PROJECT_NAME} PRIVATE "/ENTRY:mainCRTStartup")

# ============================================================
# 🪟 MSVC-specific settings
# ============================================================
if(MSVC)
    target_compile_definitions(Delaunay-Flow PRIVATE
        _UNICODE
        UNICODE
        NOMINMAX
        WALLPAPER_HOST_STATIC=1
    )

    # Compiler optimization settings per configuration
    target_compile_options(Delaunay-Flow PRIVATE
        $<$<CONFIG:Release>:
            /O2               # Maximize speed
            /Ot               # Favor fast code
            /fp:fast          # Fast floating-point model
            /DNDEBUG
            /MT               # Static runtime for portability
            /GL               # Whole program optimization
        >
        $<$<CONFIG:Debug>:
            /Od               # Disable optimizations
            /RTC1             # Runtime error checks
            /D_DEBUG
            /MTd              # Static debug runtime
            /Zi               # Debug information
        >
    )

    # Linker optimization settings
    target_link_options(Delaunay-Flow PRIVATE
        $<$<CONFIG:Release>:
            /LTCG             # Link-time code generation
            /OPT:REF          # Remove unreferenced functions/data
            /OPT:ICF          # Fold identical COMDATs
            /INCREMENTAL:NO   # Disable incremental linking for full optimization
            /RELEASE          # Remove debug information from PE header
        >
        $<$<CONFIG:Debug>:
            /DEBUG            # Generate debug info
            /INCREMENTAL      # Enable incremental linking for faster debug builds
        >
    )
endif()

# ============================================================
# Linking
# ============================================================
target_link_libraries(Delaunay-Flow PRIVATE
    wallpaper_host_static_mt.lib
    opengl32.lib
    glfw3_mt.lib

)

target_link_directories(Delaunay-Flow PRIVATE lib)



# ============================================================
# Post-build steps
# ============================================================
add_custom_command(TARGET Delaunay-Flow POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/resource/settings.json
        $<TARGET_FILE_DIR:Delaunay-Flow>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:Delaunay-Flow>/shaders
)
