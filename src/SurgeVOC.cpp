#include "SurgeVOC.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeVOCWidget : SurgeModuleWidgetCommon {
    typedef SurgeVOC M;
    SurgeVOCWidget(M *module);


    int fontId = -1;
    void moduleBackground(NVGcontext *vg) {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGBA(255,0,0,255));
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 18);
        nvgSave(vg);
        nvgTranslate(vg, SCREW_WIDTH * 9, RACK_HEIGHT / 2.4 );
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgRotate(vg, -M_PI / 4);
        nvgText(vg, 0, 0, "SurgeVOC", NULL );
        nvgText(vg, 0, 22, "Under Construction", NULL);
        nvgRestore(vg);
    }
};

SurgeVOCWidget::SurgeVOCWidget(SurgeVOCWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 20, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "VOC");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);
}

rack::Model *modelSurgeVOC =
    rack::createModel<SurgeVOCWidget::M, SurgeVOCWidget>("SurgeVOC");
