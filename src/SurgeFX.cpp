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
        int textAreaWidth = box.size.x - 5 * padMargin - 2 * portX - surgeSwitchX;

        for( int i=0; i<nControls; ++i )
        {
            SurgeStyle::drawTextBGRect(vg, 4*padMargin+2*portX + surgeSwitchX, i*controlHeight + SCREW_WIDTH + padMargin,
                                       textAreaWidth, controlHeight-padMargin);
        }

        drawLeftRightInputOutputBackground(vg, box);

        drawBlueIORect(vg, box.size.x/2 - portX / 2 - padMargin, orangeLine + ioMargin,
                       portX + 2 * padMargin, box.size.y - orangeLine - 2 * ioMargin );
        nvgBeginPath(vg);
        nvgFillColor(vg, surgeWhite() );
        nvgFontFaceId(vg, fontId(vg) );
        nvgFontSize(vg, 11);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
        nvgText(vg, box.size.x/2, box.size.y - ioMargin - 1.5, "clock cv", NULL );
    }

};

template <int effectType>
SurgeFXWidget<effectType>::SurgeFXWidget(SurgeFXWidget<effectType>::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 19, RACK_HEIGHT);
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
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box), module,
                                               M::OUTPUT_GAIN

                                               ));

    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec(box.size.x/2 - portX/2, orangeLine + 1.5 * ioMargin),
                 module, M::CLOCK_CV_INPUT));

    int parmMargin = 3;

    
    int textAreaWidth = box.size.x - 5 * padMargin - 2 * portX - surgeSwitchX;
    for( int i=0; i<nControls; ++i )
    {
        float yPos = i * controlHeight + SCREW_WIDTH + padMargin;
        int pa = M::FX_PARAM_0 + i;
        int cv = M::FX_PARAM_INPUT_0 + i;
        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(2 * padMargin + portX, yPos), module, cv ));
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(padMargin, yPos), module,
                                                   pa
                                                   ));

        if( module && module->canTempoSync(i) )
        {
            addParam(rack::createParam<SurgeSwitch>(rack::Vec(3 * padMargin + 2 * portX, yPos ), module,
                                                    M::PARAM_TEMPOSYNC_0 + i
                         ));
        }
        
        int tx = 4 * padMargin + 2 * portX + surgeSwitchX + 2;
        addChild(TextDisplayLight::create(rack::Vec(tx, yPos),
                                          rack::Vec(textAreaWidth, controlHeight - padMargin),
                                          module ? &(module->labelCache[i]) : nullptr,
                                          13, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, SurgeStyle::surgeOrange()));

        addChild(TextDisplayLight::create(rack::Vec(tx, yPos),
                                          rack::Vec(textAreaWidth, controlHeight - padMargin),
                                          module ? &(module->groupCache[i]) : nullptr,
                                          9, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, SurgeStyle::surgeWhite()));

        addChild(TextDisplayLight::create(rack::Vec(tx , yPos),
                                          rack::Vec(textAreaWidth - 2 * padMargin, controlHeight - padMargin),
                                          module ? &(module->paramDisplayCache[i]) : nullptr,
                                          14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, SurgeStyle::surgeWhite()));
        
    }
}


#define CREATE_FX( type, name ) auto v ## type = addFX( \
     rack::createModel<SurgeFXWidget< type >::M, SurgeFXWidget< type >>(name), type);

// FIXME: Eventually each of these get their own panel and this list drops to 0
CREATE_FX( fxt_delay, "SurgeDelay" );
CREATE_FX( fxt_phaser, "SurgePhaser" );
CREATE_FX( fxt_distortion, "SurgeDistort" );
CREATE_FX( fxt_reverb, "SurgeReverb" );
CREATE_FX( fxt_reverb2, "SurgeReverb2" );
CREATE_FX( fxt_chorus4, "SurgeChorus" );
CREATE_FX( fxt_conditioner, "SurgeConditioner" );
