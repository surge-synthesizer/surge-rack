#pragma once

#include "rack.hpp"
#include <set>

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

#ifndef RACK_V1
#define INFO(format, ...)                                                      \
    loggerLog(rack::INFO_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#endif


extern rack::Plugin *pluginInstance;


extern std::set<rack::Model *> modelSurgeFXSet;

extern rack::Model *modelSurgeADSR;
extern rack::Model *modelSurgeOSC;
extern rack::Model *modelSurgeWaveShaper;
extern rack::Model *modelSurgeClock;
extern rack::Model *modelSurgeWTOSC;
extern rack::Model *modelSurgeVCF;
extern rack::Model *modelSurgeLFO;
extern rack::Model *modelSurgeVOC;
