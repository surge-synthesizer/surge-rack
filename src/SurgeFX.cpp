#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

template <int effectType>
struct SurgeFXWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<effectType> M;
    SurgeFXWidget(M *module);

    int nControls = 12;
    
    float controlAreaHeight = orangeLine - padMargin - SCREW_WIDTH;
    float controlHeight = controlAreaHeight / nControls;


    void moduleBackground(NVGcontext *vg) {
        int textAreaWidth = box.size.x - 4 * padMargin - 2 * portX;

        for( int i=0; i<nControls; ++i )
        {
            SurgeStyle::drawTextBGRect(vg, 3*padMargin+2*portX, i*controlHeight + SCREW_WIDTH + padMargin,
                                       box.size.x - 4*padMargin - 2 * portX, controlHeight-padMargin);
        }

        drawLeftRightInputOutputBackground(vg, box);
    }

};

template <int effectType>
SurgeFXWidget<effectType>::SurgeFXWidget(SurgeFXWidget<effectType>::M *module)
#ifndef RACK_V1
    : SurgeModuleWidgetCommon( module ), rack::ModuleWidget( module )
#else
    : SurgeModuleWidgetCommon()
#endif      
{
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 16, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, SurgeFXName<effectType>::getName());
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };

    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box), module,
                                               M::INPUT_GAIN
#if !RACK_V1
                                               ,
                                               0, 1, 1
#endif
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box), module,
                                               M::OUTPUT_GAIN
#if !RACK_V1
                                               ,
                                               0, 1, 1
#endif

                                               ));
    int parmMargin = 3;

    
    int textAreaWidth = box.size.x - 4 * padMargin - 2 * portX;
    for( int i=0; i<nControls; ++i )
    {
        float yPos = i * controlHeight + SCREW_WIDTH + padMargin;
        int pa = M::FX_PARAM_0 + i;
        int cv = M::FX_PARAM_INPUT_0 + i;
        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(padMargin, yPos), module, cv ));
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(2 * padMargin + portX, yPos), module,
                                                   pa
#if !RACK_V1
                                                   ,
                                                   0, 1, 0.5
#endif
                                                   ));

        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * portX + 2, yPos),
                                          rack::Vec(textAreaWidth, controlHeight - padMargin),
                                          module ? &(module->labelCache[i]) : nullptr,
                                          13, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, SurgeStyle::surgeOrange()));

        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * portX + 2, yPos),
                                          rack::Vec(textAreaWidth, controlHeight - padMargin),
                                          module ? &(module->groupCache[i]) : nullptr,
                                          9, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, SurgeStyle::surgeWhite()));

        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * portX + 2 , yPos),
                                          rack::Vec(textAreaWidth - 2 * padMargin, controlHeight - padMargin),
                                          module ? &(module->paramDisplayCache[i]) : nullptr,
                                          14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, SurgeStyle::surgeWhite()));
        
    }
}


#if RACK_V1
#define CREATE_FX( type, name ) auto v ## type = modelSurgeFXSet.insert( \
        rack::createModel<SurgeFXWidget< type >::M, SurgeFXWidget< type >>(name));
#else
#define CREATE_FX( type, name ) auto v ## type = modelSurgeFXSet.insert( \
        rack::createModel<SurgeFXWidget< type >::M, SurgeFXWidget< type >>( \
            "Surge Team", name, name, rack::EFFECT_TAG));
#endif

// FIXME: Eventually each of these get their own panel and this list drops to 0
CREATE_FX( fxt_delay, "SurgeDelay" );
CREATE_FX( fxt_eq, "SurgeEQ" );
CREATE_FX( fxt_phaser, "SurgePhaser" );
CREATE_FX( fxt_distortion, "SurgeDistort" );
CREATE_FX( fxt_reverb, "SurgeReverb" );
CREATE_FX( fxt_reverb2, "SurgeReverb2" );
CREATE_FX( fxt_chorus4, "SurgeChorus" );
CREATE_FX( fxt_conditioner, "SurgeConditioner" );
