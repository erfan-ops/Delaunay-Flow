# embed_shaders.cmake
string(REPLACE "|" ";" INPUT_FILES "${INPUT_FILES}")

file(WRITE ${OUTPUT_FILE} "#pragma once\n\n")

foreach(SHADER ${INPUT_FILES})
    # Trim any whitespace that might have been introduced
    string(STRIP "${SHADER}" SHADER)
    
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)

    file(READ ${SHADER} CONTENTS)

    file(APPEND ${OUTPUT_FILE}
        "static constexpr const char* ${SHADER_NAME}_glsl = R\"glsl(\n${CONTENTS}\n)glsl\";\n\n"
    )
endforeach()
