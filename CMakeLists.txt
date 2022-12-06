cmake_minimum_required (VERSION 3.16)
project (surge-rack LANGUAGES C CXX)
include (RackSDK.cmake)
set (CMAKE_CXX_STANDARD 17)

set (PLUGIN_NAME SurgeXTRack)
## check for Windows non gcc env in RackSDK.cmake!!!

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
set (NO_WARN_UNUSED_BUT_SET_VARIABLE "-Wno-unused-but-set-variable")
set (NO_WARN_MAYBE_UNINITIALIZED -Wno-maybe-uninitialized)
set (NO_WARN_STRINGOP_TRUNCATION -Wno-stringop-truncation)
set (NO_WARN_CLASS_MEMACCESS -Wno-class-memaccess)
set (NO_WARN_INFINITE_RECURSION "")
set (NO_WARN_UNUSED_PRIVATE_FIELD "")
set (NO_WARN_UNUSED_LAMBDA_CAPTURE "")
set (NO_WARN_MISMATCHED_TAGS "")
set (NO_WARN_SELF_ASSIGN "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (NO_WARN_UNUSED_BUT_SET_VARIABLE "-Wno-unused-const-variable")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set (NO_WARN_UNUSED_BUT_SET_VARIABLE "-Wno-unused-but-set-variable")
  endif ()
  set (NO_WARN_MAYBE_UNINITIALIZED -Wno-uninitialized)
  set (NO_WARN_STRINGOP_TRUNCATION -Wno-string-concatenation)
  set (NO_WARN_CLASS_MEMACCESS -Wno-class-varargs)
  set (NO_WARN_INFINITE_RECURSION -Wno-infinite-recursion)
  set (NO_WARN_UNUSED_BUT_SET_VARIABLE "-Wno-unused-const-variable")
  set (NO_WARN_UNUSED_PRIVATE_FIELD -Wno-unused-private-field)
  set (NO_WARN_UNUSED_LAMBDA_CAPTURE -Wno-unused-lambda-capture)
  set (NO_WARN_MISMATCHED_TAGS -Wno-mismatched-tags)
  set (NO_WARN_SELF_ASSIGN -Wno-self-assign)
endif ()

target_compile_options (juce_dsp_rack_sub PRIVATE -Wno-deprecated-copy ${NO_WARN_MAYBE_UNINITIALIZED})
target_compile_options (eurorack PRIVATE -Wno-unused-local-typedefs ${NO_WARN_INFINITE_RECURSION})
target_compile_options (oddsound-mts PRIVATE -Wno-char-subscripts)
target_compile_options (sqlite PRIVATE -Wno-error)
target_compile_options (sst-plugininfra PRIVATE -Wno-unused-variable)
target_compile_options (sst-waveshapers INTERFACE -Wno-unused-variable)
target_compile_options (tinyxml PRIVATE -Wno-implicit-fallthrough -Wno-sign-compare)
target_compile_options (tuning-library INTERFACE ${NO_WARN_MAYBE_UNINITIALIZED} ${NO_WARN_UNUSED_PRIVATE_FIELD})
target_compile_options (surge-common PRIVATE -Wno-sign-compare -Wno-ignored-qualifiers -Wno-unused-value -Wno-uninitialized
  -Wno-implicit-fallthrough -Wno-char-subscripts -Wno-unused-local-typedefs -Wno-range-loop-construct -Wno-nonnull -Wno-reorder
  ${NO_WARN_UNUSED_BUT_SET_VARIABLE} ${NO_WARN_STRINGOP_TRUNCATION} ${NO_WARN_CLASS_MEMACCESS} ${NO_WARN_SELF_ASSIGN}
  ${NO_WARN_UNUSED_LAMBDA_CAPTURE} ${NO_WARN_MAYBE_UNINITIALIZED} ${NO_WARN_MISMATCHED_TAGS} ${NO_WARN_INFINITE_RECURSION})

file (GLOB RACK_SOURCES src/*.cpp)
list (REMOVE_ITEM RACK_SOURCES src/SurgeXT.cpp)

add_library (${PROJECT_NAME} STATIC ${RACK_SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
target_link_libraries (${PROJECT_NAME} PRIVATE surge-common)

# Do not change the LIB_NAME!
set(LIB_NAME plugin)
add_library (${LIB_NAME} MODULE src/SurgeXT.cpp)
target_include_directories (${LIB_NAME} PRIVATE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
target_link_directories(${LIB_NAME} PRIVATE ${RACK_SDK_DIR})
target_compile_options(${LIB_NAME} PRIVATE "-fuse-ld=gold" "-flto")
target_link_libraries (${LIB_NAME} PRIVATE Rack)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(${LIB_NAME} PROPERTIES SUFFIX ".dylib")
  set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif ()
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_directories(${LIB_NAME} PRIVATE ${RACK_SDK_DIR})
  target_link_libraries (${LIB_NAME} PRIVATE ${PROJECT_NAME} surge-common juce_dsp_rack_sub tinyxml strnatcmp
    sst-plugininfra fmt sqlite oddsound-mts eurorack airwindows winmm uuid wsock32 shlwapi version wininet ole32 ws2_32)
endif ()

install(TARGETS ${LIB_NAME} surge-common juce_dsp_rack_sub tinyxml strnatcmp sst-plugininfra fmt sqlite oddsound-mts eurorack airwindows
        RUNTIME DESTINATION bin
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
