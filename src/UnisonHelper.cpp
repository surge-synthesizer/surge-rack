/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "UnisonHelper.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "SurgeXT.h"

namespace sst::surgext_rack::unisonhelper::ui
{
struct UnisonHelperWidget : public widgets::XTModuleWidget
{
    typedef sst::surgext_rack::unisonhelper::UnisonHelper M;
    UnisonHelperWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, n_fx_params> overlays;
    std::array<widgets::ModulatableKnob *, n_fx_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    widgets::PlotAreaLabel *infoLabel{nullptr};

    void step() override
    {
        if (module)
        {
            auto mod = static_cast<M *>(module);
            if (infoLabel->label != mod->infoDisplay)
            {
                infoLabel->label = mod->infoDisplay;
                infoLabel->bdw->dirty = true;
            }
        }

        widgets::XTModuleWidget::step();
    }

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
    }
};

UnisonHelperWidget::UnisonHelperWidget(
    sst::surgext_rack::unisonhelper::ui::UnisonHelperWidget::M *module)
{
    setModule(module);
    typedef layout::LayoutEngine<UnisonHelperWidget, M::DETUNE> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "UNISON", "fx", "BlankNoDisplay");
    addChild(bg);
    bg->addAlpha();

    const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1];
    const auto row2 = layout::LayoutConstants::vcoRowCenters_MM[0]; //
    const auto row3 = row2 - (row1 - row2);
    const auto row4 = row3 - (row1 - row2) - rack::mm2px(0.25);

    float cols[4];
    for (int i = 0; i < 4; ++i)
    {
        cols[i] = layout::LayoutConstants::firstColumnCenter_MM +
                  i * layout::LayoutConstants::columnWidth_MM;
    }

    typedef layout::LayoutItem li_t;
    std::vector<li_t> layout;
    for (int i = 0; i < 4; ++i)
    {
        layout.push_back(
            {li_t::PORT, "RTN " + std::to_string(i + 1), M::INPUT_SUB1 + i, cols[i], row2});
    }
    for (int i = 0; i < 4; ++i)
    {
        layout.push_back({li_t::OUT_PORT, "V/O " + std::to_string(i + 1), M::OUTPUT_VOCT_SUB1 + i,
                          cols[i], row1});
    }

    layout.push_back({li_t::KNOB9, "DETUNE", M::DETUNE, cols[2], row4});
    layout.push_back({li_t::EXTEND_LIGHT, "", M::DETUNE_EXTEND, cols[2], row4, +1});

    layout.push_back(
        {li_t::KNOB16, "VOICES", M::VOICE_COUNT, (cols[0] + cols[1]) * 0.5f, (row3 + row4) * 0.5f});
    layout.back().skipModulation = true;
    layout.push_back({li_t::KNOB9, "DRIFT", M::DRIFT, cols[3], row4});

    layout.push_back({li_t::KNOB9, "", M::LOCUT, cols[2], row3});
    layout.push_back({li_t::POWER_LIGHT, "", M::LOCUT_ENABLED, cols[2], row3, +1});
    layout.push_back({li_t::KNOB9, "", M::HICUT, cols[3], row3});
    layout.push_back({li_t::POWER_LIGHT, "", M::HICUT_ENABLED, cols[3], row3, +1});
    layout.push_back(li_t::createKnobSpanLabel("LO - CUT - HI", cols[2], row3, 2));
    layout.push_back(li_t::createLCDArea(row3 - rack::mm2px(8.50)));
    for (const auto &lay : layout)
    {
        engine_t::layoutItem(this, lay, "UNISON");
    }
    auto lcdb = box;
    {
        auto lc = getFirstDescendantOfType<widgets::LCDBackground>();
        if (lc)
        {
            lcdb = lc->box;
        }
    }

    auto oct = widgets::PlotAreaMenuItem::create(
        rack::Vec(lcdb.pos.x, lcdb.pos.y + lcdb.size.y - rack::mm2px(6)),
        rack::Vec(lcdb.size.x - rack::mm2px(1), rack::mm2px(5)), module, M::CHARACTER);
    addChild(oct);

    infoLabel = widgets::PlotAreaLabel::create(
        rack::Vec(lcdb.pos.x + rack::mm2px(1), lcdb.pos.y + lcdb.size.y - rack::mm2px(6)),
        rack::Vec(lcdb.size.x - rack::mm2px(1), rack::mm2px(5)));
    infoLabel->label = "";
    addChild(infoLabel);

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0);
    engine_t::createLeftRightInputLabels(this, "V/OCT", "");
    engine_t::createInputOutputPorts(this, M::INPUT_VOCT, -1, M::OUTPUT_L, M::OUTPUT_R);
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::unisonhelper::ui

rack::Model *modelUnisonHelper =
    rack::createModel<sst::surgext_rack::unisonhelper::ui::UnisonHelperWidget::M,
                      sst::surgext_rack::unisonhelper::ui::UnisonHelperWidget>(
        "SurgeXTUnisonHelper");
