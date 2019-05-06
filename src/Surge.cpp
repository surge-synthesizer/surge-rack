#include "Surge.hpp"

rack::Plugin *pluginInstance;

void init(rack::Plugin *p) {
    pluginInstance = p;

#ifndef RACK_V1
    p->slug = "SurgeRack";
#ifdef VERSION
    p->version = TOSTRING(VERSION);
#endif
    p->website = "https://github.com/surge-synthesizer/surge-rack";
#endif

    p->addModel(modelSurgeFX);
    p->addModel(modelSurgeADSR);
    p->addModel(modelSurgeOSC);
    p->addModel(modelSurgeWaveShaper);
    p->addModel(modelSurgeWTOSC);
    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeLFO);
}
