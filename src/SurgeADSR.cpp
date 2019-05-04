#include "SurgeADSR.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeADSRWidget : rack::ModuleWidget {
    typedef SurgeADSR M;
    SurgeADSRWidget(M *module);
};

SurgeADSRWidget::SurgeADSRWidget(SurgeADSRWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 8, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "SurgeEnv");
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(15,15),
                                                 module, M::GATE_IN));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(15 + 24.6 + 3, 15),
                                                     module, M::RETRIG_IN));

    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(15, 15 + 24.6 + 10),
                                                   module, M::OUTPUT_ENV));

    int envStart = 15 + 24.6 * 2 + 10 + 15;
    int envHeight = 40;
    int x0 = 30;

    for( int i=M::A_PARAM; i<= M::R_PARAM; ++i )
    {
        int ipos = i - M::A_PARAM;
        addParam( rack::createParam<SurgeSmallKnob>( rack::Vec(x0+30, ipos * envHeight + envStart ), module, i
#if !RACK_V1
                                                     ,0,1,0.5
#endif                                                     

                      ) );
    }

    for( int i=M::A_CV; i<= M::R_CV; ++i )
    {
        int ipos = i - M::A_CV;
        addInput( rack::createInput<rack::PJ301MPort>( rack::Vec(x0, ipos * envHeight + envStart ), module, i ) );
    }

    for( int i=M::A_S_PARAM; i<=M::R_S_PARAM; ++i)
    {
        int ipos = i - M::A_S_PARAM;
        if( i == M::R_S_PARAM)
            ipos++;
        
        addParam(rack::createParam<rack::CKSSThree>( rack::Vec( x0+60, ipos * envHeight + envStart ), module, i
#if !RACK_V1
                                                     ,0,2,0
#endif                                                     

                     ));
    }

    addParam(rack::createParam<SurgeSwitchFull>(rack::Vec(15 + 2 * 24.6 + 6, 15), module, M::MODE_PARAM
#if !RACK_V1
                                                     ,0,1,0
#endif                                                     
                 ));
    
}

#if RACK_V1
rack::Model *modelSurgeADSR =
    rack::createModel<SurgeADSRWidget::M, SurgeADSRWidget>("SurgeADSR");
#else
rack::Model *modelSurgeADSR = rack::createModel<SurgeADSRWidget::M, SurgeADSRWidget>(
    "Surge Team", "SurgeADSR", "SurgeADSR", rack::ENVELOPE_GENERATOR_TAG);
#endif
