/*
** Widgets to ease double buffering with lambdas, to create input panel areas,
** to have a common background widget and more
*/
#include "Surge.hpp"
#include "SurgeStyle.hpp"
#include "SurgeWidgets.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <functional>
#include <map>
#ifndef RACK_V1
#include "widgets.hpp"
#endif

struct SurgeModuleWidgetCommon : public virtual rack::ModuleWidget, SurgeStyle {
#if RACK_V1
    SurgeModuleWidgetCommon() : rack::ModuleWidget() { }
#else
    SurgeModuleWidgetCommon(rack::Module *m) : rack::ModuleWidget(m) { }

    int stepsSinceRefresh = 0;
    void step() override {
        /*
        ** This basically moves the surgesmallknobs if they are invalidated
        ** which is required in V0.6.2 (but not in V1 where this is done internally
        ** in the knobs). Since I use a dyn cast to find em, only do it every
        ** 10 frames or so
        */
        if( module && stepsSinceRefresh == 0)
        {
            for(auto pw : params )
            {
                SurgeSmallKnob *ssk;
                if( (ssk = dynamic_cast<SurgeSmallKnob *>(pw) ) != nullptr )
                {
                    if( ssk->value != module->params[ssk->paramId].value )
                    {
                        ssk->setValue(module->params[ssk->paramId].value);
                    }
                }
            }
        }
        stepsSinceRefresh ++;
        if( stepsSinceRefresh >= 10 ) stepsSinceRefresh = 0;
        
        ModuleWidget::step();
    }
    
#endif    
};

