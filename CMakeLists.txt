cmake_minimum_required(VERSION 3.20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
project(rack-hack VERSION 1.0 LANGUAGES C CXX)

set(SURGE_SKIP_JUCE_FOR_RACK TRUE)
set(SURGE_SKIP_LUA TRUE)
add_subdirectory(surge)

set(CMAKE_CXX_STANDARD 17)

file(GLOB RACK_SOURCES src/*.cpp)
set(RACK_DIR "/Users/paul/dev/music/rack/Rack-SDK")

add_library(${PROJECT_NAME} STATIC ${RACK_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${RACK_DIR}/include
        ${RACK_DIR}/dep/include
        )
target_link_libraries(${PROJECT_NAME} PRIVATE surge-common)

add_custom_command(TARGET ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        # COMMAND RACK_DIR=${RACK_DIR} arch -x86_64 make dist -j
        COMMAND RACK_DIR=${RACK_DIR} arch -x86_64 make install -j
        USES_TERMINAL)