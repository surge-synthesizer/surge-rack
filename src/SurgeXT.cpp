#include "SurgeXT.hpp"
#include "SurgeStorage.h"
#include "XTStyle.hpp"

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

    p->addModel(modelFXReverb2);
    p->addModel(modelFXFrequencyShifter);
    p->addModel(modelFXFlanger);
    p->addModel(modelFXDelay);
    p->addModel(modelFXSpringReverb);

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
    sst::surgext_rack::style::XTStyle::initialize();
}
