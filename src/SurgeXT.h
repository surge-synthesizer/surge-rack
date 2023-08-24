/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_SRC_SURGEXT_H
#define SURGE_XT_RACK_SRC_SURGEXT_H

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
extern rack::Model *modelSurgeDelay;
extern rack::Model *modelSurgeDelayLineByFreq;
extern rack::Model *modelSurgeDelayLineByFreqExpanded;
extern rack::Model *modelSurgeWaveshaper;
extern rack::Model *modelSurgeLFO;
extern rack::Model *modelSurgeMixer;
extern rack::Model *modelSurgeModMatrix;
extern rack::Model *modelSurgeDigitalRingMods;

// FX

extern rack::Model *modelFXReverb;
extern rack::Model *modelFXPhaser;
extern rack::Model *modelFXRotarySpeaker;
extern rack::Model *modelFXDistortion;
extern rack::Model *modelFXFrequencyShifter;
extern rack::Model *modelFXChorus;
extern rack::Model *modelFXVocoder;
extern rack::Model *modelFXReverb2;
extern rack::Model *modelFXFlanger;
extern rack::Model *modelFXRingMod;
extern rack::Model *modelFXNeuron;
extern rack::Model *modelFXResonator;
extern rack::Model *modelFXChow;
extern rack::Model *modelFXExciter;
extern rack::Model *modelFXEnsemble;
extern rack::Model *modelFXCombulator;
extern rack::Model *modelFXSpringReverb;
extern rack::Model *modelFXTreeMonster;
extern rack::Model *modelFXBonsai;
extern rack::Model *modelFXNimbus;

extern rack::Model *modelEGxVCA;
extern rack::Model *modelQuadAD;
extern rack::Model *modelQuadLFO;

extern rack::Model *modelUnisonHelper;

#endif // SCXT_SRC_SURGEXT_H
