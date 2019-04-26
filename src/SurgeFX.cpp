#include "Surge.hpp"
#include "SurgeFX.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeFXWidget : rack::ModuleWidget {
    typedef SurgeFX<rack::Module> M;
    SurgeFXWidget(M *module);
};

SurgeFXWidget::SurgeFXWidget(SurgeFXWidget::M *module) : rack::ModuleWidget(
#ifndef RACK_V1
    module
#endif
    ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 14, RACK_HEIGHT);
    addChild(new SurgeRackBG(rack::Vec(0,0), box.size, "SurgeFX"));
}
    

rack::Model *modelSurgeFX = rack::createModel<SurgeFXWidget::M, SurgeFXWidget>("SurgeFX");
