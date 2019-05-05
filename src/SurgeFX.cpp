#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeFXWidget : rack::ModuleWidget {
    typedef SurgeFX M;
    SurgeFXWidget(M *module);

    int ioMargin = 7;
    int ioRegionWidth = 105;
    int fontId = -1;

    void moduleBackground(NVGcontext *vg) {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

        for (int i = 0; i < 2; ++i) {
            nvgBeginPath(vg);
            int x0 = 7;
            if (i == 1)
                x0 = box.size.x - ioRegionWidth - 2 * ioMargin - 7;

            SurgeStyle::drawBlueIORect(
                vg, x0 + ioMargin, SurgeLayout::orangeLine + ioMargin,
                ioRegionWidth,
                box.size.y - SurgeLayout::orangeLine - 2 * ioMargin,
                (i == 0) ? 0 : 1);

            nvgFillColor(vg, SurgeStyle::backgroundGray());
            nvgFontFaceId(vg, fontId);
            nvgFontSize(vg, 12);
            if (i == 0) {
                nvgSave(vg);
                nvgTranslate(vg, x0 + ioMargin + 2,
                             SurgeLayout::orangeLine + ioMargin * 1.5);
                nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgRotate(vg, -M_PI / 2);
                nvgText(vg, 0, 0, "Input", NULL);
                nvgRestore(vg);
            } else {
                nvgSave(vg);
                nvgTranslate(vg, x0 + ioMargin + ioRegionWidth - 2,
                             SurgeLayout::orangeLine + ioMargin * 1.5);
                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgRotate(vg, M_PI / 2);
                nvgText(vg, 0, 0, "Output", NULL);
                nvgRestore(vg);
            }
            rack::Vec ll;
            ll = ioPortLocation(i == 0, 0);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "L/Mon", NULL);

            ll = ioPortLocation(i == 0, 1);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "R", NULL);

            ll = ioPortLocation(i == 0, 2);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "Gain", NULL);
        }

        SurgeStyle::drawTextBGRect(vg, box.size.x / 4 + 36, 15,
                                   box.size.x / 2 - SCREW_WIDTH / 2 - 36, 34);
    }

    rack::Vec ioPortLocation(bool input,
                             int ctrl) { // 0 is L; 1 is R; 2 is gain
        int x0 = 7;
        if (!input)
            x0 = box.size.x - ioRegionWidth - 2 * ioMargin - 7;

        int padFromEdge = input ? 17 : 5;
        int xRes =
            x0 + ioMargin + padFromEdge + (ctrl * (SurgeLayout::portX + 4));
        int yRes = SurgeLayout::orangeLine + 1.5 * ioMargin;

        return rack::Vec(xRes, yRes);
    }
};

SurgeFXWidget::SurgeFXWidget(SurgeFXWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 29, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "FX");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };

    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0),
                                                 module, M::INPUT_R_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1),
                                                 module, M::INPUT_L));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2), module,
                                               M::INPUT_GAIN
#if !RACK_V1
                                               ,
                                               0, 1, 1
#endif
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
        ioPortLocation(false, 0), module, M::OUTPUT_R_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1),
                                                   module, M::OUTPUT_L));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2), module,
                                               M::OUTPUT_GAIN
#if !RACK_V1
                                               ,
                                               0, 1, 1
#endif

                                               ));
    int parmMargin = 3;

    addChild(TextDisplayLight::create(
        rack::Vec(box.size.x / 4 + 2 + 36, 17),
        rack::Vec(box.size.x / 2 - SCREW_WIDTH / 2 - 36, 31),
        [module]() { return module ? module->getEffectNameString() : "null"; },
        [module]() {
            return module ? module->getEffectNameStringDirty() : true;
        },
        20, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT, SurgeStyle::surgeWhite()));
    TextDisplayLight *tdl;
    addChild(tdl = TextDisplayLight::create(
                 rack::Vec(box.size.x / 4 + 2 + 36, 16),
                 rack::Vec(box.size.x / 2 - SCREW_WIDTH / 2 - 36, 31),
                 []() { return "Effect Type"; }, []() { return false; }, 12,
                 NVG_ALIGN_TOP | NVG_ALIGN_LEFT));
    tdl->font = SurgeStyle::fontFaceCondensed();

    addParam(rack::createParam<SurgeKnobRooster>(rack::Vec(box.size.x / 4, 15),
                                                 module, M::FX_TYPE
#ifndef RACK_V1
                                                 ,
                                                 0, 10, 1
#endif
                                                 ));

    for (int i = 0; i < 6; ++i) {
        int pos = 60 + i * (SurgeParamLargeWidget::height + parmMargin);
        int x0 = 0;
        addChild(SurgeParamLargeWidget::create(
            this, module, rack::Vec(SCREW_WIDTH * 0.5, pos), M::FX_PARAM_0 + i,
            M::FX_PARAM_GAIN_0 + i, M::FX_PARAM_INPUT_0 + i, M::FX_EXTEND_0 + i,
            [module, i]() { return module ? module->getLabel(i) : "null"; },
            [module, i]() { return module ? module->getLabelDirty(i) : false; },

            [module, i]() { return module ? module->getSubLabel(i) : "null"; },
            [module, i]() {
                return module ? module->getSubLabelDirty(i) : false;
            },

            module ? module->paramDisplayCache[i].getValue
                   : []() { return std::string("null"); },
            module ? module->paramDisplayCache[i].getDirty
                   : []() { return false; }));

        addChild(SurgeParamLargeWidget::create(
            this, module, rack::Vec(14.5 * SCREW_WIDTH, pos),
            M::FX_PARAM_0 + i + 6, M::FX_PARAM_GAIN_0 + i + 6,
            M::FX_PARAM_INPUT_0 + i + 6, M::FX_EXTEND_0 + 6 + i,
            [module, i]() { return module ? module->getLabel(i + 6) : "null"; },
            [module, i]() {
                return module ? module->getLabelDirty(i + 6) : false;
            },
            [module, i]() {
                return module ? module->getSubLabel(i + 6) : "null";
            },
            [module, i]() {
                return module ? module->getSubLabelDirty(i + 6) : false;
            },
            module ? module->paramDisplayCache[i + 6].getValue
                   : []() { return std::string("null"); },
            module ? module->paramDisplayCache[i + 6].getDirty
                   : []() { return false; }));
    }
}

#if RACK_V1
rack::Model *modelSurgeFX =
    rack::createModel<SurgeFXWidget::M, SurgeFXWidget>("SurgeFX");
#else
rack::Model *modelSurgeFX = rack::createModel<SurgeFXWidget::M, SurgeFXWidget>(
    "Surge Team", "SurgeFX", "SurgeFX", rack::EFFECT_TAG);
#endif
