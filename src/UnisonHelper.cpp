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
    typedef layout::LayoutEngine<UnisonHelperWidget, M::VOICE_COUNT> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "UNISON", "fx", "BlankNoDisplay");
    addChild(bg);
    bg->addAlpha();

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
