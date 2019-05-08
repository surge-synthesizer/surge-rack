#include "SurgeVCF.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeVCFWidget : rack::ModuleWidget {
    typedef SurgeVCF M;
    SurgeVCFWidget(M *module);


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
        nvgText(vg, 0, 0, "SurgeVCF", NULL );
        nvgText(vg, 0, 22, "Under Construction", NULL);
        nvgRestore(vg);
    }
};

SurgeVCFWidget::SurgeVCFWidget(SurgeVCFWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 20, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "VCF");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);
}

#if RACK_V1
rack::Model *modelSurgeVCF =
    rack::createModel<SurgeVCFWidget::M, SurgeVCFWidget>("SurgeVCF");
#else
rack::Model *modelSurgeVCF =
    rack::createModel<SurgeVCFWidget::M, SurgeVCFWidget>(
        "Surge Team", "SurgeVCF", "SurgeVCF", rack::ENVELOPE_GENERATOR_TAG);
#endif
