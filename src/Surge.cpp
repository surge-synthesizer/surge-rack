#include "Surge.hpp"
#include "SurgeStorage.h"

namespace logger = rack::logger;

rack::Model **fxModels = nullptr;
int addFX(rack::Model *m, int type)
{
    if (fxModels == nullptr)
    {
        fxModels = new rack::Model *[n_fx_types];
        for (auto i = 0; i < n_fx_types; ++i)
            fxModels[i] = nullptr;
    }
    fxModels[type] = m;

    return type;
}

rack::Plugin *pluginInstance;

__attribute__((__visibility__("default"))) void init(rack::Plugin *p)
{
    pluginInstance = p;
    INFO("[SurgeXTRack] initializing");

    p->addModel(modelSurgeOSCClassic);
    p->addModel(modelSurgeOSCModern);
    p->addModel(modelSurgeOSCWavetable);
    p->addModel(modelSurgeOSCWindow);
    p->addModel(modelSurgeOSCSine);
    p->addModel(modelSurgeOSCFM2);
    p->addModel(modelSurgeOSCFM3);
    p->addModel(modelSurgeOSCSHNoise);
    p->addModel(modelSurgeOSCAlias);
    p->addModel(modelSurgeOSCString);

#if 0
    p->addModel(modelSurgeClock);
    p->addModel(modelSurgeADSR);

    p->addModel(modelSurgeOSC);
    p->addModel(modelSurgeWTOSC);
    p->addModel(modelSurgeNoise);
    p->addModel(modelSurgeWaveShaper);

    p->addModel(modelSurgeLFO);

#endif

    // Add the ported ones
    p->addModel(modelSurgeVCF);

#if 0
    if (fxModels != nullptr)
        for (auto i = 0; i < n_fx_types; ++i)
        {
            if (fxModels[i] != nullptr)
            {
                p->addModel(fxModels[i]);
            }
        }
#endif
}

#if OLD_PLUGIN_JSON
// What it used to be
{
    "slug" : "SurgeXTDelay",
    "name" : "SurgeXTDelay",
    "description" : "The Surge stereo delay effect",
    "tags" : [ "Effect", "Polyphonic", "Delay" ]
},
    {
        "slug" : "SurgeXTChorus",
        "name" : "SurgeXTChorus",
        "description" : "The Surge chorus effect",
        "tags" : [ "Effect", "Polyphonic", "Chorus" ]
    },
    {
        "slug" : "SurgeXTEnsemble",
        "name" : "SurgeXTEnsemble",
        "description" : "The Surge ensemble effect",
        "tags" : [ "Effect", "Polyphonic", "Chorus" ]
    },
    {
        "slug" : "SurgeXTEQ",
        "name" : "SurgeXTEQ",
        "description" : "The Surge 3-band equalizer",
        "tags" : [ "Effect", "Polyphonic", "Equalizer" ]
    },
    {
        "slug" : "SurgeXTPhaser",
        "name" : "SurgeXTPhaser",
        "description" : "The Surge phaser",
        "tags" : [ "Effect", "Polyphonic", "Phaser" ]
    },
    {
        "slug" : "SurgeXTRotary",
        "name" : "SurgeXTRotary",
        "description" : "The Surge rotary speaker emulation effect",
        "tags" : [ "Effect", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTDistort",
        "name" : "SurgeXTDistort",
        "description" : "The Surge distortion effect",
        "tags" : [ "Effect", "Polyphonic", "Distortion", "Waveshaper" ]
    },
    {
        "slug" : "SurgeXTReverb",
        "name" : "SurgeXTReverb",
        "description" : "The Surge reverb",
        "tags" : [ "Effect", "Polyphonic", "Reverb" ]
    },
    {
        "slug" : "SurgeXTReverb2",
        "name" : "SurgeXTReverb2",
        "description" : "The Surge reverb (second one)",
        "tags" : [ "Effect", "Polyphonic", "Reverb" ]
    },
    {
        "slug" : "SurgeXTFreqShift",
        "name" : "SurgeXTFreqShift",
        "description" : "The Surge frequency shifter",
        "tags" : [ "Effect", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTConditioner",
        "name" : "SurgeXTConditioner",
        "description" : "The Surge conditioner/limiter effect",
        "tags" : [ "Effect", "Polyphonic", "Limiter", "EQ" ]
    },
    {
        "slug" : "SurgeXTFlanger",
        "name" : "SurgeXTFlanger",
        "description" : "The Surge flanger effect",
        "tags" : [ "Effect", "Polyphonic", "Flanger" ]
    },
    {
        "slug" : "SurgeXTRingMod",
        "name" : "SurgeXTRingMod",
        "description" : "The Surge ring modulator effect",
        "tags" : [ "Effect", "Polyphonic", "Ring Modulator" ]
    },
    {
        "slug" : "SurgeXTChow",
        "name" : "SurgeXTChow",
        "description" : "The Surge CHOW Distortion effect",
        "tags" : [ "Effect", "Polyphonic", "Distortion" ]
    },
    {
        "slug" : "SurgeXTResonator",
        "name" : "SurgeXTResonator",
        "description" : "The Surge Resonator Model Effect",
        "tags" : [ "Effect", "Polyphonic", "Filter" ]
    },
    {
        "slug" : "SurgeXTExciter",
        "name" : "SurgeXTExciter",
        "description" : "The Surge Exciter Effect",
        "tags" : [ "Effect", "Polyphonic", "Filter" ]
    },
    {
        "slug" : "SurgeXTCombulator",
        "name" : "SurgeXTCombulator",
        "description" : "The Surge Comb Filter Bank Effect",
        "tags" : [ "Effect", "Polyphonic", "Filter" ]
    },
    {
        "slug" : "SurgeXTTreeMonster",
        "name" : "SurgeXTTreeMonster",
        "description" : "The Surge TreeMonster Effect. Watch out!",
        "tags" : [ "Effect", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTClock",
        "name" : "SurgeXTClock",
        "description" : "A utility clock generator",
        "tags" : ["Clock Generator"]
    },
    {
        "slug" : "SurgeXTADSR",
        "name" : "SurgeXTADSR",
        "description" : "The Surge ADSR envelope generator with various modes",
        "tags" : [ "Envelope generator", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTOSC",
        "name" : "SurgeXTOSC",
        "description" : "The Surge classic oscillators, including square, saw, sine, FM and Noise",
        "tags" : [ "VCO", "Polyphonic" ]
    },

    {
        "slug" : "SurgeXTWaveShaper",
        "name" : "SurgeXTWaveShaper",
        "description" : "The Surge waveshapers",
        "tags" : [ "Waveshaper", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTWTOSC",
        "name" : "SurgeXTWTOSC",
        "description" : "The Surge WaveTable collection with a pair of interpolating oscillators",
        "tags" : [ "VCO", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTNoise",
        "name" : "SurgeXTNoise",
        "description" : "The Surge correlated noise channel",
        "tags" : ["Noise"]
    },
    {
        "slug" : "SurgeXTLFO",
        "name" : "SurgeXTLFO",
        "description" : "The Surge LFO",
        "tags" : [ "LFO", "Polyphonic" ]
    },
    {
        "slug" : "SurgeXTVOC",
        "name" : "SurgeXTVOC",
        "description" : "The Surge vocoder",
        "tags" : [ "Vocoder", "Polyphonic" ]
    },
#endif