#pragma once

#include "rack.hpp"

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

#ifndef RACK_V1
#define INFO(format, ...)                                                      \
    loggerLog(rack::INFO_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#endif

extern rack::Plugin *pluginInstance;
extern rack::Model *modelSurgeFX;
extern rack::Model *modelSurgeADSR;
extern rack::Model *modelSurgeOSC;
extern rack::Model *modelSurgeWaveShaper;
