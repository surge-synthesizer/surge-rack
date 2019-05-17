#include "SurgeADSR.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeADSRWidget : SurgeModuleWidgetCommon {
    typedef SurgeADSR M;
    SurgeADSRWidget(M *module);

    int adsrHeight = 52;
    int adsrSpace = 10;
    int adsrTextH = 18;

    int modeYPos = 35;

    float inputXPos(int which) {
        float xSize = box.size.x - 2 * padFromEdge;
        float perI = xSize / 3.0;
        float posn = padFromEdge + (which + 0.5) * perI - portX / 2;
        return posn;
    }

    float inputYPos(int which) {
        return orangeLine + ioMargin + padMargin;
    }

    float ADSRYPos(int which) {
        int endOfIn = modeYPos + 21 + padMargin; // what is that 21
        return endOfIn + (adsrHeight + adsrSpace) * which + 12;
    }

    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        float ioyb = orangeLine + ioMargin;
        drawBlueIORect(
            vg, padFromEdge, ioyb, box.size.x - 2 * padFromEdge,
            box.size.y - orangeLine - 2 * ioMargin);

        int iotxt = box.size.y - ioMargin - 1.5;
        nvgBeginPath(vg);
        nvgFillColor(vg, surgeWhite());
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);
        nvgText(vg, inputXPos(0) + portX / 2, iotxt, "Gate", NULL);

        nvgBeginPath(vg);
        nvgText(vg, inputXPos(1) + portX / 2, iotxt, "Retrig",
                NULL);

        nvgBeginPath(vg);
        nvgText(vg, inputXPos(2) + portX / 2, iotxt, "Out", NULL);

        std::vector<std::string> lab = {"A", "D", "S", "R"};
        for (int i = 0; i < 4; ++i) {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 20);
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);

            nvgFillColor(vg, surgeBlue());
            nvgText(vg, padFromEdge  + 5, ADSRYPos(i), lab[i].c_str(), NULL);
            drawTextBGRect(vg, padFromEdge * 2 + 12, ADSRYPos(i) + 1,
                                       box.size.x - padFromEdge * 3 - 20,
                                       adsrTextH);
            nvgBeginPath(vg);
            nvgMoveTo(vg, padFromEdge  + 5, ADSRYPos(i) + 20 + padMargin);

            nvgLineTo(vg, padFromEdge  + 5,
                      ADSRYPos(i) + adsrTextH + padMargin + 3 +
                          portY / 2);
            nvgLineTo(vg, (i == 2) ? 55 : 85,
                      ADSRYPos(i) + adsrTextH + padMargin + 3 +
                          portY / 2);

            nvgStrokeColor(vg, surgeBlue());
            nvgStroke(vg);
        }

        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
        nvgFillColor(vg, surgeBlue());
        nvgText(vg, box.size.x / 2 - 9, modeYPos + 21 / 2.0, "Mode", NULL);

        nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT);
        nvgText(vg, box.size.x / 2 + 9, modeYPos + 21 / 2.0 - 5, "Digi", NULL);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
        nvgText(vg, box.size.x / 2 + 9, modeYPos + 21 / 2.0 + 5, "Anlg", NULL);
    }
};

SurgeADSRWidget::SurgeADSRWidget(SurgeADSRWidget::M *module)
    : SurgeModuleWidgetCommon(
#ifndef RACK_V1
          module
#endif
        )
#if !RACK_V1
    , rack::ModuleWidget(module) // why do I need this gcc? SMWC calls it.
#endif    
{
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 7, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "ADSR");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(inputXPos(0), inputYPos(0)), module, M::GATE_IN));
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(inputXPos(1), inputYPos(1)), module, M::RETRIG_IN));
    addOutput(rack::createOutput<rack::PJ301MPort>(
        rack::Vec(inputXPos(2), inputYPos(2)), module, M::OUTPUT_ENV));

    int envStart = padFromTop + 24.6 * 2 + 15 + padFromTop + 30;
    int envHeight = 40;
    int x0 = 18;

    for (int i = 0; i < 4; ++i) {
        addChild(TextDisplayLight::create(
            rack::Vec(padFromEdge * 2 + 15, ADSRYPos(i) +2),
            rack::Vec(box.size.x - padFromEdge * 3 - 20, adsrTextH),
            module ? module->adsrStrings[i].getValue : []() { return "null"; },
            module ? module->adsrStrings[i].getDirty : []() { return false; },
            13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, surgeWhite()));
    }

    for (int i = M::A_PARAM; i <= M::R_PARAM; ++i) {
        int ipos = i - M::A_PARAM;
        addParam(rack::createParam<SurgeSmallKnob>(
                     rack::Vec(x0 + portX + padMargin, ADSRYPos(ipos) + adsrTextH + padMargin + 3),
            module, i
#if !RACK_V1
            ,
            0, 1, 0.5
#endif

            ));
    }

    for (int i = M::A_CV; i <= M::R_CV; ++i) {
        int ipos = i - M::A_CV;
        addInput(rack::createInput<rack::PJ301MPort>(
            rack::Vec(x0, ADSRYPos(ipos) + adsrTextH + padMargin + 3), module,
            i));
    }

    for (int i = M::A_S_PARAM; i <= M::R_S_PARAM; ++i) {
        int ipos = i - M::A_S_PARAM;
        if (i == M::R_S_PARAM)
            ipos++;

        addParam(rack::createParam<rack::CKSSThree>(
                     rack::Vec(x0 + 2 * portX + 3 * padMargin, ADSRYPos(ipos) + adsrTextH + padMargin + 1),
            module, i
#if !RACK_V1
            ,
            0, 2, 0
#endif

            ));
        addChild(rack::createLight<rack::SmallLight<rack::GreenLight>>(
            rack::Vec(x0 + 60 + 14 + padMargin,
                      ADSRYPos(ipos) + adsrTextH + padMargin),
            module, M::DIGI_LIGHT));
    }

    addParam(rack::createParam<SurgeSwitchFull>(
        rack::Vec(box.size.x / 2 - 7, modeYPos), module, M::MODE_PARAM
#if !RACK_V1
        ,
        0, 1, 0
#endif
        ));
}

#if RACK_V1
rack::Model *modelSurgeADSR =
    rack::createModel<SurgeADSRWidget::M, SurgeADSRWidget>("SurgeADSR");
#else
rack::Model *modelSurgeADSR =
    rack::createModel<SurgeADSRWidget::M, SurgeADSRWidget>(
        "Surge Team", "SurgeADSR", "SurgeADSR", rack::ENVELOPE_GENERATOR_TAG);
#endif
