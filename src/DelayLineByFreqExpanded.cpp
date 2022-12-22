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

#include "DelayLineByFreqExpanded.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::delay::ui
{
struct DelayLineByFreqExpandedWidget : widgets::XTModuleWidget
{
    typedef delay::DelayLineByFreqExpanded M;
    DelayLineByFreqExpandedWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, n_fx_params> overlays;
    std::array<widgets::ModulatableKnob *, n_fx_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

DelayLineByFreqExpandedWidget::DelayLineByFreqExpandedWidget(
    DelayLineByFreqExpandedWidget::M *module)
    : XTModuleWidget()
{
    setModule(module);
    typedef layout::LayoutEngine<DelayLineByFreqExpandedWidget, M::VOCT> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "TUNED DELAY +", "fx", "BlankNoDisplay");
    bg->addBeta();
    addChild(bg);

    const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1];
    const auto row2 = layout::LayoutConstants::vcoRowCenters_MM[0] - rack::mm2px(1.75);
    const auto row3 = row2 - (row1 - row2);

    float cols[4];
    for (int i = 0; i < 4; ++i)
    {
        cols[i] = layout::LayoutConstants::firstColumnCenter_MM +
                  i * layout::LayoutConstants::columnWidth_MM;
    }

    typedef layout::LayoutItem li_t;
    std::vector<li_t> layout = {
        // clang-format off
        {li_t::PORT, "V/OCT", M::INPUT_VOCT, cols[0], row1 },
        {li_t::PORT, "LEFT", M::INPUT_FBL, cols[1], row1 },
        {li_t::PORT, "RIGHT", M::INPUT_FBR, cols[2], row1 },
        {li_t::KNOB9, "ATTEN", M::FB_ATTENUATION, cols[3], row1 },
        {li_t::EXTEND_LIGHT, "", M::FB_EXTEND, cols[3], row1, +1 },
        li_t::createGrouplabel("FEEDBACK INPUT", cols[1], row1, 3).withExtra("SHORTRIGHT", 1),

        {li_t::KNOB9, "FINE L", M::VOCT_FINE_LEFT, cols[1], row3},
        {li_t::KNOB9, "FINE R", M::VOCT_FINE_RIGHT, cols[3], row3},
        {li_t::KNOB9, "CENTER", M::VOCT, cols[2], row3},
        li_t::createGrouplabel("V/OCT ADJUST", cols[1], row3, 3),


        {li_t::KNOB9, "LP", M::FILTER_LP_CUTOFF_DIFF, cols[1], row2},
        {li_t::KNOB9, "HP", M::FILTER_HP_CUTOFF_DIFF, cols[2], row2},
        {li_t::KNOB9, "WET/DRY", M::FILTER_MIX, cols[3], row2},
        {li_t::POWER_LIGHT, "", M::FILTER_LP_ON, cols[1], row2, -1 },
        {li_t::POWER_LIGHT, "", M::FILTER_HP_ON, cols[2], row2, +1 },
        li_t::createGrouplabel("FEEDBACK FILTER", cols[1], row2, 3).withExtra("SHORTLEFT", 1),

        {li_t::PORT, "EXCITE", M::INPUT_EXCITER_AMP, cols[0], row2 },
        li_t::createLCDArea(row3 - rack::mm2px(3.75))
        // clang-format on
    };
    auto corr = li_t{li_t::KNOB9, "CORRECT", M::CORRECTION, cols[0], row3};
    corr.skipModulation = true;
    layout.push_back(corr);

    for (const auto &lay : layout)
    {
        engine_t::layoutItem(this, lay, "DealByFreqExpanded");
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);

    engine_t::createLeftRightInputLabels(this);
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::delay::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDelayLineByFreqExpanded =
    rack::createModel<sst::surgext_rack::delay::ui::DelayLineByFreqExpandedWidget::M,
                      sst::surgext_rack::delay::ui::DelayLineByFreqExpandedWidget>(
        "SurgeXTDelayLineByFreqExpanded");
