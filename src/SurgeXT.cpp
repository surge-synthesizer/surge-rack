#include "SurgeXT.hpp"
#include "SurgeStorage.h"
#include "XTStyle.hpp"

namespace logger = rack::logger;
rack::Plugin *pluginInstance;

__attribute__((__visibility__("default"))) void init(rack::Plugin *p)
{
    pluginInstance = p;
    INFO("[SurgeXTRack] initializing");

    p->addModel(modelVCOClassic);
    p->addModel(modelVCOModern);
    p->addModel(modelVCOWavetable);
    p->addModel(modelVCOWindow);
    p->addModel(modelVCOSine);
    p->addModel(modelVCOFM2);
    p->addModel(modelVCOFM3);
    p->addModel(modelVCOSHNoise);
    p->addModel(modelVCOAlias);
    p->addModel(modelVCOString);
    p->addModel(modelVCOTwist);

    // Add the ported ones
    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeDelay);
    p->addModel(modelSurgeDelayLineByFreq);
    p->addModel(modelSurgeWaveshaper);
    p->addModel(modelSurgeLFO);
    p->addModel(modelSurgeMixer);
    p->addModel(modelSurgeModMatrix);
    p->addModel(modelSurgeTreeMonster);

    p->addModel(modelFXReverb);
    p->addModel(modelFXPhaser);
    p->addModel(modelFXRotarySpeaker);
    p->addModel(modelFXDistortion);
    p->addModel(modelFXFrequencyShifter);
    p->addModel(modelFXChorus);
    p->addModel(modelFXVocoder);
    p->addModel(modelFXReverb2);
    p->addModel(modelFXFlanger);
    p->addModel(modelFXRingMod);
    p->addModel(modelFXNeuron);
    p->addModel(modelFXResonator);
    p->addModel(modelFXChow);
    p->addModel(modelFXExciter);
    p->addModel(modelFXEnsemble);
    p->addModel(modelFXCombulator);
    p->addModel(modelFXSpringReverb);

    sst::surgext_rack::style::XTStyle::initialize();
}
