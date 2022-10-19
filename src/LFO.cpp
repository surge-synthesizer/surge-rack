/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "LFO.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::lfo::ui
{
struct LFOWidget : widgets::XTModuleWidget
{
    typedef lfo::LFO M;
    LFOWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_lfo_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_lfo_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

LFOWidget::LFOWidget(LFOWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    int screwWidth = 20;
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * screwWidth, rack::app::RACK_GRID_HEIGHT);

    typedef layout::LayoutEngine<LFOWidget, M::RATE> engine_t;
    engine_t::initializeModulationToBlank(this);

    auto bg = new widgets::Background(box.size, "LFO x EG", "other", "EGLFO");
    addChild(bg);

    static constexpr float LFOColumnWidth = 13.78f;

    typedef layout::LayoutItem li_t;

    {
        const auto col = layout::LayoutConstants::firstColumnCenter_MM;
        const auto cw = LFOColumnWidth; // LFOColumnWidth;
        const auto row1 = layout::LayoutConstants::rowStart_MM - 3;
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::KNOB9, "RATE", M::RATE, col, row1},
                {li_t::KNOB9, "PHASE", M::PHASE, col + cw, row1},
                {li_t::KNOB9, "DEFORM", M::DEFORM, col + 2 * cw, row1},
                {li_t::KNOB9, "AMP", M::AMPLITUDE, col + 3 * cw, row1},
                li_t::createGrouplabel("WAVEFORM", col, row1, 4, cw)
         };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }
    }

    {
        const auto row1 = layout::LayoutConstants::rowStart_MM + 3;
        float cw = LFOColumnWidth * 0.46;
        float c0 = layout::LayoutConstants::firstColumnCenter_MM + LFOColumnWidth * 3 + 1.75 * cw;
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::VSLIDER, "D", M::E_DELAY, c0, row1, 23},
                {li_t::VSLIDER, "A", M::E_ATTACK, c0 + cw, row1, 23},
                {li_t::VSLIDER, "H", M::E_HOLD, c0 + 2 * cw, row1, 23},
                {li_t::VSLIDER, "D", M::E_DECAY, c0 + 3 * cw, row1, 23},
                {li_t::VSLIDER, "S", M::E_SUSTAIN, c0 + 4 * cw, row1, 23},
                {li_t::VSLIDER, "R", M::E_RELEASE, c0 + 5 *cw, row1, 23},
                li_t::createGrouplabel("ENVELOPE", c0 + 0.25*cw, layout::LayoutConstants::rowStart_MM - 3, 3.1, LFOColumnWidth)
         };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIG", "CLK RATE", "CLK ENV", "PHASE"};
        for (auto p :
             {M::INPUT_TRIGGER, M::INPUT_CLOCK_RATE, M::INPUT_CLOCK_ENV, M::INPUT_PHASE_DIRECT})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::firstColumnCenter_MM + LFOColumnWidth * col;
            addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                              module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto lab = engine_t::makeLabelAt(bl, col, labv[col], style::XTStyle::TEXT_LABEL,
                                             LFOColumnWidth);
            addChild(lab);

            col++;
        }
    }

    auto ht = layout::LayoutConstants::rowStart_MM - 16;
    auto lcd = widgets::LCDBackground::createWithHeight(ht, screwWidth);
    addChild(lcd);

    li_t shape{li_t::LCD_MENU_ITEM_SURGE_PARAM, "SHAPE", M::SHAPE, 0, ht};
    engine_t::layoutItem(this, shape, "LFO");

    {
        int col = 0;
        std::vector<std::string> labv{"OUTLFO", "OUTENV", "OUTWAV"};
        for (auto p : {M::OUTPUT_MIX, M::OUTPUT_ENV, M::OUTPUT_WAVE})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp =
                layout::LayoutConstants::firstColumnCenter_MM + LFOColumnWidth * (3 + col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto cx = xp;

            auto boxx0 = cx - LFOColumnWidth * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(LFOColumnWidth, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, labv[col]);
            addChild(lab);

            col++;
        }
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIGP", "TRIGF", "TRIGA"};
        for (auto p : {M::OUTPUT_TRIGPHASE, M::OUTPUT_TRIGF, M::OUTPUT_TRIGA})
        {
            auto yp = layout::LayoutConstants::modulationRowCenters_MM[1];
            auto xp =
                layout::LayoutConstants::firstColumnCenter_MM + LFOColumnWidth * (3 + col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = yp - 5;
            auto cx = xp;

            auto boxx0 = cx - LFOColumnWidth * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(LFOColumnWidth, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, labv[col]);
            addChild(lab);

            col++;
        }
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::LFO_MOD_INPUT, 0, LFOColumnWidth);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::lfo::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeLFO = rack::createModel<sst::surgext_rack::lfo::ui::LFOWidget::M,
                                               sst::surgext_rack::lfo::ui::LFOWidget>("SurgeXTLFO");
