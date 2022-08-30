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

<<<<<<< Updated upstream
    addParam(rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 20), module, M::A_PARAM));
    addParam(
        rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 80), module, M::A_MOD1_DEPTH));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100,80), module, M::MOD_1));

    addParam(
        rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 140), module, M::A_MOD2_DEPTH));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100,140), module, M::MOD_2));
=======
    // knobA = SurgeModulatableKnob::create(rack::Vec(20, 20), module, M::A_PARAM);
    knobA = rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 20), module, M::A_PARAM);
    modA_1 = SurgeModulatableRing::create(rack::Vec(20, 20), module, M::A_MOD1_DEPTH);
    modA_1->underlyerParamWidget = knobA;
    modA_2 = SurgeModulatableRing::create(rack::Vec(20, 20), module, M::A_MOD2_DEPTH);
    modA_2->underlyerParamWidget = knobA;
    addChild(knobA);
    modA_1->setVisible(false);
    modA_2->setVisible(false);
    
    addChild(modA_1);
    addChild(modA_2);

    addParam(rack::createParam<rack::RoundBigBlackKnob>(rack::Vec(20, 80), module, M::A_PARAM));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(100, 80), module, M::MOD_1));

    toggle1 = rack::createWidget<SurgeUIOnlyToggleButton>(rack::Vec(75, 80));
    toggle1->pressedState = false;
    toggle1->onToggle = [this](bool isOn) {
        if (isOn)
        {
            toggle2->pressedState = false;
            toggle2->bdw->dirty = true;

            modA_1->setVisible(true);
            modA_2->setVisible(false);
            modA_1->bdw->dirty = true;
        }
        else
        {
            modA_1->setVisible(false);
            modA_2->setVisible(false);
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
            toggle1->pressedState = false;
            toggle1->bdw->dirty = true;

            modA_1->setVisible(false);
            modA_2->setVisible(true);
            modA_2->bdw->dirty = true;
        }
        else
        {
            modA_1->setVisible(false);
            modA_2->setVisible(false);
        }
    };
    addChild(toggle2);
>>>>>>> Stashed changes

    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(20, 200), module, M::SIGNAL_OUT));
}

rack::Model *modelSurgeVCF = rack::createModel<SurgeVCFWidget::M, SurgeVCFWidget>("SurgeVCF");
