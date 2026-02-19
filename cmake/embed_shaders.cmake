# embed_shaders.cmake
string(REPLACE "|" ";" INPUT_FILES "${INPUT_FILES}")

file(WRITE ${OUTPUT_FILE} "#pragma once\n\n")

foreach(SHADER ${INPUT_FILES})
    string(STRIP "${SHADER}" SHADER)

    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_filename_component(SHADER_EXT  ${SHADER} EXT)

    # Remove leading dot from extension
    string(SUBSTRING "${SHADER_EXT}" 1 -1 SHADER_EXT)

    # make name lowercase
    string(TOLOWER "${SHADER_NAME}" SHADER_NAME)
    string(TOLOWER "${SHADER_EXT}" SHADER_EXT)

    file(READ ${SHADER} CONTENTS)
    string(STRIP "${CONTENTS}" CONTENTS)

    file(APPEND ${OUTPUT_FILE}
        "static constexpr const char* ${SHADER_NAME}_${SHADER_EXT} = R\"glsl(\n${CONTENTS}\n)glsl\";\n\n"
    )
endforeach()
