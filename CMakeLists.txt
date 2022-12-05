#cmake_minimum_required (VERSION 3.20)
cmake_minimum_required (VERSION 3.16)
project (surge-rack VERSION 1.0 LANGUAGES C CXX)
include (RackSDK.cmake)
set (CMAKE_CXX_STANDARD 17)

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
set (NO_WARN_MAYBE_UNINITIALIZED -Wno-maybe-uninitialized)
set (NO_WARN_STRINGOP_TRUNCATION -Wno-stringop-truncation)
set (NO_WARN_CLASS_MEMACCESS -Wno-class-memaccess)
set (NO_WARN_INFINITE_RECURSION "")
set (NO_WARN_UNUSED_PRIVATE_FIELD "")
set (NO_WARN_UNUSED_LAMBDA_CAPTURE "")
set (NO_WARN_MISMATCHED_TAGS "")
set (NO_WARN_SELF_ASSIGN "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (NO_WARN_MAYBE_UNINITIALIZED -Wno-uninitialized)
  set (NO_WARN_STRINGOP_TRUNCATION -Wno-string-concatenation)
  set (NO_WARN_CLASS_MEMACCESS -Wno-class-varargs)
  set (NO_WARN_INFINITE_RECURSION -Wno-infinite-recursion)
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
  -Wno-unused-but-set-variable ${NO_WARN_STRINGOP_TRUNCATION} ${NO_WARN_CLASS_MEMACCESS} ${NO_WARN_SELF_ASSIGN}
  ${NO_WARN_UNUSED_LAMBDA_CAPTURE} ${NO_WARN_MAYBE_UNINITIALIZED} ${NO_WARN_MISMATCHED_TAGS} ${NO_WARN_INFINITE_RECURSION})

file (GLOB RACK_SOURCES src/*.cpp)
list (REMOVE_ITEM RACK_SOURCES src/SurgeXT.cpp)

add_library (${PROJECT_NAME} STATIC ${RACK_SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
target_link_libraries (${PROJECT_NAME} PRIVATE surge-common)

install(TARGETS surge-common juce_dsp_rack_sub tinyxml strnatcmp sst-plugininfra fmt sqlite oddsound-mts eurorack airwindows
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

#add_library (${LIB_NAME} MODULE ${RACK_SOURCES})
#set_target_properties (${LIB_NAME} PROPERTIES PREFIX "")
#target_compile_definitions (${LIB_NAME} PRIVATE SURGE_COMPILE_BLOCK_SIZE=8)
#target_compile_options(${LIB_NAME} PRIVATE "-fuse-ld=gold" "-flto")
#target_link_directories(${LIB_NAME} PRIVATE ${RACK_SDK_DIR})
#target_link_libraries (${LIB_NAME} PRIVATE surge-common Rack)
#target_include_directories (${LIB_NAME} PRIVATE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
        #target_include_directories (plugin PRIVATE surge/src/common surge/src/common/dsp src/common/dsp/vembertech
#  surge/libs/sst/sst-filters/include surge/libs/sst/sst-cpputils/include surge/libs/sst/sst-plugininfra/include
#  surge/libs/sst/sst-waveshapers/include surge/libs/tuning-library/include
#  surge/libs/sst/sst-plugininfra/libs/tinyxml/include
#  ${CMAKE_CURRENT_BINARY_DIR}/libs/sst/sst-plugininfra/libs/filesystem/include
#  ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
#target_link_libraries (plugin PRIVATE ${PROJECT_NAME})

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
