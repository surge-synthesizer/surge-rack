#include "Surge.hpp"

std::set<rack::Model *> modelSurgeFXSet;

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

    for( auto m : modelSurgeFXSet )
        p->addModel(m);
    
    p->addModel(modelSurgeADSR);
    p->addModel(modelSurgeOSC);
    p->addModel(modelSurgeWaveShaper);
    p->addModel(modelSurgeWTOSC);
    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeLFO);
    p->addModel(modelSurgeVOC);
}
