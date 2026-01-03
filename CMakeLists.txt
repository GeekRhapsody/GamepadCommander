cmake_minimum_required(VERSION 3.16)
project(GamepadCommander LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(CURL)

add_executable(GamepadCommander
    src/main.cpp
)

if (TARGET SDL2::SDL2)
    target_link_libraries(GamepadCommander PRIVATE SDL2::SDL2)
    if (TARGET SDL2::SDL2main)
        target_link_libraries(GamepadCommander PRIVATE SDL2::SDL2main)
    endif()
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
