#include "SurgeOSCSingle.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

template <int oscType>
struct SurgeOSCSingleWidget : public virtual SurgeModuleWidgetCommon {
    typedef SurgeOSCSingle<oscType> M;
    SurgeOSCSingleWidget(M *module);

    int ioRegionWidth = 105;

    float buttonBankY = SCREW_WIDTH + padFromEdge;
    float buttonBankHeight = 25;
    
    float pitchY = buttonBankY + buttonBankHeight + padMargin;
    float pitchCtrlX = 36;
    
    float controlsY = pitchY + padMargin + surgeRoosterY;
    float controlsHeight = orangeLine - controlsY - padMargin;
    float controlHeightPer = controlsHeight / n_osc_params;
    
    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        nvgBeginPath(vg);

        // Draw the output blue box
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;
        drawBlueIORect(
            vg, x0 + ioMargin, orangeLine + ioMargin,
            ioRegionWidth, box.size.y - orangeLine - 2 * ioMargin);

        nvgFillColor(vg, ioRegionText());
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgSave(vg);
        nvgTranslate(vg, x0 + ioMargin + ioRegionWidth - 2,
                     orangeLine + ioMargin * 1.5);
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

        float pitchLY = pitchY + surgeRoosterY / 2.0;
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 15);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT );
        nvgFillColor(vg, panelLabel() );
        nvgText(vg, padFromEdge, pitchLY, "Pitch", NULL );

        float xt = pitchCtrlX + surgeRoosterX + portX + 2 * padMargin + 12;

        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 10);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT );
        nvgFillColor(vg, panelLabel() );
        nvgText(vg, xt, pitchY + 2, "f", NULL );

        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 10);
        nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT );
        nvgFillColor(vg, panelLabel() );
        nvgText(vg, xt, pitchY + surgeRoosterY - 4, "n", NULL );

        
        xt += 7 + padMargin;
        drawTextBGRect(vg, xt, pitchY+6, box.size.x - padFromEdge - xt, surgeRoosterY-12 );
            
        for (int i = 0; i < n_osc_params; ++i) {
            float yp = i * controlHeightPer + controlsY;
            float xp = padFromEdge + 2 * padMargin + 2 * portX + 24;
            drawTextBGRect(vg, xp, yp, box.size.x - padFromEdge - xp, controlHeightPer - padMargin);
        }
    }

    rack::Vec ioPortLocation(int ctrl) { // 0 is L; 1 is R; 2 is gain
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

        int xRes =
            x0 + ioMargin + padFromEdge + (ctrl * (portX + 4));
        int yRes = orangeLine + 1.5 * ioMargin;

        return rack::Vec(xRes, yRes);
    }
};

template<int oscType>
SurgeOSCSingleWidget<oscType>::SurgeOSCSingleWidget(SurgeOSCSingleWidget<oscType>::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 14, RACK_HEIGHT);
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
                                               ));

    if (SingleConfig<oscType>::needsRetrigger())
    {
        addInput(rack::createInput<rack::PJ301MPort>(
            rack::Vec( pitchCtrlX,
                      pitchY - 30 ),
            module, M::RETRIGGER
            ));
    }


    int xp = pitchCtrlX;
    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(xp, pitchY), module, M::PITCH_0
        ));
    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec( xp + surgeRoosterX + padMargin,
                            pitchY + ( surgeRoosterY - portY ) / 2),
                 module, M::PITCH_CV));

    addChild(TextDisplayLight::create(
                 rack::Vec(xp + surgeRoosterX + portX + 3 * padMargin + 20,
                           pitchY + 3 ),
                 rack::Vec(50, surgeRoosterY - 8 ),
                 module ? &(module->pitch0DisplayCache) : nullptr,
                 14,
                 NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE,
                 parameterValueText_KEY()
                 ));

    for (int i = 0; i < n_osc_params; ++i) {
        float yp = i * controlHeightPer + controlsY;
        float yctrl = yp + controlHeightPer / 2 - portY / 2 - padMargin/2;
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(padFromEdge, yctrl), module,
                                                   M::OSC_CTRL_PARAM_0 + i
                                                   ));
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(padFromEdge + padMargin + portX, yctrl), module,
                                                     M::OSC_CTRL_CV_0 + i));

        addParam(rack::createParam<SurgeDisableStateSwitch>(rack::Vec(padFromEdge + 2 * padMargin + 2 * portX, yctrl ), module,
                                                M::OSC_EXTEND_PARAM_0 + i ) );
        addParam(rack::createParam<SurgeDisableStateSwitch>(rack::Vec(padFromEdge + 2 * padMargin + 2 * portX + 12, yctrl ), module,
                                                M::OSC_DEACTIVATE_INVERSE_PARAM_0 + i ) );

        float xt = padFromEdge + 2 * padMargin + 2 * portX + 12;

        addChild(TextDisplayLight::create(
                     rack::Vec(xt+2 + 12, yp + 0.5), rack::Vec(box.size.x - xt - padMargin, controlHeightPer - padMargin - 2),
                     module ? &(module->paramNameCache[i]) : nullptr,
                     12));
        addChild(TextDisplayLight::create(
                     rack::Vec(xt+2 + 12, yp+1 ), rack::Vec(box.size.x - xt - padMargin, controlHeightPer - padMargin - 2),
                     module ? (&module->paramValueCache[i]) : nullptr,
                     15, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, parameterValueText_KEY()));
    }
}

template<>
constexpr bool SingleConfig<ot_string>::needsRetrigger() { return true; }

rack::Model *modelSurgeOSCModern =
    rack::createModel<SurgeOSCSingleWidget<ot_modern>::M, SurgeOSCSingleWidget<ot_modern>>("SurgeOSCModern");

rack::Model *modelSurgeOSCString =
    rack::createModel<SurgeOSCSingleWidget<ot_string>::M, SurgeOSCSingleWidget<ot_string>>("SurgeOSCString");

rack::Model *modelSurgeOSCAlias =
    rack::createModel<SurgeOSCSingleWidget<ot_alias>::M, SurgeOSCSingleWidget<ot_alias>>("SurgeOSCAlias");
