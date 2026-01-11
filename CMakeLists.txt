cmake_minimum_required(VERSION 3.16)
project(GamepadCommander LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(CURL)

add_executable(GamepadCommander
    src/main.cpp
    src/SteamHelper.cpp
)

if (WIN32)
    set(WINDOWS_ICON_RC "${CMAKE_BINARY_DIR}/GamepadCommander.rc")
    configure_file(
        "${CMAKE_SOURCE_DIR}/packaging/GamepadCommander.rc.in"
        "${WINDOWS_ICON_RC}"
        @ONLY
    )
    target_sources(GamepadCommander PRIVATE "${WINDOWS_ICON_RC}")
endif()

if (TARGET SDL2::SDL2)
    if (TARGET SDL2::SDL2main)
        target_link_libraries(GamepadCommander PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(GamepadCommander PRIVATE SDL2::SDL2)
else()
    target_include_directories(GamepadCommander PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(GamepadCommander PRIVATE ${SDL2_LIBRARIES})
endif()

if (CURL_FOUND)
    target_compile_definitions(GamepadCommander PRIVATE USE_CURL=1)
    if (TARGET CURL::libcurl)
        target_link_libraries(GamepadCommander PRIVATE CURL::libcurl)
    else()
        target_include_directories(GamepadCommander PRIVATE ${CURL_INCLUDE_DIRS})
        target_link_libraries(GamepadCommander PRIVATE ${CURL_LIBRARIES})
    endif()
endif()

if (UNIX AND NOT APPLE)
    option(BUILD_APPIMAGE "Build AppImage during the default build" OFF)
    set(APPIMAGETOOL "" CACHE FILEPATH "Path to appimagetool")
    if (NOT APPIMAGETOOL)
        find_program(APPIMAGETOOL appimagetool)
    endif()
    if (APPIMAGETOOL)
        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" APPIMAGE_ARCH)
        if (APPIMAGE_ARCH STREQUAL "amd64")
            set(APPIMAGE_ARCH "x86_64")
        elseif (APPIMAGE_ARCH STREQUAL "arm64")
            set(APPIMAGE_ARCH "aarch64")
        endif()

        set(APPIMAGE_APPDIR "${CMAKE_BINARY_DIR}/AppDir")
        set(APPIMAGE_OUTPUT "${CMAKE_BINARY_DIR}/GamepadCommander.AppImage")

        set(APPIMAGE_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPIMAGE_APPDIR}/usr/bin"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPIMAGE_APPDIR}/usr/share/applications"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPIMAGE_APPDIR}/usr/share/icons/hicolor/scalable/apps"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GamepadCommander>" "${APPIMAGE_APPDIR}/usr/bin/GamepadCommander"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/packaging/AppRun" "${APPIMAGE_APPDIR}/AppRun"
            COMMAND chmod +x "${APPIMAGE_APPDIR}/AppRun"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/packaging/GamepadCommander.desktop"
                    "${APPIMAGE_APPDIR}/usr/share/applications/GamepadCommander.desktop"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/packaging/GamepadCommander.desktop"
                    "${APPIMAGE_APPDIR}/GamepadCommander.desktop"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/packaging/GamepadCommander.svg"
                    "${APPIMAGE_APPDIR}/usr/share/icons/hicolor/scalable/apps/GamepadCommander.svg"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/packaging/GamepadCommander.svg"
                    "${APPIMAGE_APPDIR}/GamepadCommander.svg"
        )
        list(APPEND APPIMAGE_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E env "APPIMAGE_EXTRACT_AND_RUN=1" "ARCH=${APPIMAGE_ARCH}" "${APPIMAGETOOL}"
                    "${APPIMAGE_APPDIR}" "${APPIMAGE_OUTPUT}"
        )

        add_custom_command(
            OUTPUT "${APPIMAGE_OUTPUT}"
            ${APPIMAGE_COMMANDS}
            DEPENDS GamepadCommander
            COMMENT "Creating AppImage"
            VERBATIM
        )

        if (BUILD_APPIMAGE)
            add_custom_target(appimage ALL DEPENDS "${APPIMAGE_OUTPUT}")
        else()
            add_custom_target(appimage DEPENDS "${APPIMAGE_OUTPUT}")
        endif()
    elseif (BUILD_APPIMAGE)
        message(FATAL_ERROR "BUILD_APPIMAGE is ON but appimagetool was not found in PATH.")
    endif()
endif()
