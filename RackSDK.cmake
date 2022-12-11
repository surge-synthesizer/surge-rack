# Mapping of plugin build definitions from the Rack-SDK arch.mk, compile.mk, dep.mk and plugin.mk to CMake.

set(RACK_SDK_VERSION 2.2.1)
message(STATUS "load RackSDK.cmake (mapping based on Rack-SDK-${RACK_SDK_VERSION})")

include_directories(${RACK_SDK_DIR}/include ${RACK_SDK_DIR}/dep/include)
link_directories(${RACK_SDK_DIR})
link_libraries(Rack)

# This is needed for Rack for DAWs.
# Static libs don't usually compiled with -fPIC, but since we're including them in a shared library, it's needed.
add_compile_options(-fPIC)
# Generate dependency files alongside the object files
add_compile_options($<$<NOT:$<CXX_COMPILER_ID:AppleClang>>:-MMD>)
add_compile_options($<$<NOT:$<CXX_COMPILER_ID:AppleClang>>:-MP>)
# Debugger symbols. These are removed with `strip`.
add_compile_options(-g)
# Optimization
add_compile_options(-O3 -funsafe-math-optimizations -fno-omit-frame-pointer)
# Warnings
add_compile_options(-Wall -Wextra -Wno-unused-parameter)
# C++ standard
set(CMAKE_CXX_STANDARD 11)

# Since the plugin's compiler could be a different version than Rack's compiler, link libstdc++ and libgcc statically to avoid ABI issues.
add_link_options($<$<CXX_COMPILER_ID:GNU>:-static-libstdc++> $<$<PLATFORM_ID:Linux>:-static-libgcc>)
add_compile_options($<IF:$<STREQUAL:${CMAKE_OSX_ARCHITECTURES},arm64>,-march=armv8-a+fp+simd,-march=nehalem>)
add_compile_definitions($<IF:$<STREQUAL:${CMAKE_OSX_ARCHITECTURES},arm64>,ARCH_ARM64,ARCH_X64>)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (NOT MINGW)
    message(FATAL_ERROR "Rack plugin development environment is only supported for MSYS2/MinGW")
  endif ()
  add_compile_definitions(ARCH_WIN)
  add_compile_definitions(_USE_MATH_DEFINES)
  # disabled - causes surge-rack/surge/libs/sst/sst-plugininfra/src/misc_windows.cpp:20:20: error: cannot convert 'const char*' to 'LPCWSTR' {aka 'const wchar_t*'}
  # see also https://gcc.gnu.org/onlinedocs/gcc/x86-Windows-Options.html
  #add_compile_options(-municode)
  add_compile_options(-Wsuggest-override)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  message(STATUS "Build Mac OSX Plugin for architecture ${CMAKE_OSX_ARCHITECTURES}")
  add_compile_definitions(ARCH_MAC)
  if (${CMAKE_OSX_ARCHITECTURES} MATCHES "x86_64")
    add_compile_options(-arch x86_64)
  endif ()
  if (${CMAKE_OSX_ARCHITECTURES} MATCHES "arm64")
    add_compile_options(-arch arm64)
    add_compile_options(-faligned-allocation)
  endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_compile_definitions(ARCH_LIN)
  # This prevents static variables in the DSO (dynamic shared object) from being preserved after dlclose().
  add_compile_options(-fno-gnu-unique)
  # When Rack loads a plugin, it symlinks /tmp/Rack2 to its system dir, so the plugin can link to libRack.
  add_compile_options(-Wl,-rpath=/tmp/Rack2)
endif ()
