#pragma once

#include "rack.hpp"
#include <set>

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

#define MAX_POLY 16

// https://vcvrack.com/manual/VoltageStandards.html
#define RACK_CV_MAX_LEVEL 10
#define SURGE_CV_MAX_LEVEL 1

#define RACK_OSC_MAX_LEVEL 5
#define SURGE_OSC_PEAK_TO_PEAK 2
#define SURGE_OSC_MAX_LEVEL 1

#define SURGE_TO_RACK_OSC_MUL 5
#define RACK_TO_SURGE_OSC_MUL 0.2

#define SURGE_TO_RACK_CV_MUL 10
#define RACK_TO_SURGE_CV_MUL 0.1

extern rack::Plugin *pluginInstance;

extern rack::Model *modelSurgeClock;
extern rack::Model *modelSurgeADSR;
extern rack::Model *modelSurgeLFO;

extern rack::Model *modelSurgeOSC;
extern rack::Model *modelSurgeOSCModern;
extern rack::Model *modelSurgeOSCAlias;
extern rack::Model *modelSurgeOSCString;
extern rack::Model *modelSurgeWTOSC;
extern rack::Model *modelSurgeNoise;
extern rack::Model *modelSurgeWaveShaper;

extern rack::Model *modelSurgePatchPlayer;

extern rack::Model *modelSurgeVCF;
extern rack::Model *modelSurgeBiquad;


extern int addFX(rack::Model *, int type);
