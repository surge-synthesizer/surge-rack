#include "SurgeWTOSC.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeWTOSCWidget : rack::ModuleWidget {
    typedef SurgeWTOSC M;
    SurgeWTOSCWidget(M *module);


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
        nvgText(vg, 0, 0, "SurgeWTOSC", NULL );
        nvgText(vg, 0, 22, "Under Construction", NULL);
        nvgRestore(vg);
    }
};

SurgeWTOSCWidget::SurgeWTOSCWidget(SurgeWTOSCWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 20, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "WT");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);
}

#if RACK_V1
rack::Model *modelSurgeWTOSC =
    rack::createModel<SurgeWTOSCWidget::M, SurgeWTOSCWidget>("SurgeWTOSC");
#else
rack::Model *modelSurgeWTOSC =
    rack::createModel<SurgeWTOSCWidget::M, SurgeWTOSCWidget>(
        "Surge Team", "SurgeWTOSC", "SurgeWTOSC", rack::ENVELOPE_GENERATOR_TAG);
#endif
