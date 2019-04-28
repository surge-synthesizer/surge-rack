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

    box.size = rack::Vec(SCREW_WIDTH * 50, RACK_HEIGHT);
    addChild(new SurgeRackBG(rack::Vec(0,0), box.size, "SurgeFX"));

    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(7, 340), module, M::INPUT_R_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(47, 340), module, M::INPUT_L));

    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(SCREW_WIDTH * 50 - 90, 340), module, M::OUTPUT_R_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(SCREW_WIDTH * 50 - 50, 340), module, M::OUTPUT_L));

    for( int i=0; i<6; ++i )
    {
        int pos = 60 + i * 40;
        int x0 = 0;
        addParam(rack::createParam<rack::RoundSmallBlackKnob>(rack::Vec( x0 + 10, pos), module, M::FX_PARAM_0 + i
#ifndef RACK_V1
                                                              , 0, 1, 0.5
#endif
                     ) );
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec( x0 + 40, pos ), module, M::FX_PARAM_INPUT_0 + i ) );
        addChild(TextDisplayLight::create(rack::Vec( x0 + 90, pos ), rack::Vec( SCREW_WIDTH * 17, 35 ),
                                          [module, i]() { return module ? module->getStringDirty(i) : false; },
                                          [module, i]() { return module ? module->getStringName(i) : "NAME"; } ) );

        x0 = SCREW_WIDTH * 25;;
        addParam(rack::createParam<rack::RoundSmallBlackKnob>(rack::Vec( x0 + 10, pos), module, M::FX_PARAM_0 + i + 6
#ifndef RACK_V1
                                                              , 0, 1, 0.5
#endif
                     ) );
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec( x0 + 40, pos ), module, M::FX_PARAM_INPUT_0 + i + 6) );
        addChild(TextDisplayLight::create(rack::Vec( x0 + 90, pos ), rack::Vec( SCREW_WIDTH * 17, 35 ),
                                          [module, i]() { return module ?  module->getStringDirty(i + 6) : false; },
                                          [module, i]() { return module ? module->getStringName(i + 6) : "NAME"; } ) );

    }
}
    
#if RACK_V1
rack::Model *modelSurgeFX = rack::createModel<SurgeFXWidget::M, SurgeFXWidget>("SurgeFX");
#else
rack::Model *modelSurgeFX = rack::createModel<SurgeFXWidget::M, SurgeFXWidget>("Surge Team", "SurgeRack", "SurgeFX", rack::EFFECT_TAG);
#endif
