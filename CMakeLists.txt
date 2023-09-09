cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(SurgeXTRack VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PLUGIN_NAME ${PROJECT_NAME})
set(ADDITIONAL_PLUGIN_DISTRIBUTABLES docs README.md)
include(RackSDK.cmake)

message(STATUS "SurgeXT for Rack Build Process" )
message(STATUS "Installing into '${CMAKE_INSTALL_PREFIX}'")

find_package(Git REQUIRED)
execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE SURGE_RACK_PLUG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/surge
        OUTPUT_VARIABLE SURGE_RACK_SURGE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "SURGE_RACK_PLUG_VERSION ${SURGE_RACK_PLUG_VERSION}")
message(STATUS "SURGE_RACK_SURGE_VERSION ${SURGE_RACK_SURGE_VERSION}")

add_subdirectory(libs/sst-rackhelpers)

set(SURGE_SKIP_JUCE_FOR_RACK TRUE CACHE BOOL "skip surge Juce build")
set(SURGE_SKIP_LUA TRUE CACHE BOOL "skip surge Lua build")
set(SURGE_SKIP_AIRWINDOWS TRUE CACHE BOOL "skip surge airwindows build")
set(SURGE_SKIP_ODDSOUND_MTS TRUE CACHE BOOL "skip surge oddsound-mts build")
set(SURGE_SKIP_VST3 TRUE CACHE BOOL "skip surge VST3 build")
set(SURGE_SKIP_ALSA TRUE CACHE BOOL "skip surge Alsa build")
set(SURGE_SKIP_STANDALONE TRUE CACHE BOOL "skip surge standalone build")
set(SURGE_COMPILE_BLOCK_SIZE 8 CACHE STRING "surge compile block size")

# Its super common to use RACK_SDK_DIR as a relative path, so resolve for simde
set(SURGE_SIMDE_PATH_INIT "${RACK_SDK_DIR}/dep/include" CACHE STRING "surge simde is rack simde")
get_filename_component(SURGE_SIMDE_PATH_ABS ${SURGE_SIMDE_PATH_INIT} ABSOLUTE)
set(SURGE_SIMDE_PATH "${SURGE_SIMDE_PATH_ABS}" CACHE STRING "surge simde is rack simde, resolved")

add_subdirectory(surge)
# turn off specific warning as errors enforced by Rack-SDK compiler options for surge libs
# the warnings should be fixed in the dependencies or the warning could be disabled in the
# dependency CMake definitions and removed from here
target_compile_options(juce_dsp_rack_sub PRIVATE -Wno-deprecated-copy
        $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-dangling-pointer>)
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
                       $<$<CXX_COMPILER_ID:GNU>:-Wno-restrict> # TODO - Remove this
                       $<$<PLATFORM_ID:Darwin>:-Wno-self-assign>
                       $<$<PLATFORM_ID:Darwin>:-Wno-unused-lambda-capture>
                       $<$<PLATFORM_ID:Darwin>:-Wno-mismatched-tags>
                       $<$<PLATFORM_ID:Darwin>:-Wno-unused-but-set-variable>
                       $<$<PLATFORM_ID:Darwin>:-Wno-infinite-recursion>)

target_compile_options(surge-common PRIVATE $<$<STREQUAL:${CMAKE_OSX_ARCHITECTURES},arm64>:-faligned-allocation>)

add_compile_options($<$<BOOL:${SURGE_XT_WARNINGS_ARE_ERRORS}>:-Werror> $<$<COMPILE_LANGUAGE:CXX>:-fno-char8_t>)
add_compile_definitions($<$<CONFIG:Debug>:SURGEXT_RACK_DEBUG>)

add_library(Surge INTERFACE)
target_compile_options(Surge INTERFACE $<$<STREQUAL:${CMAKE_OSX_ARCHITECTURES},arm64>:-faligned-allocation>)

set(SURGE_LIBS surge-common juce_dsp_rack_sub tinyxml strnatcmp samplerate sst-plugininfra fmt sqlite eurorack airwindows sst-basic-blocks)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(SURGE_WIN_LIBS winmm wsock32 shlwapi version wininet ws2_32)
else ()
  set(SURGE_LIBS ${SURGE_LIBS} filesystem)
endif ()

target_link_libraries(Surge INTERFACE ${SURGE_LIBS} ${SURGE_WIN_LIBS})

target_sources(${RACK_PLUGIN_LIB} PRIVATE
        src/SurgeXT.cpp
        src/Delay.cpp
        src/DelayLineByFreq.cpp
        src/DelayLineByFreqExpanded.cpp
        src/DigitalRingMod.cpp
        src/EGxVCA.cpp
        src/FX.cpp
        src/LFO.cpp
        src/Mixer.cpp
        src/ModMatrix.cpp
        src/QuadAD.cpp
        src/QuadLFO.cpp
        src/UnisonHelper.cpp
        src/VCF.cpp
        src/VCO.cpp
        src/Waveshaper.cpp
        src/XTModule.cpp
        src/XTModuleWidget.cpp
        src/XTStyle.cpp
        src/XTWidgets.cpp)

target_link_libraries(${RACK_PLUGIN_LIB} PRIVATE Surge sst-rackhelpers)
target_compile_options(${RACK_PLUGIN_LIB} PUBLIC -fno-char8_t)

file(COPY surge/resources/surge-shared/configuration.xml
             surge/resources/surge-shared/windows.wt
             surge/resources/data/wavetables
             surge/resources/data/fx_presets
             res/surge_extra_data/fx_presets
    DESTINATION ${PLUGIN_NAME}/build/surge-data)

file(COPY
        surge/resources/data/wavetables_3rdparty
        DESTINATION ${PLUGIN_NAME}_ExtraContent)

add_custom_target(xt-rack-code-checks)

# Clang Format checks
find_program(CLANG_FORMAT_EXE NAMES clang-format-12 clang-format)
set(CLANG_FORMAT_DIRS src)
set(CLANG_FORMAT_EXTS cpp h)
foreach(dir ${CLANG_FORMAT_DIRS})
  foreach(ext ${CLANG_FORMAT_EXTS})
    list(APPEND CLANG_FORMAT_GLOBS "':(glob)${dir}/**/*.${ext}'")
  endforeach()
endforeach()
add_custom_command(TARGET xt-rack-code-checks
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo About to check clang-format using ${CLANG_FORMAT_EXE}
        COMMAND git ls-files -- ${CLANG_FORMAT_GLOBS} | xargs ${CLANG_FORMAT_EXE} --dry-run --Werror
        )
# }}}

add_custom_target(xt-rack-extra-content)
add_custom_command(TARGET xt-rack-extra-content
  POST_BUILD
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND tar cf ${PLUGIN_NAME}_ExtraContent.tar ${PLUGIN_NAME}_ExtraContent
  COMMAND zstd -zf ${PLUGIN_NAME}_ExtraContent.tar
)
