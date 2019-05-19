#include "Surge.hpp"

std::set<rack::Model *> modelSurgeFXSet;

rack::Plugin *pluginInstance;

void init(rack::Plugin *p) {
    pluginInstance = p;

    for( auto m : modelSurgeFXSet )
        p->addModel(m);
    
    p->addModel(modelSurgeADSR);
    p->addModel(modelSurgeOSC);
    p->addModel(modelSurgeWaveShaper);
    p->addModel(modelSurgeClock);
    p->addModel(modelSurgeWTOSC);
    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeLFO);
    p->addModel(modelSurgeVOC);
}
