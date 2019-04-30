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

    box.size = rack::Vec(SCREW_WIDTH * 25, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "SurgeFX");
    bg->hasInput = true;
    bg->hasOutput = true;

    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(bg->ioPortLocation(true, 0),
                                                 module, M::INPUT_R_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(bg->ioPortLocation(true, 1),
                                                 module, M::INPUT_L));
    addParam(rack::createParam<rack::RoundSmallBlackKnob>(
        bg->ioPortLocation(true, 2), module, M::INPUT_GAIN
#if !RACK_V1
        ,
        0, 1, 1
#endif
        ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
        bg->ioPortLocation(false, 0), module, M::OUTPUT_R_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(bg->ioPortLocation(false, 1),
                                                   module, M::OUTPUT_L));
    addParam(rack::createParam<rack::RoundSmallBlackKnob>(
        bg->ioPortLocation(false, 2), module, M::OUTPUT_GAIN
#if !RACK_V1
        ,
        0, 1, 1
#endif

        ));

    int parmMargin = 3;

    addChild(new SurgeRoundedRect(rack::Vec(SCREW_WIDTH*6.5,10),
                                  rack::Vec(SCREW_WIDTH*18,45)));

    addChild(TextDisplayLight::create(rack::Vec(SCREW_WIDTH*6.5 + 3, 13),
                                      rack::Vec(SCREW_WIDTH*18, 39),
                                      [module]() { return module ? module->getEffectNameString() : "null"; },
                                      [module]() { return module ? module->getEffectNameStringDirty() : true; },
                                      25, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT));
    addParam(rack::createParam<rack::RoundLargeBlackKnob>(
                 rack::Vec(SCREW_WIDTH*1, 4), module, M::FX_TYPE
#ifndef RACK_V1
                 , 0, 10, 1
#endif                 
                 ));
    
    for (int i = 0; i < 6; ++i) {
        int pos = 60 + i * (SurgeParamLargeWidget::height + parmMargin);
        int x0 = 0;
        addChild(SurgeParamLargeWidget::create(
            this, module, rack::Vec(SCREW_WIDTH * 0.5, pos), M::FX_PARAM_0 + i,
            M::FX_PARAM_INPUT_0 + i, M::FX_EXTEND_0 + i,
            [module, i]() { return module ? module->getLabel(i) : "null"; },
            []() { return true; },
            [module, i]() { return module ? module->getSubLabel(i) : "null"; },
            []() { return true; },
            [module, i]() {
                return module ? module->getValueString(i) : "null";
            },
            []() { return true; }));

        addChild(SurgeParamLargeWidget::create(
            this, module, rack::Vec(12.5 * SCREW_WIDTH, pos),
            M::FX_PARAM_0 + i + 6, M::FX_PARAM_INPUT_0 + i + 6, M::FX_EXTEND_0 + 6 + i,
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
