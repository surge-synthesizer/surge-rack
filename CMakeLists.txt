# This just exists so I can at least partly use clion on my mac. Ignore it.
cmake_minimum_required(VERSION 3.22)
project( surge-rack-on-my-mac )

message(STATUS "If you are using this and you are (1) not baconpaul and (2) not on an M1 you will be sad")
message(STATUS "But if you get the idea and want to change it so you are not sad, happy to take a PR!")

add_custom_target(surge-rack-clean
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND RACK_DIR=../Rack-SDK arch -x86_64 make clean
        COMMAND rm -rf dep
        )

add_custom_target(surge-rack-on-my-mac
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        USES_TERMINAL
        COMMAND RACK_DIR=../Rack-SDK arch -x86_64 make dep
        COMMAND RACK_DIR=../Rack-SDK arch -x86_64 make dist
        COMMAND cp ${CMAKE_SOURCE_DIR}/dist/SurgeRack*vcvplugin /Users/paul/Documents/Rack2/plugins
        )
