/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "SurgeXT.h"
#include "XTStyle.h"

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
    p->addModel(modelSurgeDelayLineByFreqExpanded);
    p->addModel(modelSurgeWaveshaper);
    p->addModel(modelSurgeLFO);
    p->addModel(modelSurgeMixer);
    p->addModel(modelSurgeModMatrix);
    p->addModel(modelSurgeDigitalRingMods);

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
    p->addModel(modelFXTreeMonster);
    p->addModel(modelFXBonsai);
    p->addModel(modelFXNimbus);

    p->addModel(modelEGxVCA);
    p->addModel(modelQuadAD);
    p->addModel(modelQuadLFO);

    sst::surgext_rack::style::XTStyle::initialize();
}
