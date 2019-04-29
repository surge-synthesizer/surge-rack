#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeFXWidget : rack::ModuleWidget {
    typedef SurgeFX<rack::Module> M;
    SurgeFXWidget(M *module);
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

    box.size = rack::Vec(SCREW_WIDTH * 40, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "SurgeFX");
    bg->hasInput = true;
    bg->hasOutput = true;

    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(
        bg->ioPortLocation(true, false), module, M::INPUT_R_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(bg->ioPortLocation(true, true),
                                                 module, M::INPUT_L));

    addOutput(rack::createOutput<rack::PJ301MPort>(
        bg->ioPortLocation(false, false), module, M::OUTPUT_R_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(
        bg->ioPortLocation(false, true), module, M::OUTPUT_L));

    int parmMargin = 3;

    for (int i = 0; i < 6; ++i) {
        int pos = 60 + i * (SurgeParamLargeWidget::height + parmMargin);
        int x0 = 0;
        addChild(SurgeParamLargeWidget::create(
            this, module, rack::Vec(SCREW_WIDTH * 0.5, pos), M::FX_PARAM_0 + i,
            M::FX_PARAM_INPUT_0 + i, -1,
            [module, i]() { return module ? module->getLabel(i) : "null"; },
            []() { return true; },
            [module, i]() { return module ? module->getSubLabel(i) : "null"; },
            []() { return true; },
            [module, i]() {
                return module ? module->getValueString(i) : "null";
            },
            []() { return true; }));

        addChild(SurgeParamLargeWidget::create(
            this, module, rack::Vec(20.5 * SCREW_WIDTH, pos),
            M::FX_PARAM_0 + i + 6, M::FX_PARAM_INPUT_0 + i + 6, -1,
            [module, i]() { return module ? module->getLabel(i + 6) : "null"; },
            []() { return true; },
            [module, i]() {
                return module ? module->getSubLabel(i + 6) : "null";
            },
            []() { return true; },
            [module, i]() {
                return module ? module->getValueString(i + 6) : "null";
            },
            []() { return true; }));
    }
}

#if RACK_V1
rack::Model *modelSurgeFX =
    rack::createModel<SurgeFXWidget::M, SurgeFXWidget>("SurgeFX");
#else
rack::Model *modelSurgeFX = rack::createModel<SurgeFXWidget::M, SurgeFXWidget>(
    "Surge Team", "SurgeRack", "SurgeFX", rack::EFFECT_TAG);
#endif
