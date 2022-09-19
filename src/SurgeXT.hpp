#pragma once

#include "rack.hpp"
#include <set>

// FIXME - remove these eventually
#define SCREW_WIDTH rack::app::RACK_GRID_WIDTH
#define RACK_HEIGHT rack::app::RACK_GRID_HEIGHT

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

// VCOs
extern rack::Model *modelVCOClassic;
extern rack::Model *modelVCOModern;
extern rack::Model *modelVCOWavetable;
extern rack::Model *modelVCOWindow;
extern rack::Model *modelVCOSine;
extern rack::Model *modelVCOFM2;
extern rack::Model *modelVCOFM3;
extern rack::Model *modelVCOSHNoise;
extern rack::Model *modelVCOAlias;
extern rack::Model *modelVCOString;
extern rack::Model *modelVCOTwist;

extern rack::Model *modelSurgeVCF;
extern rack::Model *modelSurgeWaveshaper;

// FX
extern rack::Model *modelFXReverb;
extern rack::Model *modelFXReverb2;
extern rack::Model *modelFXFrequencyShifter;
extern rack::Model *modelFXFlanger;
extern rack::Model *modelFXDelay;
extern rack::Model *modelFXSpringReverb;