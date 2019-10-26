#include "SurgeNoise.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeNoiseWidget : SurgeModuleWidgetCommon {
    typedef SurgeNoise M;
    SurgeNoiseWidget(M *module);

    int buttonsY0 = padFromTop;
    int drivey0 = 160;
    int textArea = 20;
    int labelHeight = 13;
    int driveOffset = 20;

    int topOfInput = orangeLine + padMargin;
    
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

        yPos -= 80;
        addLabel(vg, yPos, "Color");

        yPos += 80;
        
        yPos += labelHeight + surgeRoosterY + padMargin +
                portY + 4 * padMargin;
        yPos += padMargin;

        yPos += textArea + padMargin;
        int sz = portX / 2 + 2 * padMargin;

        yPos = orangeLine + ioMargin;
        drawBlueIORect(vg, box.size.x / 2 - sz, yPos, sz * 2,
                       box.size.y - orangeLine - 2*ioMargin );
        yPos += padMargin;

        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgFillColor(vg, ioRegionText());
        nvgText(vg, box.size.x / 2, box.size.y - ioMargin - 1.5, "Noise", NULL);


        yPos += labelHeight + portY + padMargin;
    }
};

SurgeNoiseWidget::SurgeNoiseWidget(SurgeNoiseWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 3, RACK_HEIGHT);
    topOfInput = box.size.y - 5 * padMargin - 3 * labelHeight - 2 * portY;

    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "NZ");
    bg->narrowMode = true;
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    int yPos = buttonsY0;
    

    yPos = drivey0 + labelHeight + padMargin;
    yPos -= 80;
    addParam(rack::createParam<SurgeKnobRooster>(
        rack::Vec(box.size.x / 2 - surgeRoosterX / 2, yPos),
        module, M::CORRELATION_PARAM
        ));
    yPos += surgeRoosterY + 2 * padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - portX / 2, yPos), module,
        M::CORRELATION_CV));

    yPos += 80;
    yPos += portY + padMargin;

    yPos += textArea + padMargin;
    yPos += padMargin;

    yPos = orangeLine + 1.5 * ioMargin; 
    addOutput(rack::createOutput<rack::PJ301MPort>(
        rack::Vec(box.size.x / 2 - portX / 2, yPos), module,
        M::SIGNAL_OUT));
}

rack::Model *modelSurgeNoise =
    rack::createModel<SurgeNoiseWidget::M, SurgeNoiseWidget>(
        "SurgeNoise");
