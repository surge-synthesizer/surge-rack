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
    
    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        nvgBeginPath(vg);

        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());
        

        // Draw the output blue box
        nvgBeginPath(vg);
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin - 7;
        NVGpaint sideGradient;
        sideGradient = nvgLinearGradient(
            vg, x0 + ioMargin, SurgeLayout::orangeLine + ioMargin,
            x0 + ioMargin + ioRegionWidth, SurgeLayout::orangeLine + ioMargin,
            SurgeStyle::surgeBlueBright(), SurgeStyle::surgeBlue());
            
        nvgRoundedRect(
            vg, x0 + ioMargin, SurgeLayout::orangeLine + ioMargin, ioRegionWidth,
            box.size.y - SurgeLayout::orangeLine - 2 * ioMargin, ioMargin);
        nvgFillPaint(vg, sideGradient);
        nvgFill(vg);
        nvgStrokeColor(vg, SurgeStyle::backgroundGray());
        nvgStroke(vg);
        
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

        // Background behind the type light
        nvgBeginPath(vg);
        int text0 = padFromEdge + padMargin + SurgeLayout::surgeRoosterX;
        nvgRoundedRect(vg, text0, SCREW_WIDTH, 6*SCREW_WIDTH, SurgeLayout::surgeRoosterY, 5);
        NVGpaint gradient =
            nvgLinearGradient(vg, text0, SCREW_WIDTH, text0, SCREW_WIDTH+SurgeLayout::surgeRoosterY,
                              SurgeStyle::color2Bright(), SurgeStyle::color2());
        nvgFillPaint(vg, gradient);
        nvgFill(vg);
        nvgStrokeColor(vg, SurgeStyle::surgeOrange());
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 12);
        nvgFillColor(vg, nvgRGBA(0,0,0,255));
        nvgSave(vg);
        nvgTranslate(vg, SCREW_WIDTH*13,SCREW_WIDTH+padMargin);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgRotate(vg, M_PI / 2);
        nvgText(vg, 0, 0, "Pitch", NULL);
        nvgRestore(vg);

        
    }

    rack::Vec ioPortLocation(int ctrl) { // 0 is L; 1 is R; 2 is gain
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin - 7;

        int xRes = x0 + ioMargin + padFromEdge + (ctrl * (SurgeLayout::portX + 4));
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

    box.size = rack::Vec(SCREW_WIDTH * 24, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "SurgeOSC");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(0),
                                                   module,
                                                   M::OUTPUT_L));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(1),
                                                   module,
                                                   M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(
        ioPortLocation(2), module, M::OUTPUT_GAIN
#if !RACK_V1
        ,
        0, 1, 1
#endif
        ));


    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(padFromEdge, SCREW_WIDTH + padMargin), module, M::OSC_TYPE
#if !RACK_V1
                 ,0,4,0
#endif
                 ));
    addChild(TextDisplayLight::create(
                 rack::Vec(padFromEdge + 2 * padMargin + SurgeLayout::surgeRoosterX, SCREW_WIDTH + padMargin),
                 rack::Vec(100, SurgeLayout::surgeRoosterY ),
                 module ? module->oscNameCache.getValue : []() { return std::string("null"); },
                 module ? module->oscNameCache.getDirty : []() { return false; },
                 20, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT ) );

    addParam(rack::createParam<SurgeKnobRooster>(
                 rack::Vec(13 * SCREW_WIDTH, SCREW_WIDTH + padMargin), module, M::PITCH_0
#if !RACK_V1
                 ,1,127,72
#endif
                 ));
    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec(13 * SCREW_WIDTH + SurgeLayout::surgeRoosterX + padMargin, SCREW_WIDTH + padMargin), module, M::PITCH_CV ) );
    
}

#if RACK_V1
rack::Model *modelSurgeOSC =
    rack::createModel<SurgeOSCWidget::M, SurgeOSCWidget>("SurgeOSC");
#else
rack::Model *modelSurgeOSC = rack::createModel<SurgeOSCWidget::M, SurgeOSCWidget>(
    "Surge Team", "SurgeOSC", "SurgeOSC", rack::ENVELOPE_GENERATOR_TAG);
#endif
