#include "SurgeWTOSC.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeWTOSCWidget : public virtual SurgeModuleWidgetCommon {
    typedef SurgeWTOSC M;
    SurgeWTOSCWidget(M *module);

    int ioRegionWidth = 105;

    float pitchY = SCREW_WIDTH + padFromEdge;
    float pitchCtrlX = padFromEdge + 2 * padMargin + 2 * portX;
    
    float controlsY = pitchY + 2 * padMargin + surgeRoosterY + portY;
    float controlsHeight = orangeLine - controlsY - padMargin;
    float controlHeightPer = controlsHeight / n_osc_params;

    float colOneEnd = surgeRoosterX + portX + 2 * padMargin + 14 + pitchCtrlX + padFromEdge;
    float wtSelY = pitchY + 113;

    float catitlodsz = ( SCREW_WIDTH * 18 - colOneEnd ) / 3;
    

    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        nvgBeginPath(vg);

        drawLeftRightInputOutputBackground(vg, box, "", false, true);
        
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 15);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT );
        nvgFillColor(vg, surgeBlue() );
        nvgText(vg, padFromEdge + 10, pitchY + surgeRoosterY / 2, "Pitch", NULL );

        float bounds[4];
        nvgTextBounds( vg, padFromEdge + 10, pitchY + surgeRoosterY / 2, "Pitc", NULL, bounds );

        float xt = pitchCtrlX + surgeRoosterX + portX + 2 * padMargin + 12;

        nvgBeginPath(vg);
        float bmx = (bounds[2]-bounds[0])/2.0 + bounds[ 0 ];
        dropRightLine(vg, bmx, bounds[3], xt, pitchY + surgeRoosterY + padMargin + portY/2);
        
        drawTextBGRect(vg, pitchCtrlX, pitchY+surgeRoosterY + padMargin,
                       surgeRoosterX + portX + 2 * padMargin + 14, portY );
            
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 10);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT );
        nvgFillColor(vg, surgeBlue() );
        nvgText(vg, xt, pitchY + 2, "f", NULL );

        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 10);
        nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT );
        nvgFillColor(vg, surgeBlue() );
        nvgText(vg, xt, pitchY + surgeRoosterY - 4, "n", NULL );

        xt += 7 + padMargin;
        
        for (int i = 0; i < n_osc_params; ++i) {
            float yp = i * controlHeightPer + controlsY;
            float xp = padFromEdge + 2 * padMargin + 2 * portX;
            drawTextBGRect(vg, xp, yp, colOneEnd - padFromEdge - xp, controlHeightPer - padMargin);
        }

        auto xpf = [this](int i) { return this->colOneEnd + this->catitlodsz * ( i + 0.5 );  };
        std::vector<std::string> cil = { "Cat", "WT", "Load" };
        for( int i=0; i<3; ++i )
        {
            nvgBeginPath(vg);
            nvgFontFaceId(vg,fontId(vg));
            nvgFontSize(vg,12);
            nvgFillColor(vg, surgeBlue());
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER );
            nvgText(vg, xpf(i), wtSelY - 14, cil[i].c_str(), NULL );
        }
                

        auto yTPos = wtSelY + surgeRoosterY + padMargin;
        drawTextBGRect(vg, colOneEnd + padFromEdge, yTPos,
                                   box.size.x - colOneEnd - 2 * padFromEdge, 20 );
        yTPos += 20 + padMargin;
        drawTextBGRect(vg, colOneEnd + padFromEdge, yTPos,
                                   box.size.x - colOneEnd - 2 * padFromEdge, 124 );
        
    }

    rack::Vec ioPortLocation(int ctrl) { // 0 is L; 1 is R; 2 is gain
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

        int xRes =
            x0 + ioMargin + padFromEdge + (ctrl * (portX + 4));
        int yRes = orangeLine + 1.5 * ioMargin;

        return rack::Vec(xRes, yRes);
    }
};

