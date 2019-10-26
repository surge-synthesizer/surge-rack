#include "SurgeWaveShaper.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeWaveShaperWidget : SurgeModuleWidgetCommon {
    typedef SurgeWaveShaper M;
    SurgeWaveShaperWidget(M *module);

    int buttonsY0 = padFromTop;
    int drivey0 = 160;
    int textArea = 20;
    int labelHeight = 13;
    int driveOffset = 20;

    int topOfInput = RACK_HEIGHT - 5 * padMargin - 3 * labelHeight - 2 * portY;
    
    void addLabel(NVGcontext *vg, int yp, const char *label,
                  NVGcolor col = panelLabel()) {
        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgFillColor(vg, col);
        nvgText(vg, box.size.x / 2, yp, label, NULL);
    }
    void moduleBackground(NVGcontext *vg) {
        float yPos = drivey0;
        addLabel(vg, yPos, "Drive");

        yPos += labelHeight + surgeRoosterY + padMargin +
                portY + 4 * padMargin;
        drawTextBGRect(vg, padFromEdge, yPos,
                                   box.size.x - 2 * padFromEdge, textArea);
        yPos += padMargin;

        yPos += textArea + padMargin;
        int sz = portX / 2 + 2 * padMargin;

        yPos = topOfInput;
        drawBlueIORect(vg, box.size.x / 2 - sz, yPos, sz * 2,
                                   2 * portY + 2 * labelHeight +
                                       4 * padMargin);
        yPos += padMargin;
        addLabel(vg, yPos, "Input", ioRegionText());
        yPos += labelHeight + portY + padMargin;
        addLabel(vg, yPos, "Output", ioRegionText());
    }
};

SurgeWaveShaperWidget::SurgeWaveShaperWidget(SurgeWaveShaperWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 3, RACK_HEIGHT);
    topOfInput = box.size.y - 5 * padMargin - 3 * labelHeight - 2 * portY;

    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "WS");
    bg->narrowMode = true;
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    int yPos = buttonsY0;

    SurgeButtonBank *bank = SurgeButtonBank::create( rack::Vec(padMargin, yPos),
                                                     rack::Vec(box.size.x - 2 * padMargin, n_ws_type * 23 ),
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
        rack::Vec(box.size.x / 2 - surgeRoosterX / 2, yPos),
        module, M::DRIVE_PARAM
        ));
    yPos += surgeRoosterY + 2 * padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - portX / 2, yPos), module,
        M::DRIVE_CV));

    yPos += portY + padMargin;

    addChild(TextDisplayLight::create(
        rack::Vec(padFromEdge, yPos + 2),
        rack::Vec(box.size.x - 2 * padFromEdge, textArea),
        module ? module->dbGainCache.getValue
               : []() { return std::string("null"); },
        module ? module->dbGainCache.getDirty : []() { return false; }, 10,
        NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER,
        parameterValueText()
                 ));

    yPos += textArea + padMargin;
    yPos += padMargin;

    yPos = topOfInput;
    yPos += labelHeight + padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - portX / 2, yPos), module,
        M::SIGNAL_IN));
    yPos += portY + labelHeight + 2 * padMargin;
    addOutput(rack::createOutput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - portX / 2, yPos), module,
        M::SIGNAL_OUT));
}

rack::Model *modelSurgeWaveShaper =
    rack::createModel<SurgeWaveShaperWidget::M, SurgeWaveShaperWidget>(
        "SurgeWaveShaper");
