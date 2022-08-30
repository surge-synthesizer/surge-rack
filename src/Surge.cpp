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
    INFO("[SurgeRack] initializing");

    p->addModel(modelSurgeClock);
    p->addModel(modelSurgeADSR);

    p->addModel(modelSurgeOSC);
    p->addModel(modelSurgeOSCModern);
    p->addModel(modelSurgeOSCAlias);
    p->addModel(modelSurgeOSCString);
    p->addModel(modelSurgeWTOSC);
    p->addModel(modelSurgeNoise);
    p->addModel(modelSurgeWaveShaper);

    p->addModel(modelSurgePatchPlayer);

    p->addModel(modelSurgeLFO);

    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeBiquad);

    if (fxModels != nullptr)
        for (auto i = 0; i < n_fx_types; ++i)
        {
            if (fxModels[i] != nullptr)
            {
                p->addModel(fxModels[i]);
            }
        }
}