SurgeWTOSCWidget::SurgeWTOSCWidget(SurgeWTOSCWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 18, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "WTOSC");
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


    int xp = pitchCtrlX;
    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(xp, pitchY), module, M::PITCH_0
        ));
    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec( xp + surgeRoosterX + padMargin,
                            pitchY + ( surgeRoosterY - portY ) / 2),
                 module, M::PITCH_CV));
    addParam(rack::createParam<SurgeSwitch>(rack::Vec(xp + surgeRoosterX + portX + 2 * padMargin,
                                                      pitchY + ( surgeRoosterY - 21 ) / 2),
                                            module, M::PITCH_0_IN_FREQ
                 ));

    addChild(TextDisplayLight::create(
                 rack::Vec(pitchCtrlX + 2, pitchY+surgeRoosterY + padMargin),
                 rack::Vec(surgeRoosterX + portX + 2 * padMargin + 14, portY ),
                 module ? &(module->pitch0DisplayCache) : nullptr,
                 14,
                 NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE,
                 surgeWhite()
                 ));

    for (int i = 0; i < n_osc_params; ++i) {
        float yp = i * controlHeightPer + controlsY;
        float yctrl = yp + controlHeightPer / 2 - portY / 2 - padMargin/2;
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(padFromEdge, yctrl), module,
                                                   M::OSC_CTRL_PARAM_0 + i
                                                   ));
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(padFromEdge + padMargin + portX, yctrl), module,
                                                     M::OSC_CTRL_CV_0 + i));

        float xt = padFromEdge + 2 * padMargin + 2 * portX;

        addChild(TextDisplayLight::create(
                     rack::Vec(xt+2, yp + 0.5), rack::Vec(colOneEnd - xt - padMargin, controlHeightPer - padMargin - 2),
                     module ? &(module->paramNameCache[i]) : nullptr,
                     12));
        addChild(TextDisplayLight::create(
                     rack::Vec(xt+2, yp+1 ), rack::Vec(colOneEnd - xt - padMargin, controlHeightPer - padMargin - 2),
                     module ? (&module->paramValueCache[i]) : nullptr,
                     15, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, surgeWhite()));
    }

    float c2w = (box.size.x - colOneEnd) / 2 + colOneEnd;
    SurgeButtonBank *bank = SurgeButtonBank::create( rack::Vec(c2w - 100/2, pitchY),
                                                     rack::Vec(100, 25),
                                                     module, M::WT_OR_WINDOW,
                                                     2, 1, 13 );

    bank->addLabel("Wavetable");
    bank->addLabel("Window");
    
    addParam(bank);

    auto xpf = [this](int i) { return this->colOneEnd + this->catitlodsz * ( i + 0.5 ) - this->surgeRoosterX / 2.0; };
    
    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(xpf(0), wtSelY), module, M::CATEGORY_IDX
        ));


    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(xpf(1), wtSelY),
                 module, M::WT_IN_CATEGORY_IDX
                 ));

    addParam(rack::createParam<rack::CKD6>(
                 rack::Vec(xpf(2), wtSelY + 4),
                 module, M::LOAD_WT
                 ));

    addChild(rack::createLight<rack::SmallLight<rack::RedLight>>(
                 rack::Vec(xpf(2) + 27, wtSelY+1),
                 module, M::NEEDS_LOAD
                 ));


    auto xTPos = colOneEnd + padMargin;
    auto yTPos = wtSelY + surgeRoosterY + padMargin;
    addChild(TextDisplayLight::create(
                 rack::Vec(xTPos + 4, yTPos),
                 rack::Vec(box.size.x - xTPos - 2 * padMargin, 19 ),
                 module ? &(module->wtCategoryName) : nullptr,
                 15, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, surgeWhite()));

    for( int i=0; i<7; ++i )
    {
        if( i == 0 ) yTPos += 22 + padMargin;
        else if( i == 4 ) yTPos += 16 + padMargin;
        else yTPos += 14 + padMargin;

        float fs = 15 - ( i - 3 ) * ( i - 3 ) / 3.0;
        float colR = 255 - ( i - 3 ) * ( i - 3 ) * 10 - (i!=3?70:0);
        float colG = 255 - ( i - 3 ) * ( i - 3 ) * 10 - (i!=3?70:0);
        float colB = 255 - ( i - 3 ) * ( i - 3 ) * 8 - (i!=3?50:0);
        NVGcolor col = nvgRGB(colR, colG, colB );

        addChild(TextDisplayLight::create(
                     rack::Vec(xTPos + 4, yTPos),
                     rack::Vec(box.size.x - xTPos - 2 * padMargin, (i == 3 ? 16 : 14) ),
                     module ? &(module->wtItemName[i]) : nullptr,
                     fs, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, col ));
    }
 
}

rack::Model *modelSurgeWTOSC =
    rack::createModel<SurgeWTOSCWidget::M, SurgeWTOSCWidget>("SurgeWTOSC");
