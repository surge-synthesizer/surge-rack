#include "SurgeWaveShaper.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeWaveShaperWidget : rack::ModuleWidget {
    typedef SurgeWaveShaper M;
    SurgeWaveShaperWidget(M *module);

    int roostery0 = 45;
    int drivey0 = 160;
    int textArea = 20;
    int labelHeight = 13;
    int padMargin = 3;
    int textMargin = 3;
    int driveOffset = 20;

    int fontId = -1;
    int topOfInput = RACK_HEIGHT - 5 * padMargin - 3 * labelHeight - 2 * SurgeLayout::portY;
    
    void addLabel(NVGcontext *vg, int yp, const char *label,
                  NVGcolor col = SurgeStyle::surgeBlue()) {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 12);
        nvgFillColor(vg, col);
        nvgText(vg, box.size.x / 2, yp, label, NULL);
    }
    void moduleBackground(NVGcontext *vg) {
        int yPos = roostery0 - labelHeight - padMargin;
        addLabel(vg, yPos, "Type");

        yPos = drivey0;
        addLabel(vg, yPos, "Drive");

        yPos += labelHeight + SurgeLayout::surgeRoosterY + padMargin +
                SurgeLayout::portY + 4 * padMargin;
        SurgeStyle::drawTextBGRect(vg, textMargin, yPos,
                                   box.size.x - 2 * textMargin, textArea);
        yPos += padMargin;

        yPos += textArea + padMargin;
        int sz = SurgeLayout::portX / 2 + 2 * padMargin;

        yPos = topOfInput;
        SurgeStyle::drawBlueIORect(vg, box.size.x / 2 - sz, yPos, sz * 2,
                                   2 * SurgeLayout::portY + 2 * labelHeight +
                                       4 * padMargin);
        yPos += padMargin;
        addLabel(vg, yPos, "Input", SurgeStyle::surgeWhite());
        yPos += labelHeight + SurgeLayout::portY + padMargin;
        addLabel(vg, yPos, "Output", SurgeStyle::surgeWhite());
    }
};

SurgeWaveShaperWidget::SurgeWaveShaperWidget(SurgeWaveShaperWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 3, RACK_HEIGHT);
    topOfInput = box.size.y - 5 * padMargin - 3 * labelHeight - 2 * SurgeLayout::portY;

    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "WS");
    bg->narrowMode = true;
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    int yPos = roostery0;

    SurgeButtonBank *bank = SurgeButtonBank::create( rack::Vec(padMargin, yPos),
                                                     rack::Vec(box.size.x - 2 * padMargin, n_ws_type * 15 ),
                                                     module, M::MODE_PARAM,
                                                     1, n_ws_type, 13 );

    bank->addLabel("none");
    bank->addLabel("tanh");
    bank->addLabel("hard");
    bank->addLabel("asym");
    bank->addLabel("sinus");
    bank->addLabel("digi");

    addParam( bank );
    

    yPos = drivey0 + labelHeight + padMargin;
    addParam(rack::createParam<SurgeKnobRooster>(
        rack::Vec(box.size.x / 2 - SurgeLayout::surgeRoosterX / 2, yPos),
        module, M::DRIVE_PARAM
#if !RACK_V1
        ,
        -24.0, 0, 24.0
#endif
        ));
    yPos += SurgeLayout::surgeRoosterY + 2 * padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - SurgeLayout::portX / 2, yPos), module,
        M::DRIVE_CV));

    yPos += SurgeLayout::portY + padMargin;

    addChild(TextDisplayLight::create(
        rack::Vec(textMargin, yPos + 2),
        rack::Vec(box.size.x - 2 * textMargin, textArea),
        module ? module->dbGainCache.getValue
               : []() { return std::string("null"); },
        module ? module->dbGainCache.getDirty : []() { return false; }, 10,
        NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER,
        SurgeStyle::surgeWhite()
                 ));

    yPos += textArea + padMargin;
    yPos += padMargin;

    yPos = topOfInput;
    yPos += labelHeight + padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - SurgeLayout::portX / 2, yPos), module,
        M::SIGNAL_IN));
    yPos += SurgeLayout::portY + labelHeight + 2 * padMargin;
    addOutput(rack::createOutput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - SurgeLayout::portX / 2, yPos), module,
        M::SIGNAL_OUT));
}

#if RACK_V1
rack::Model *modelSurgeWaveShaper =
    rack::createModel<SurgeWaveShaperWidget::M, SurgeWaveShaperWidget>(
        "SurgeWaveShaper");
#else
rack::Model *modelSurgeWaveShaper =
    rack::createModel<SurgeWaveShaperWidget::M, SurgeWaveShaperWidget>(
        "Surge Team", "SurgeWaveShaper", "SurgeWaveShaper",
        rack::ENVELOPE_GENERATOR_TAG);
#endif
