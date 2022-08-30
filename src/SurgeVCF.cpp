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

    rack::app::Knob *knobA{nullptr};
    SurgeModulatableRing *modA_1{nullptr};
    SurgeModulatableRing *modA_2{nullptr};

    SurgeUIOnlyToggleButton *toggle1{nullptr}, *toggle2{nullptr};

    void moduleBackground(NVGcontext *vg) {}
};

SurgeVCFWidget::SurgeVCFWidget(SurgeVCFWidget::M *module) : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 9, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "VCF");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) { this->moduleBackground(vg); };
    addChild(bg);

    // knobA = SurgeModulatableKnob::create(rack::Vec(20, 20), module, M::A_PARAM);
    knobA = rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 20), module, M::A_PARAM);
    modA_1 = SurgeModulatableRing::create(rack::Vec(20, 20), module, M::A_MOD1_DEPTH);
    modA_1->underlyerParamWidget = knobA;
    modA_2 = SurgeModulatableRing::create(rack::Vec(20, 20), module, M::A_MOD2_DEPTH);
    modA_2->underlyerParamWidget = knobA;
    addChild(knobA);
    addParam(rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 80), module, M::A_PARAM));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100, 80), module, M::MOD_1));

    toggle1 = rack::createWidget<SurgeUIOnlyToggleButton>(rack::Vec(75, 80));
    toggle1->pressedState = false;
    toggle1->onToggle = [this](bool isOn) {
        if (isOn)
        {
            if (hasChild(modA_2))
                removeChild(modA_2);
            toggle2->pressedState = false;
            toggle2->bdw->dirty = true;
            addChild(modA_1);
            modA_1->bdw->dirty = true;
        }
        else
        {
            if (hasChild(modA_1))
                removeChild(modA_1);
            if (hasChild(modA_2))
                removeChild(modA_2);
        }
    };
    addChild(toggle1);

    addParam(
        rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 140), module, M::A_MOD2_DEPTH));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100, 140), module, M::MOD_2));

    toggle2 = rack::createWidget<SurgeUIOnlyToggleButton>(rack::Vec(75, 140));
    toggle2->pressedState = false;
    toggle2->onToggle = [this](bool isOn) {
        if (isOn)
        {
            if (hasChild(modA_1))
                removeChild(modA_1);
            toggle1->pressedState = false;
            toggle1->bdw->dirty = true;
            addChild(modA_2);
            modA_2->bdw->dirty = true;
        }
        else
        {
            if (hasChild(modA_1))
                removeChild(modA_1);
            if (hasChild(modA_2))
                removeChild(modA_2);
        }
    };
    addChild(toggle2);

    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(20, 200), module, M::SIGNAL_OUT));
}

rack::Model *modelSurgeVCF = rack::createModel<SurgeVCFWidget::M, SurgeVCFWidget>("SurgeVCF");
