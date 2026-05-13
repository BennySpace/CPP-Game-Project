if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is required")
endif()

file(READ "${SOURCE_DIR}/data/items.txt" ITEMS_TEXT)
file(READ "${SOURCE_DIR}/data/rivals.txt" RIVALS_TEXT)
file(READ "${SOURCE_DIR}/data/locations.txt" LOCATIONS_TEXT)
file(READ "${SOURCE_DIR}/data/logs.txt" LOGS_TEXT)
file(READ "${SOURCE_DIR}/data/strings.txt" STRINGS_TEXT)

get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

file(WRITE "${OUTPUT_FILE}" "#pragma once\n\n#include <string_view>\n\nnamespace EmbeddedData {\n")
file(APPEND "${OUTPUT_FILE}" "inline constexpr std::string_view kItemsText = R\"EMBED_ITEMS(${ITEMS_TEXT})EMBED_ITEMS\";\n\n")
file(APPEND "${OUTPUT_FILE}" "inline constexpr std::string_view kRivalsText = R\"EMBED_RIVALS(${RIVALS_TEXT})EMBED_RIVALS\";\n\n")
file(APPEND "${OUTPUT_FILE}" "inline constexpr std::string_view kLocationsText = R\"EMBED_LOCATIONS(${LOCATIONS_TEXT})EMBED_LOCATIONS\";\n\n")
file(APPEND "${OUTPUT_FILE}" "inline constexpr std::string_view kLogsText = R\"EMBED_LOGS(${LOGS_TEXT})EMBED_LOGS\";\n\n")
file(APPEND "${OUTPUT_FILE}" "inline constexpr std::string_view kStringsText = R\"EMBED_STRINGS(${STRINGS_TEXT})EMBED_STRINGS\";\n")
file(APPEND "${OUTPUT_FILE}" "}\n")
