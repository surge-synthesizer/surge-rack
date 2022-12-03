message("RackSDK.cmake loaded!")

# default flags from CMake
#  -O3 -DNDEBUG -fPIC

add_definitions(-funsafe-math-optimizations -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-parameter)

message (STATUS "CMAKE_SYSTEM_NAME - ${CMAKE_SYSTEM_NAME}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message (STATUS "CMAKE SYSTEM is Linux")
  add_definitions(-DARCH_LIN=1)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  message (STATUS "CMAKE SYSTEM is Windows")
  add_definitions(-DARCH_WIN=1)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  message (STATUS "CMAKE SYSTEM is Darwin/MacOS")
  add_definitions(-DARCH_MAC=1 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9)
endif ()




#  -DARCH_LIN=1 -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
#  -DJUCE_MODULE_AVAILABLE_juce_audio_basics=1
#  -DJUCE_MODULE_AVAILABLE_juce_core=1
#  -DJUCE_MODULE_AVAILABLE_juce_dsp=1
#  -DJUCE_MODULE_VAILABLE_juce_audio_formats=1
#  -DJUCE_STANDALONE_APPLICATION=0
#  DJUCE_USE_CURL=0
#  -DJUCE_WEB_BROWSER=0
#  -DNDEBUG=1
#  -I/home/user/Downloads/surge-rack/surge/src/common/../../libs/JUCE/modules
#  -O3 -DNDEBUG -fPIC
#  -funsafe-math-optimizations -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-parameter -funsafe-math-optimizations
#  -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-parameter -Wno-multichar -Werror
#  -fvisibility=hidden
#  -fvisibility-inlines-hidden -Wformat-truncation=0 -Wno-free-nonheap-object -Wno-return-local-addr
#  -std=c++1z -MD -MT
#
#  surge/src/common/CMakeFiles/juce_dsp_rack_sub.dir/__/__/libs/JUCE/modules/juce_audio_formats/juce_audio_formats.cpp.o -MF surge/src/common/CMakeFiles/juce_dsp_rack_sub.dir/__/__/libs/JUCE/modules/juce_audio_formats/juce_audio_formats.cpp.o.d -o surge/src/common/CMakeFiles/juce_dsp_rack_sub.dir/__/__/libs/JUCE/modules/juce_audio_formats/juce_audio_formats.cpp.o
#  -c /home/user/Downloads/surge-rack/surge/libs/JUCE/modules/juce_audio_formats/juce_audio_formats.cpp