//
// Created by Paul Walker on 9/20/22.
//

#include "Mixer.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::mixer::ui
{
struct MixerWidget : widgets::XTModuleWidget, widgets::VCOVCFConstants
{
    typedef mixer::Mixer M;
    MixerWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, M::n_mixer_params> overlays;
    std::array<widgets::KnobN *, M::n_mixer_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

MixerWidget::MixerWidget(MixerWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "Mixer", "other", "TotalBlank");
    addChild(bg);

    int kr{0}, kc{0};
    for (int i = M::OSC1_LEV; i <= M::GAIN; ++i)
    {
        auto yc = inputRowCenter_MM - 70 - (1 - kr) * 16;
        auto xc = columnCenters_MM[kc];

        auto pos = rack::mm2px(rack::Vec(xc, yc));
        auto knob = rack::createParamCentered<widgets::Knob9>(pos, module, i);
        addChild(knob);

        auto boxx0 = xc - columnWidth_MM * 0.5;
        auto boxy0 = yc + 4.073;

        auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
        auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));
        auto lab =
            widgets::Label::createWithBaselineBox(p0, s0, std::string("P") + std::to_string(i));
        addChild(lab);

        for (int m = 0; m < M::n_mod_inputs; ++m)
        {
            auto radius = rack::mm2px(knob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
            int id = M::modulatorIndexFor(i, m);
            auto *k = widgets::ModRingKnob::createCentered(pos, radius, module, id);
            overlays[i][m] = k;
            k->setVisible(false);
            k->underlyerParamWidget = knob;
            knob->modRings.insert(k);
            addChild(k);
        }

        kc++;
        if (kc == 4)
        {
            kc = 0;
            kr++;
        }
    }

    auto solcd = widgets::LCDBackground::posy_MM;
    auto eolcd = inputRowCenter_MM - 70 - 16 - 7;
    auto padlcd = 1;
    auto lcd = widgets::LCDBackground::createWithHeight(eolcd);
    if (!module)
        lcd->noModuleText = "Mixer";
    addChild(lcd);
    for (int i = 0; i < 6; ++i)
    {
        std::string lab = "O" + std::to_string(i + 1);
        if (i == 3)
            lab = "N";
        if (i == 4)
            lab = "1X2";
        if (i == 5)
            lab = "2x3";

        auto pm = M::OSC1_MUTE + i;
        auto ps = M::OSC1_SOLO + i;
        auto sxm = widgets::LCDBackground::posx_MM + padlcd;
        auto wm = rack::app::RACK_GRID_WIDTH * 12 * 25.4 / 75 - 2 * sxm - 2 * padlcd;

        auto dx = wm / 6.0;

        auto hm = eolcd - solcd - 2 * padlcd;
        auto dy = hm / 3.0;
        auto y0 = solcd + padlcd;

        auto sz = rack::mm2px(rack::Vec(dx, dy + 2));
        auto pmute = rack::mm2px(rack::Vec(sxm + i * dx, y0 + dy - 1));
        addParam(widgets::PlotAreaSwitch::create(pmute, sz, "M", module, pm));

        auto psolo = rack::mm2px(rack::Vec(sxm + i * dx, y0 + 2 * dy - 1));
        addParam(widgets::PlotAreaSwitch::create(psolo, sz, "S", module, ps));
    }

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto ydiff = modulationRowCenters_MM[1] - inputRowCenter_MM + 40;
        auto uxp = columnCenters_MM[i];
        auto uyp = modulationRowCenters_MM[0] - ydiff;

        auto *k =
            rack::createWidgetCentered<widgets::ModToggleButton>(rack::mm2px(rack::Vec(uxp, uyp)));
        toggles[i] = k;
        k->onToggle = [this, toggleIdx = i](bool isOn) {
            for (const auto &t : toggles)
                if (t)
                    t->setState(false);
            for (const auto &ob : overlays)
                for (const auto &o : ob)
                    if (o)
                        o->setVisible(false);
            if (isOn)
            {
                toggles[toggleIdx]->setState(true);
                for (const auto &ob : overlays)
                    if (ob[toggleIdx])
                    {
                        ob[toggleIdx]->setVisible(true);
                        ob[toggleIdx]->bdw->dirty = true;
                    }
                for (const auto &uk : underKnobs)
                    if (uk)
                        uk->setIsModEditing(true);
            }
            else
            {
                for (const auto &uk : underKnobs)
                    if (uk)
                        uk->setIsModEditing(false);
            }
        };

        addChild(k);
        uyp = modulationRowCenters_MM[1] - ydiff;
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module,
                                                          M::MIXER_MOD_INPUT + i));

        auto label = "MOD " + std::to_string(i + 1);
        auto bl = modulationLabelBaseline_MM - ydiff;

        addChild(makeLabelAt(bl, i, label));
    }

    int col = 0;
    float gls = 2.5;
    for (auto p : {M::INPUT_OSC1_L, M::INPUT_OSC1_R, M::INPUT_OSC2_L, M::INPUT_OSC2_R})
    {
        auto yp = inputRowCenter_MM - 20;
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));

        addChild(makeLabelAt(yp - 5.3, col, col % 2 ? "RIGHT" : "LEFT"));

        if (col % 2 == 0)
        {
            auto gl = widgets::GroupLabel::createAboveCenterWithColSpan(col == 0 ? "OSC1" : "OSC2",
                                                                        rack::Vec(xp, yp - gls), 2);
            addChild(gl);
        }
        col++;
    }

    col = 0;
    for (auto p : {M::INPUT_OSC3_L, M::INPUT_OSC3_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        if (col == 0)
        {
            auto gl = widgets::GroupLabel::createAboveCenterWithColSpan("OSC3",
                                                                        rack::Vec(xp, yp - gls), 2);
            addChild(gl);
        }
        col++;
    }

    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        if (col == 2)
        {
            auto gl = widgets::GroupLabel::createAboveCenterWithColSpan("OUTPUT",
                                                                        rack::Vec(xp, yp - gls), 2);
            addChild(gl);
        }
        col++;
    }

    col = 0;
    for (const std::string &s : {"LEFT", "RIGHT", "LEFT", "RIGHT"})
    {
        addChild(makeIORowLabel(col, s, col < 2));
        col++;
    }

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::mixer::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeMixer =
    rack::createModel<sst::surgext_rack::mixer::ui::MixerWidget::M,
                      sst::surgext_rack::mixer::ui::MixerWidget>("SurgeXTMixer");
