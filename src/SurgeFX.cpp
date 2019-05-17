#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

template <int effectType>
struct SurgeFXWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<effectType> M;
    SurgeFXWidget(M *module);

    int ioRegionWidth = 105;

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

        
        for (int i = 0; i < 2; ++i) {
            nvgBeginPath(vg);
            int x0 = 0;
            if (i == 1)
                x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

            SurgeStyle::drawBlueIORect(
                vg, x0 + ioMargin, orangeLine + ioMargin,
                ioRegionWidth,
                box.size.y - orangeLine - 2 * ioMargin,
                (i == 0) ? 0 : 1);

            nvgFillColor(vg, SurgeStyle::backgroundGray());
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 12);
            if (i == 0) {
                nvgSave(vg);
                nvgTranslate(vg, x0 + ioMargin + 2,
                             orangeLine + ioMargin * 1.5);
                nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgRotate(vg, -M_PI / 2);
                nvgText(vg, 0, 0, "Input", NULL);
                nvgRestore(vg);
            } else {
                nvgSave(vg);
                nvgTranslate(vg, x0 + ioMargin + ioRegionWidth - 2,
                             orangeLine + ioMargin * 1.5);
                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgRotate(vg, M_PI / 2);
                nvgText(vg, 0, 0, "Output", NULL);
                nvgRestore(vg);
            }
            rack::Vec ll;
            ll = ioPortLocation(i == 0, 0);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "L/Mon", NULL);

            ll = ioPortLocation(i == 0, 1);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "R", NULL);

            ll = ioPortLocation(i == 0, 2);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "Gain", NULL);
        }
    }

    rack::Vec ioPortLocation(bool input,
                             int ctrl) { // 0 is L; 1 is R; 2 is gain
        int x0 = 0;
        if (!input)
            x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

        int padFromEdge = input ? 17 : 5;
        int xRes =
            x0 + ioMargin + padFromEdge + (ctrl * (portX + 4));
        int yRes = orangeLine + 1.5 * ioMargin;

        return rack::Vec(xRes, yRes);
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

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2), module,
                                               M::INPUT_GAIN
#if !RACK_V1
                                               ,
                                               0, 1, 1
#endif
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
        ioPortLocation(false, 0), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2), module,
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

CREATE_FX( fxt_delay, "SurgeDelay" );
CREATE_FX( fxt_eq, "SurgeEQ" );
CREATE_FX( fxt_phaser, "SurgePhaser" );
CREATE_FX( fxt_rotaryspeaker, "SurgeRotary" );
CREATE_FX( fxt_distortion, "SurgeDistort" );
CREATE_FX( fxt_reverb, "SurgeReverb" );
CREATE_FX( fxt_reverb2, "SurgeReverb2" );
CREATE_FX( fxt_freqshift, "SurgeFreqShift" );
CREATE_FX( fxt_chorus4, "SurgeChorus" );
CREATE_FX( fxt_conditioner, "SurgeConditioner" );
