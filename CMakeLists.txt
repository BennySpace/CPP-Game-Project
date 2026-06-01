cmake_minimum_required(VERSION 3.16)

# Core project metadata used by CMake and IDE integrations.
project(EchoesOfTheMajestic
    VERSION 1.0
    DESCRIPTION "Text-based survival-horror dungeon crawler"
    LANGUAGES CXX
)

# Keep the toolchain predictable across local builds and CI.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Build toggle for stricter local development diagnostics.
option(ENABLE_WARNINGS "Enable strict compiler warnings" ON)

set(APP_TARGET majestic_station)

# Main source paths and generated artifacts used across the build.
set(APP_ENTRYPOINT "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
set(APP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/favicon.ico")
set(GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
set(EMBEDDED_DATA_HEADER "${GENERATED_INCLUDE_DIR}/Data/EmbeddedData.h")

# Collect implementation files automatically; the entry point is added separately.
file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(GAME_SOURCES ${SOURCES})
list(REMOVE_ITEM GAME_SOURCES "${APP_ENTRYPOINT}")

add_executable(${APP_TARGET})

target_sources(${APP_TARGET} PRIVATE
    ${APP_ENTRYPOINT}
    ${GAME_SOURCES}
)

# Generate a reserve built-in copy of the text files so the standalone exe can
# still run without an external data folder.
add_custom_command(
    OUTPUT "${EMBEDDED_DATA_HEADER}"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DOUTPUT_FILE=${EMBEDDED_DATA_HEADER}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateEmbeddedData.cmake"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateEmbeddedData.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/items.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/rivals.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/locations.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/logs.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/strings.txt"
    COMMENT "Generating embedded data header"
)

add_custom_target(generate_embedded_data DEPENDS "${EMBEDDED_DATA_HEADER}")
add_dependencies(${APP_TARGET} generate_embedded_data)

# Attach the Windows icon resource when the .ico file is present locally.
if(WIN32 AND EXISTS "${APP_ICON_FILE}")
    set(APP_ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    set(ICON_FILE "${APP_ICON_FILE}")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/app_icon.rc.in"
        "${APP_ICON_RC}"
        @ONLY
    )
    target_sources(${APP_TARGET} PRIVATE "${APP_ICON_RC}")
    target_compile_definitions(${APP_TARGET} PRIVATE APP_HAS_ICON_RESOURCE=1)
    message(STATUS "Windows icon resource enabled: ${APP_ICON_FILE}")
elseif(WIN32)
    message(STATUS "Windows icon resource skipped: ${APP_ICON_FILE} not found")
endif()

# Include source headers plus generated headers from the embedded data step.
target_include_directories(${APP_TARGET} PRIVATE
    ${GENERATED_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows
)

# Prefer strict warnings in development to catch portability and type issues early.
if(ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(${APP_TARGET} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${APP_TARGET} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
        )
    endif()
endif()

# Ship Release with the static MSVC runtime so the exe can be shared as a
# single file, while Debug keeps the dynamic debug runtime for local tooling.
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET ${APP_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    else()
        set_property(TARGET ${APP_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
endif()

# Mirror the active build type into compile definitions where the code expects it.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${APP_TARGET} PRIVATE DEBUG)
    message(STATUS "Build type: Debug")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

# Basic install rule for manual packaging or future installer work.
install(TARGETS ${APP_TARGET}
    RUNTIME DESTINATION bin
)

# Keep the external data folder next to the executable for development builds;
# the game can still fall back to embedded data when the folder is absent.
add_custom_command(TARGET ${APP_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:${APP_TARGET}>/data
    COMMENT "Copying data folder to executable directory"
)

# Emit a compact summary so local configure/build logs are easier to scan.
message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Target: ${APP_TARGET}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Warnings: ${ENABLE_WARNINGS}")
message(STATUS "========================================")
