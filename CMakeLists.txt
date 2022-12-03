#cmake_minimum_required (VERSION 3.20)
cmake_minimum_required (VERSION 3.16)
project (surge-rack VERSION 1.0 LANGUAGES C CXX)
include (RackSDK.cmake)
set (CMAKE_CXX_STANDARD 17)

## check for Windows non gcc env!!!

if ("${RACK_SDK_DIR}" STREQUAL "")
  message(FATAL_ERROR "Path to Rack SDK is missing! Add -DRACK_SDK_DIR=<PATH> to the cmake call.")
else ()
  message (STATUS "use Rack-SDK in '${RACK_SDK_DIR}'")
endif ()

find_package (Git REQUIRED)

execute_process (COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE SURGE_RACK_PLUG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process (COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/surge
        OUTPUT_VARIABLE SURGE_RACK_SURGE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

set (SURGE_RACK_BASE_VERSION XT1-0-1)

message (STATUS "SURGE_RACK_BASE_VERSION ${SURGE_RACK_BASE_VERSION}")
message (STATUS "SURGE_RACK_PLUG_VERSION ${SURGE_RACK_PLUG_VERSION}")
message (STATUS "SURGE_RACK_SURGE_VERSION ${SURGE_RACK_SURGE_VERSION}")


set (SURGE_SKIP_JUCE_FOR_RACK TRUE CACHE BOOL "skip surge Juce build")
set (SURGE_SKIP_LUA TRUE CACHE BOOL "skip surge Lua build")
set (SURGE_SKIP_AIRWINDOWS TRUE CACHE BOOL "skip surge airwindows build")
set (SURGE_SKIP_VST3 TRUE CACHE BOOL "skip surge VST3 build")
set (SURGE_SKIP_ALSA TRUE CACHE BOOL "skip surge Alsa build")
set (SURGE_SKIP_STANDALONE TRUE CACHE BOOL "skip surge standalone build")
set (SURGE_COMPILE_BLOCK_SIZE 8 CACHE STRING "surge compile block size")

add_subdirectory (surge)
# turn off specific warning as errors enforced by Rack-SDK compiler options for surge libs
# the warnings should be fixed in the dependencies or the warning could be disabled in the
# dependency CMake definitions and removed from here
target_compile_options (juce_dsp_rack_sub PRIVATE -Wno-maybe-uninitialized)
target_compile_options (eurorack PRIVATE -Wno-unused-local-typedefs)
target_compile_options (oddsound-mts PRIVATE -Wno-char-subscripts)
target_compile_options (sqlite PRIVATE -Wno-implicit-fallthrough -Wno-cast-function-type -Wno-misleading-indentation)
target_compile_options (sst-plugininfra PRIVATE -Wno-unused-variable)
target_compile_options (sst-waveshapers INTERFACE -Wno-unused-variable)
target_compile_options (tinyxml PRIVATE -Wno-implicit-fallthrough -Wno-sign-compare)
target_compile_options (tuning-library INTERFACE -Wno-maybe-uninitialized)
target_compile_options (surge-common PRIVATE -Wno-sign-compare -Wno-ignored-qualifiers -Wno-unused-value
  -Wno-implicit-fallthrough -Wno-reorder -Wno-unused-but-set-variable -Wno-char-subscripts -Wno-stringop-truncation
  -Wno-unused-local-typedefs -Wno-class-memaccess -Wno-range-loop-construct -Wno-nonnull -Wno-uninitialized)

file (GLOB RACK_SOURCES src/*.cpp)
list(REMOVE_ITEM RACK_SOURCES src/SurgeXT.cpp)

add_library (${PROJECT_NAME} STATIC ${RACK_SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)

target_link_libraries (${PROJECT_NAME} PRIVATE surge-common)

add_library (plugin SHARED src/SurgeXT.cpp)
target_include_directories (plugin PRIVATE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
target_link_libraries (plugin PRIVATE ${PROJECT_NAME})

function (_get_all_cmake_targets out_var current_dir)
    get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)

    foreach(subdir ${subdirs})
        _get_all_cmake_targets(subdir_targets ${subdir})
        list(APPEND targets ${subdir_targets})
    endforeach()

    set(${out_var} ${targets} PARENT_SCOPE)
endfunction()

# Run at end of top-level CMakeLists
_get_all_cmake_targets(all_targets ${CMAKE_CURRENT_LIST_DIR})

message (STATUS "Buildtargets -- ${all_targets}")
