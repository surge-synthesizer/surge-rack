#if BUILD_OLD_WIDGETS
#include "SurgeClock.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeClockWidget : SurgeModuleWidgetCommon
{
    typedef SurgeClock M;
    SurgeClockWidget(M *module);

    int labelHeight = 13;

    float bpmKnob = 50;
    float textHeight = 16;

    float pwmKnob = bpmKnob + 3 * padMargin + surgeRoosterY + 2 * textHeight + 20;

    int topOfInput = RACK_HEIGHT - 5 * padMargin - 3 * labelHeight - 2 * portY + textHeight;

    void addLabel(NVGcontext *vg, int yp, const char *label, NVGcolor col = panelLabel())
    {
        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgFillColor(vg, col);
        nvgText(vg, box.size.x / 2, yp, label, NULL);
    }
    void moduleBackground(NVGcontext *vg)
    {
        float yPos;

        addLabel(vg, bpmKnob, "BPM");
        drawTextBGRect(vg, padMargin, bpmKnob + 2 * padMargin + textHeight + surgeRoosterY,
                       box.size.x - 2 * padMargin, textHeight);

        addLabel(vg, pwmKnob, "GatePW");
        drawTextBGRect(vg, padMargin, pwmKnob + 2 * padMargin + textHeight + surgeRoosterY,
                       box.size.x - 2 * padMargin, textHeight);

        yPos = topOfInput;
        float sz = (portY + 2 * padMargin) / 2;
        drawBlueIORect(vg, box.size.x / 2 - sz, yPos, sz * 2,
                       2 * portY + 2 * labelHeight + 4 * padMargin);
        yPos += padMargin;
        addLabel(vg, yPos, "CV", ioRegionText());
        yPos += labelHeight + portY + padMargin;
        addLabel(vg, yPos, "Gate", ioRegionText());
    }
};

SurgeClockWidget::SurgeClockWidget(SurgeClockWidget::M *module) : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 3, RACK_HEIGHT);
    topOfInput = box.size.y - 5 * padMargin - 3 * labelHeight - 2 * portY;

    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "CLK");
    bg->narrowMode = true;
    bg->moduleSpecificDraw = [this](NVGcontext *vg) { this->moduleBackground(vg); };
    addChild(bg);

    float yPos;

    addParam(rack::createParam<SurgeKnobRooster>(
        rack::Vec(box.size.x / 2 - surgeRoosterX / 2, bpmKnob + textHeight + padMargin), module,
        M::CLOCK_CV));

    addChild(TextDisplayLight::create(
        rack::Vec(padFromEdge, bpmKnob + textHeight + 2 * padMargin + surgeRoosterY),
        rack::Vec(box.size.x - 2 * padFromEdge, textHeight), module ? &(module->bpmCache) : nullptr,
        11, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, parameterValueText_KEY()));

    addParam(rack::createParam<SurgeKnobRooster>(
        rack::Vec(box.size.x / 2 - surgeRoosterX / 2, pwmKnob + textHeight + padMargin), module,
        M::PULSE_WIDTH));

    addChild(TextDisplayLight::create(
        rack::Vec(padFromEdge, pwmKnob + textHeight + 2 * padMargin + surgeRoosterY),
        rack::Vec(box.size.x - 2 * padFromEdge, textHeight), module ? &(module->pwCache) : nullptr,
        11, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, parameterValueText_KEY()));

    yPos = topOfInput;
    yPos += labelHeight + padMargin;
    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(box.size.x / 2 - portX / 2, yPos),
                                                   module, M::CLOCK_CV_OUT));
    yPos += portY + labelHeight + 2 * padMargin;
    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(box.size.x / 2 - portX / 2, yPos),
                                                   module, M::GATE_OUT));
}

rack::Model *modelSurgeClock =
    rack::createModel<SurgeClockWidget::M, SurgeClockWidget>("SurgeClock");
#endif
