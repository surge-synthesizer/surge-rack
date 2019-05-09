#include "SurgeOSC.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeOSCWidget : rack::ModuleWidget {
    typedef SurgeOSC M;
    SurgeOSCWidget(M *module);

    int fontId = -1;
    int ioMargin = 7;
    int ioRegionWidth = 105;
    int padMargin = 3;
    int padFromEdge = 5;

    float buttonBankY = SCREW_WIDTH + padFromEdge;
    float buttonBankHeight = 25;
    
    float pitchY = buttonBankY + buttonBankHeight + padMargin;
    float controlsY = pitchY + padMargin + SurgeLayout::surgeRoosterY;
    float controlsHeight = SurgeLayout::orangeLine - controlsY - padMargin;
    float controlHeightPer = controlsHeight / n_osc_params;
    
    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        nvgBeginPath(vg);

        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

        // Draw the output blue box
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;
        SurgeStyle::drawBlueIORect(
            vg, x0 + ioMargin, SurgeLayout::orangeLine + ioMargin,
            ioRegionWidth, box.size.y - SurgeLayout::orangeLine - 2 * ioMargin);

        nvgFillColor(vg, SurgeStyle::backgroundGray());
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 12);
        nvgSave(vg);
        nvgTranslate(vg, x0 + ioMargin + ioRegionWidth - 2,
                     SurgeLayout::orangeLine + ioMargin * 1.5);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgRotate(vg, M_PI / 2);
        nvgText(vg, 0, 0, "Output", NULL);
        nvgRestore(vg);

        rack::Vec ll;
        ll = ioPortLocation(0);
        ll.y = box.size.y - ioMargin - 1.5;
        ll.x += 24.6721 / 2;
        nvgFontSize(vg, 11);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "L/Mon", NULL);

        ll = ioPortLocation(1);
        ll.y = box.size.y - ioMargin - 1.5;
        ll.x += 24.6721 / 2;
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "R", NULL);

        ll = ioPortLocation(2);
        ll.y = box.size.y - ioMargin - 1.5;
        ll.x += 24.6721 / 2;
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "Gain", NULL);

        float pitchLY = pitchY + SurgeLayout::surgeRoosterY / 2.0;
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 15);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT );
        nvgFillColor(vg, SurgeStyle::surgeBlue() );
        nvgText(vg, padFromEdge, pitchLY, "Pitch", NULL );
        nvgBeginPath(vg);
        nvgMoveTo(vg,43,pitchLY);
        nvgLineTo(vg,box.size.x - padFromEdge - 2 * padMargin - SurgeLayout::surgeRoosterY - SurgeLayout::portX - 6, pitchLY );
        nvgStrokeColor(vg, SurgeStyle::surgeBlue() );
        nvgStrokeWidth(vg, 1 );
        nvgStroke(vg);

        for (int i = 0; i < n_osc_params; ++i) {
            float yp = i * controlHeightPer + controlsY;
            float xp = padFromEdge + 2 * padMargin + 2 * SurgeLayout::portX;
            SurgeStyle::drawTextBGRect(vg, xp, yp, box.size.x - padFromEdge - xp, controlHeightPer - padMargin);
        }
    }

    rack::Vec ioPortLocation(int ctrl) { // 0 is L; 1 is R; 2 is gain
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

        int xRes =
            x0 + ioMargin + padFromEdge + (ctrl * (SurgeLayout::portX + 4));
        int yRes = SurgeLayout::orangeLine + 1.5 * ioMargin;

        return rack::Vec(xRes, yRes);
    }
};

SurgeOSCWidget::SurgeOSCWidget(SurgeOSCWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 12, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "OSC");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(0), module,
                                                   M::OUTPUT_L));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(1), module,
                                                   M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(2), module,
                                               M::OUTPUT_GAIN
#if !RACK_V1
                                               ,
                                               0, 1, 1
#endif
                                               ));

    // Improve this API
    SurgeButtonBank *bank = SurgeButtonBank::create( rack::Vec(padFromEdge, SCREW_WIDTH + padMargin),
                                                     rack::Vec(box.size.x - 2 * padFromEdge, buttonBankHeight),
                                                     5, 1, 13 );
#if RACK_V1
    if( module )
        bank->paramQuantity = module->paramQuantities[M::OSC_TYPE];
#else
    bank->module = module;
    bank->paramId = M::OSC_TYPE;
    bank->setLimits(0,4);
    bank->setDefaultValue(0);
#endif    

    bank->addLabel("Classic");
    bank->addLabel("Sine");
    bank->addLabel("FM2");
    bank->addLabel("FM3");
    bank->addLabel("Noise");
    
    addParam(bank);

    int xp = box.size.x - padMargin - padFromEdge - SurgeLayout::portX - SurgeLayout::surgeRoosterX;
    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(xp, pitchY), module, M::PITCH_0
#if !RACK_V1
        ,
        1, 127, 72
#endif
        ));
    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec( xp + SurgeLayout::surgeRoosterX + padMargin,
                            pitchY + ( SurgeLayout::surgeRoosterX - SurgeLayout::portX ) / 2),
                 module, M::PITCH_CV));

    for (int i = 0; i < n_osc_params; ++i) {
        float yp = i * controlHeightPer + controlsY;
        float yctrl = yp + controlHeightPer / 2 - SurgeLayout::portY / 2 - padMargin/2;
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(padFromEdge, yctrl), module,
                                                   M::OSC_CTRL_PARAM_0 + i
#if !RACK_V1
                                                   ,
                                                   0, 1, 0.5
#endif
                                                   ));
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(padFromEdge + padMargin + SurgeLayout::portX, yctrl), module,
                                                     M::OSC_CTRL_CV_0 + i));

        float xt = padFromEdge + 2 * padMargin + 2 * SurgeLayout::portX;

        addChild(TextDisplayLight::create(
                     rack::Vec(xt+2, yp + 0.5), rack::Vec(box.size.x - xt - padMargin, controlHeightPer - padMargin - 2),
                     module ? &(module->paramNameCache[i]) : nullptr,
                     12));
        addChild(TextDisplayLight::create(
                     rack::Vec(xt+2, yp+1 ), rack::Vec(box.size.x - xt - padMargin, controlHeightPer - padMargin - 2),
                     module ? (&module->paramValueCache[i]) : nullptr,
                     15, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, SurgeStyle::surgeWhite()));
    }
}

#if RACK_V1
rack::Model *modelSurgeOSC =
    rack::createModel<SurgeOSCWidget::M, SurgeOSCWidget>("SurgeOSC");
#else
rack::Model *modelSurgeOSC =
    rack::createModel<SurgeOSCWidget::M, SurgeOSCWidget>(
        "Surge Team", "SurgeOSC", "SurgeOSC", rack::ENVELOPE_GENERATOR_TAG);
#endif
