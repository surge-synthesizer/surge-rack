#pragma once

#include "rack.hpp"
#include <set>

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

extern rack::Plugin *pluginInstance;

extern rack::Model *modelSurgeClock;
extern rack::Model *modelSurgeADSR;

extern rack::Model *modelSurgeOSC;
extern rack::Model *modelSurgeWaveShaper;

extern rack::Model *modelSurgeWTOSC;
extern rack::Model *modelSurgeVCF;
extern rack::Model *modelSurgeLFO;

extern int addFX(rack::Model *, int type);
