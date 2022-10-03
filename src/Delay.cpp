//
// Created by Paul Walker on 9/20/22.
//

#include "Delay.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::delay::ui
{
struct DelayWidget : widgets::XTModuleWidget, widgets::StandardWidthWithModulationConstants
{
    typedef delay::Delay M;
    DelayWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, M::n_delay_params> overlays;
    std::array<widgets::KnobN *, M::n_delay_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
        auto xtm = static_cast<Delay *>(module);
        typedef modules::ClockProcessor<Delay> cp_t;
        menu->addChild(new rack::ui::MenuSeparator);
        auto t = xtm->clockProc.clockStyle;
        menu->addChild(
            rack::createMenuItem("Clock in QuarterNotes", CHECKMARK(t == cp_t::QUARTER_NOTE),
                                 [xtm]() { xtm->clockProc.clockStyle = cp_t::QUARTER_NOTE; }));

        menu->addChild(
            rack::createMenuItem("Clock in BPM CV", CHECKMARK(t == cp_t::BPM_VOCT),
                                 [xtm]() { xtm->clockProc.clockStyle = cp_t::BPM_VOCT; }));
    }
};

DelayWidget::DelayWidget(DelayWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    for (auto &uk : underKnobs)
        uk = nullptr;

    for (auto &ob : overlays)
        for (auto &o : ob)
            o = nullptr;

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "Delay", "fx", "BlankNoDisplay");
    addChild(bg);

    int row{0}, col{0};
    std::map<int, std::string> nm;
    nm[M::TIME_L] = "LEFT";
    nm[M::TIME_R] = "RIGHT";
    nm[M::TIME_S] = "SENS";
    nm[M::FEEDBACK] = "FBACK";
    nm[M::CROSSFEED] = "CROSSFD";
    nm[M::LOCUT] = "LOCUT";
    nm[M::HICUT] = "HICUT";
    nm[M::MODRATE] = "MRATE";
    nm[M::MODDEPTH] = "MDEPT";
    nm[M::MIX] = "MIX";

    for (int p = M::TIME_L; p <= M::MIX; ++p)
    {
        if (p == M::FEEDBACK)
        {
            row = 1;
            col = 0;
        }
        if (p == M::MODRATE)
        {
            row = 2;
            col = 1;
        }
        auto xcmm = columnCenters_MM[col];
        auto ycmm = 71 - (2 - row) * 16;
        auto pos = rack::mm2px(rack::Vec(xcmm, ycmm));
        auto knob = rack::createParamCentered<widgets::Knob9>(pos, module, p);
        if (knob)
        {
            addChild(knob);
            auto boxx0 = xcmm - columnWidth_MM * 0.5;
            auto boxy0 = ycmm + 4.073;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));
            auto lab = widgets::Label::createWithBaselineBox(p0, s0, nm[p]);
            addChild(lab);

            underKnobs[p] = knob;

            for (int m = 0; m < M::n_mod_inputs; ++m)
            {
                auto radius = rack::mm2px(knob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
                int id = M::modulatorIndexFor(p, m);
                auto *k = widgets::ModRingKnob::createCentered(pos, radius, module, id);
                overlays[p][m] = k;
                k->setVisible(false);
                k->underlyerParamWidget = knob;
                knob->modRings.insert(k);
                addChild(k);
            }
        }
        col++;
        if (col == 4)
        {
            col = 0;
            row++;
        }
    }

    {
        auto xcmm = columnCenters_MM[0];
        auto ycmm = 71 - (2 - 2) * 16;
        auto pos = rack::mm2px(rack::Vec(xcmm, ycmm));

        auto port = rack::createInputCentered<widgets::Port>(
            rack::mm2px(rack::Vec(xcmm, ycmm + verticalPortOffset_MM)), module, M::INPUT_CLOCK);
        addChild(port);

        auto boxx0 = xcmm - columnWidth_MM * 0.5;
        auto boxy0 = ycmm + 8.573 - 5;

        auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
        auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));
        auto lab = widgets::Label::createWithBaselineBox(p0, s0, "CLOCK");

        lab->hasDynamicLabel = true;
        lab->module = module;
        lab->dynamicLabel = [](modules::XTModule *m) -> std::string {
            if (!m)
                return "CLOCK";
            auto fxm = static_cast<Delay *>(m);
            typedef modules::ClockProcessor<Delay> cp_t;

            if (fxm->clockProc.clockStyle == cp_t::QUARTER_NOTE)
            {
                return "CLOCK";
            }
            else
            {
                return "BPM";
            }
        };

        addChild(lab);
    }

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto uxp = columnCenters_MM[i];
        auto uyp = modulationRowCenters_MM[0];

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
        uyp = modulationRowCenters_MM[1];
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module,
                                                          M::DELAY_MOD_INPUT + i));
    }

    col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
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
} // namespace sst::surgext_rack::delay::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDelay =
    rack::createModel<sst::surgext_rack::delay::ui::DelayWidget::M,
                      sst::surgext_rack::delay::ui::DelayWidget>("SurgeXTDelay");
