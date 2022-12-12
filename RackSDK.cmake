# Mapping of plugin build definitions from the Rack-SDK arch.mk, compile.mk, dep.mk and plugin.mk to CMake.

set(RACK_SDK_VERSION 2.2.1)
message(STATUS "Load RackSDK.cmake (mapping based on Rack-SDK-${RACK_SDK_VERSION})")

if ("${PLUGIN_NAME}" STREQUAL "")
  message(FATAL_ERROR "PLUGIN_NAME variable not set! Add PLUGIN_NAME variable to the project CMakeLists before including RackSDK.cmake.")
else ()
  message(STATUS "Using PLUGIN_NAME '${PLUGIN_NAME}'")
endif ()

if ("${PLUGIN_DISTRIBUTABLES}" STREQUAL "")
  message(WARNING "PLUGIN_DISTRIBUTABLES variable not set. CMake will not install the PLUGIN_DISTRIBUTABLES into '${PLUGIN_NAME}' dir.")
endif ()

# Do not change the RACK_PLUGIN_LIB!
set(RACK_PLUGIN_LIB plugin)

# This is needed for Rack for DAWs.
# Static libs don't usually compiled with -fPIC, but since we're including them in a shared library, it's needed.
add_compile_options(-fPIC)
# Generate dependency files alongside the object files
# Not required with CMake
#add_compile_options(-MMD -MP)

# Debugger symbols. These are removed with `strip`.
add_compile_options(-g)
# Optimization
add_compile_options(-O3 -funsafe-math-optimizations -fno-omit-frame-pointer)
# Warnings
add_compile_options(-Wall -Wextra -Wno-unused-parameter)
# C++ standard
set(CMAKE_CXX_STANDARD 11)

add_library(${RACK_PLUGIN_LIB} MODULE)
set_target_properties(${RACK_PLUGIN_LIB} PROPERTIES PREFIX "")

# Since the plugin's compiler could be a different version than Rack's compiler, link libstdc++ and libgcc statically to avoid ABI issues.
add_link_options($<$<CXX_COMPILER_ID:GNU>:-static-libstdc++> $<$<PLATFORM_ID:Linux>:-static-libgcc>)
add_compile_options($<IF:$<STREQUAL:${CMAKE_OSX_ARCHITECTURES},arm64>,-march=armv8-a+fp+simd,-march=nehalem>)

add_library(RackSDK INTERFACE)
target_include_directories(RackSDK INTERFACE ${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
target_link_directories(RackSDK INTERFACE ${RACK_SDK_DIR})
target_link_libraries(RackSDK INTERFACE Rack)
target_compile_definitions(RackSDK INTERFACE $<IF:$<STREQUAL:${CMAKE_OSX_ARCHITECTURES},arm64>,ARCH_ARM64,ARCH_X64>)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (NOT MINGW)
    message(FATAL_ERROR "Rack plugin development environment is only supported for MSYS2/MinGW")
  endif ()
  target_compile_definitions(RackSDK INTERFACE ARCH_WIN _USE_MATH_DEFINES)
  # disabled - causes surge-rack/surge/libs/sst/sst-plugininfra/src/misc_windows.cpp:20:20: error: cannot convert 'const char*' to 'LPCWSTR' {aka 'const wchar_t*'}
  # see also https://gcc.gnu.org/onlinedocs/gcc/x86-Windows-Options.html
  #add_compile_options(-municode)
  target_compile_options(RackSDK INTERFACE -Wsuggest-override)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  message(STATUS "Build Mac OSX Plugin for architecture ${CMAKE_OSX_ARCHITECTURES}")
  target_compile_definitions(RackSDK INTERFACE ARCH_MAC)
  if (${CMAKE_OSX_ARCHITECTURES} MATCHES "x86_64")
    add_compile_options(-arch x86_64)
  endif ()
  if (${CMAKE_OSX_ARCHITECTURES} MATCHES "arm64")
    add_compile_options(-arch arm64)
    add_compile_options(-faligned-allocation)
  endif ()
  set_target_properties(${RACK_PLUGIN_LIB} PROPERTIES SUFFIX ".dylib")
  set_target_properties(${RACK_PLUGIN_LIB} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_compile_definitions(RackSDK INTERFACE ARCH_LIN)
  # This prevents static variables in the DSO (dynamic shared object) from being preserved after dlclose().
  target_compile_options(RackSDK INTERFACE -fno-gnu-unique)
  # When Rack loads a plugin, it symlinks /tmp/Rack2 to its system dir, so the plugin can link to libRack.
  target_compile_options(RackSDK INTERFACE -Wl,-rpath=/tmp/Rack2)
endif ()

target_link_libraries(${RACK_PLUGIN_LIB} PRIVATE ${RackSDK})

install(TARGETS ${RACK_PLUGIN_LIB} LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/${PLUGIN_NAME} OPTIONAL)
install(DIRECTORY ${PROJECT_BINARY_DIR}/${PLUGIN_NAME}/ DESTINATION ${PLUGIN_NAME})
file(INSTALL ${PLUGIN_DISTRIBUTABLES} DESTINATION ${PLUGIN_NAME})
