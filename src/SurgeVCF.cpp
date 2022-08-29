//
// Created by Paul Walker on 8/29/22.
//

#include "SurgeVCF.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeVCFWidget : SurgeModuleWidgetCommon
{
    typedef SurgeVCF M;
    SurgeVCFWidget(M *module);

    void moduleBackground(NVGcontext *vg) {}
};

SurgeVCFWidget::SurgeVCFWidget(SurgeVCFWidget::M *module) : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 9, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "VCF");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) { this->moduleBackground(vg); };
    addChild(bg);

    addParam(rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 20), module, M::A_PARAM));
    addParam(
        rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 80), module, M::A_MOD1_DEPTH));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100,80), module, M::MOD_1));

    addParam(
        rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 140), module, M::A_MOD2_DEPTH));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100,140), module, M::MOD_2));

    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(20, 200), module, M::SIGNAL_OUT));
}

rack::Model *modelSurgeVCF = rack::createModel<SurgeVCFWidget::M, SurgeVCFWidget>("SurgeVCF");
