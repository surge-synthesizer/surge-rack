#include "Surge.hpp"
#include "SurgeStorage.h"

rack::Model **fxModels = nullptr;
int addFX(rack::Model *m, int type)
{
   if( fxModels == nullptr ) {
       fxModels = new rack::Model *[num_fxtypes];
       for( auto i=0; i<num_fxtypes; ++i )
           fxModels[i] = nullptr;
   }
   fxModels[type] = m;
   return type;
}

rack::Plugin *pluginInstance;

void init(rack::Plugin *p) {
    pluginInstance = p;
    rack::INFO( "[SurgeRack] init" );

    p->addModel(modelSurgeClock);
    p->addModel(modelSurgeADSR);

    p->addModel(modelSurgeOSC);
    p->addModel(modelSurgeWaveShaper);

    p->addModel(modelSurgeWTOSC);
    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeLFO);

    if( fxModels != nullptr )
        for( auto i=0; i<num_fxtypes; ++i )
            if( fxModels[i] != nullptr )
                p->addModel(fxModels[i]);
}
