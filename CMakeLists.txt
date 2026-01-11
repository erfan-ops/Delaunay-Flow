cmake_minimum_required(VERSION 3.30)
project(Delaunay-Flow LANGUAGES CXX C)

# ============================================================
# 🔧 Compiler settings
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)  # Enables /GL + /LTCG automatically

# ============================================================
# 🧠 Global compile options
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

# ============================================================
# 🧩 Include and source setup
# ============================================================
include_directories(include)

set(SOURCES
    src/main.cpp
    src/glad.c
    src/settings.cpp
    src/star.cpp
    src/desktopUtils.cpp
    src/trayUtils.cpp
    src/utils.cpp
)

set(HEADERS
    src/settings.h
    src/star.h
    src/desktopUtils.h
    src/trayUtils.h
    src/resource.h
    src/utils.h
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
# 🏗️ Build target
# ============================================================
add_executable(Delaunay-Flow ${SOURCES} ${HEADERS} ${RESOURCES})

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
        NOMINMAX              # Prevent macros interfering with std::min/max
    )

    # Compiler optimization settings per configuration
    target_compile_options(Delaunay-Flow PRIVATE
        $<$<CONFIG:Release>:
            /O2               # Maximize speed
            /Ob3              # Aggressive inlining
            /Ot               # Favor fast code
            /fp:fast          # Fast floating-point model
            /DNDEBUG
            /MT               # Static runtime for portability
            /GL               # Whole program optimization
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
    )
endif()

# ============================================================
# 🔗 Linking
# ============================================================
target_link_libraries(Delaunay-Flow
    opengl32.lib
    glfw3_mt.lib
)

target_link_directories(Delaunay-Flow PRIVATE lib)

# ============================================================
# 📂 Post-build steps
# ============================================================
add_custom_command(TARGET Delaunay-Flow POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/resource/settings.json
        $<TARGET_FILE_DIR:Delaunay-Flow>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:Delaunay-Flow>/shaders
)

# ============================================================
# 🧪 Optional diagnostics (enable manually if needed)
# ============================================================
# target_compile_options(Delaunay-Flow PRIVATE /Qvec-report:2)  # Vectorization report
