cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(SurgeXTRack VERSION 1.0 LANGUAGES C CXX)

if ("${RACK_SDK_DIR}" STREQUAL "")
  message(FATAL_ERROR "Path to Rack SDK is missing! Add -DRACK_SDK_DIR=<PATH> to the cmake call.")
else ()
  message(STATUS "Using Rack-SDK in '${RACK_SDK_DIR}'")
endif ()

message(STATUS "Installing into '${CMAKE_INSTALL_PREFIX}'")

include(RackSDK.cmake)

set(CMAKE_CXX_STANDARD 17)

set(PLUGIN_NAME ${PROJECT_NAME})

# Do not change the LIB_NAME!
set(LIB_NAME plugin)

find_package(Git REQUIRED)

execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE SURGE_RACK_PLUG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/surge
        OUTPUT_VARIABLE SURGE_RACK_SURGE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "SURGE_RACK_PLUG_VERSION ${SURGE_RACK_PLUG_VERSION}")
message(STATUS "SURGE_RACK_SURGE_VERSION ${SURGE_RACK_SURGE_VERSION}")

set(SURGE_SKIP_JUCE_FOR_RACK TRUE CACHE BOOL "skip surge Juce build")
set(SURGE_SKIP_LUA TRUE CACHE BOOL "skip surge Lua build")
set(SURGE_SKIP_AIRWINDOWS TRUE CACHE BOOL "skip surge airwindows build")
set(SURGE_SKIP_ODDSOUND_MTS TRUE CACHE BOOL "skip surge oddsound-mts build")
set(SURGE_SKIP_VST3 TRUE CACHE BOOL "skip surge VST3 build")
set(SURGE_SKIP_ALSA TRUE CACHE BOOL "skip surge Alsa build")
set(SURGE_SKIP_STANDALONE TRUE CACHE BOOL "skip surge standalone build")
set(SURGE_COMPILE_BLOCK_SIZE 8 CACHE STRING "surge compile block size")

add_subdirectory(surge)
# turn off specific warning as errors enforced by Rack-SDK compiler options for surge libs
# the warnings should be fixed in the dependencies or the warning could be disabled in the
# dependency CMake definitions and removed from here
target_compile_options(juce_dsp_rack_sub PRIVATE -Wno-deprecated-copy $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>)
target_compile_options(eurorack PRIVATE -Wno-unused-local-typedefs $<$<PLATFORM_ID:Darwin>:-Wno-infinite-recursion>)
target_compile_options(sqlite PRIVATE -Wno-error)
target_compile_options(sst-plugininfra PRIVATE -Wno-unused-variable)
target_compile_options(sst-waveshapers INTERFACE -Wno-unused-variable)
target_compile_options(sst-filters INTERFACE -Wno-unused-variable $<$<CONFIG:Debug>:-Wno-sign-compare>)
target_compile_options(tinyxml PRIVATE -Wno-implicit-fallthrough -Wno-sign-compare)
target_compile_options(tuning-library INTERFACE $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
                       $<$<PLATFORM_ID:Darwin>:-Wno-unused-private-field>)
target_compile_options(surge-common PRIVATE -Wno-sign-compare -Wno-ignored-qualifiers -Wno-unused-value -Wno-uninitialized
                       -Wno-implicit-fallthrough -Wno-char-subscripts -Wno-unused-local-typedefs -Wno-range-loop-construct
                       -Wno-nonnull -Wno-reorder
                       $<IF:$<CXX_COMPILER_ID:Clang>,-Wno-unused-const-variable,-Wno-unused-but-set-variable>
                       $<IF:$<CXX_COMPILER_ID:GNU>,-Wno-stringop-truncation,-Wno-string-concatenation>
                       $<IF:$<CXX_COMPILER_ID:GNU>,-Wno-class-memaccess,-Wno-class-varargs>
                       $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
                       $<$<PLATFORM_ID:Darwin>:-Wno-self-assign>
                       $<$<PLATFORM_ID:Darwin>:-Wno-unused-lambda-capture>
                       $<$<PLATFORM_ID:Darwin>:-Wno-mismatched-tags>
                       $<$<PLATFORM_ID:Darwin>:-Wno-infinite-recursion>)

add_compile_options(-fvisibility=hidden
        -fvisibility-inlines-hidden
        $<$<PLATFORM_ID:Darwin>:-Werror>
        $<$<PLATFORM_ID:Darwin>:-Wno-unused-command-line-argument>
        $<$<CONFIG:Debug>:-DSURGEXT_RACK_DEBUG>)

add_library(${PROJECT_NAME} STATIC
        src/Delay.cpp
        src/DelayLineByFreq.cpp
        src/EGxVCA.cpp
        src/FX.cpp
        src/LFO.cpp
        src/Mixer.cpp
        src/ModMatrix.cpp
        src/QuadAD.cpp
        src/QuadLFO.cpp
        src/VCF.cpp
        src/VCO.cpp
        src/Waveshaper.cpp
        src/XTModule.cpp
        src/XTModuleWidget.cpp
        src/XTStyle.cpp
        )

target_link_libraries(${PROJECT_NAME} PRIVATE surge-common)

add_library(${LIB_NAME} MODULE src/SurgeXT.cpp)
target_link_libraries(${LIB_NAME} PRIVATE ${PROJECT_NAME} surge-common juce_dsp_rack_sub tinyxml strnatcmp
  samplerate sst-plugininfra fmt sqlite eurorack airwindows)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(${LIB_NAME} PROPERTIES SUFFIX ".dylib")
  set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif ()

add_custom_target(${PROJECT_NAME}_quick_install
        COMMAND cmake -E copy ${CMAKE_BINARY_DIR}/${LIB_NAME}.* ${CMAKE_INSTALL_PREFIX}/SurgeXTRack
        COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/plugin.json ${CMAKE_INSTALL_PREFIX}/SurgeXTRack
        )
add_dependencies(${PROJECT_NAME}_quick_install ${PROJECT_NAME})
add_dependencies(${PROJECT_NAME}_quick_install ${LIB_NAME})

set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_libraries(${LIB_NAME} PRIVATE winmm uuid wsock32 shlwapi version wininet ole32 ws2_32)
else ()
  target_link_libraries(${LIB_NAME} PRIVATE filesystem)
  install(TARGETS filesystem ARCHIVE DESTINATION lib/static)
endif ()

install(TARGETS ${LIB_NAME} ${PROJECT_NAME} surge-common juce_dsp_rack_sub tinyxml strnatcmp samplerate
        sst-plugininfra fmt sqlite eurorack airwindows
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

file(GLOB LICENSE LICENSE*)
file(INSTALL plugin.json res docs patches presets ${LICENSE} README.md DESTINATION ${PLUGIN_NAME})
file(INSTALL surge/resources/surge-shared/configuration.xml
             surge/resources/surge-shared/windows.wt
             surge/resources/data/wavetables
             surge/resources/data/wavetables_3rdparty
             surge/resources/data/fx_presets
             res/surge_extra_data/fx_presets
    DESTINATION ${PLUGIN_NAME}/build/surge-data)

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/${PLUGIN_NAME} OPTIONAL)
install(DIRECTORY ${PROJECT_BINARY_DIR}/${PLUGIN_NAME}/ DESTINATION ${PLUGIN_NAME})
