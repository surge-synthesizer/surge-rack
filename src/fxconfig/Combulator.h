/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
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

#include "dsp/effects/CombulatorEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_COMBULATOR_H
#define SURGE_XT_RACK_SRC_FXCONFIG_COMBULATOR_H

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_combulator>::specificParamCount() { return 1; }
template <> FXConfig<fxt_combulator>::layout_t FXConfig<fxt_combulator>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    typedef FX<fxt_combulator> fx_t;
    typedef CombulatorEffect c_t;

    // clang-format off
    return {
        {LayoutItem::KNOB9, "NOISE IN", c_t::combulator_noise_mix, col[0], row1},
        {LayoutItem::KNOB9, "CENTER", c_t::combulator_freq1, col[1], row1},
        {LayoutItem::KNOB9, "", c_t::combulator_freq2, col[2], row1},
        {LayoutItem::KNOB9, "", c_t::combulator_freq3, col[3], row1},
        LayoutItem::createKnobSpanLabel("1 - OFFSET - 2", col[2], row1, 2),
        LayoutItem::createGrouplabel("FREQUENCY", col[1], row1, 3),

        {LayoutItem::KNOB9, "FEEDBACK", c_t::combulator_feedback, col[0], row2},
        {LayoutItem::KNOB9, "COMB 1", c_t::combulator_gain1, col[1], row2},
        {LayoutItem::KNOB9, "COMB 2", c_t::combulator_gain2, col[2], row2},
        {LayoutItem::KNOB9, "COMB 3", c_t::combulator_gain3, col[3], row2},
        LayoutItem::createGrouplabel("LEVEL", col[1], row2, 3),

        {LayoutItem::KNOB9, "TONE", c_t::combulator_tone, col[0], row3},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], row3, 1},


        {LayoutItem::KNOB9, "PAN 2", c_t::combulator_pan2, col[1], row3},
        {LayoutItem::KNOB9, "PAN 3", c_t::combulator_pan3, col[2], row3},
        {LayoutItem::KNOB9, "MIX", c_t::combulator_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[1], row3, 3),

        LayoutItem::createPresetLCDArea(),
    };
    // clang-format on
}

template <> void FXConfig<fxt_combulator>::configSpecificParams(FX<fxt_combulator> *m)
{
    typedef FX<fxt_combulator> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 1, "Enable Tone Filter");
}

template <> void FXConfig<fxt_combulator>::processSpecificParams(FX<fxt_combulator> *m)
{
    typedef FX<fxt_combulator> fx_t;
    {
        auto drOff = m->fxstorage->p[CombulatorEffect::combulator_tone].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[CombulatorEffect::combulator_tone].deactivated = !parVa;
        }
    }
}

template <>
void FXConfig<fxt_combulator>::loadPresetOntoSpecificParams(
    FX<fxt_combulator> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_combulator> fx_t;
    typedef CombulatorEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::combulator_tone] ? 0 : 1);
}

template <>
bool FXConfig<fxt_combulator>::isDirtyPresetVsSpecificParams(
    FX<fxt_combulator> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_combulator> fx_t;
    typedef CombulatorEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::combulator_tone]);
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
