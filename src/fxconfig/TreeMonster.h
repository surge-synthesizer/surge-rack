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

#include "dsp/effects/TreemonsterEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_TREEMONSTER_H
#define SURGE_XT_RACK_SRC_FXCONFIG_TREEMONSTER_H

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_treemonster>::extraOutputs() { return 2; }
template <> constexpr int FXConfig<fxt_treemonster>::numParams() { return 8; }
template <> constexpr int FXConfig<fxt_treemonster>::specificParamCount() { return 2; }
template <> FXConfig<fxt_treemonster>::layout_t FXConfig<fxt_treemonster>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::knobGap16_MM;

    const auto col15 = (col[0] + col[1]) * 0.5f;
    const auto col25 = (col[2] + col[3]) * 0.5f;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_treemonster> fx_t;
    typedef TreemonsterEffect sx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB14, "PITCH", sx_t::tm_pitch, col15, row1},
        {LayoutItem::KNOB14, "RINGMOD", sx_t::tm_ring_mix, col25, row1},

        {LayoutItem::KNOB9, "THRESH", sx_t::tm_threshold, col[0], row2},
        {LayoutItem::KNOB9, "SPEED", sx_t::tm_speed, col[1], row2},
        {LayoutItem::KNOB9, "", sx_t::tm_hp, col[2], row2},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[2], row2, 1},
        {LayoutItem::KNOB9, "", sx_t::tm_lp, col[3], row2},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[3], row2, 1},
        LayoutItem::createKnobSpanLabel("LO - CUT - HI", col[2], row2, 2),
        LayoutItem::createGrouplabel("PITCH DETECTION", col[0], row2, 4).withExtra("SHORTRIGHT", 1),

        {LayoutItem::OUT_PORT, "V/OCT", fx_t::EXTRA_OUTPUT_0, col[0], row3},
        {LayoutItem::OUT_PORT, "ENV", fx_t::EXTRA_OUTPUT_0+1, col[1], row3},
        LayoutItem::createGrouplabel("DETECTION", col[0], row3, 2),

        {LayoutItem::KNOB9, "WIDTH", sx_t::tm_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", sx_t::tm_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createPresetLCDArea(),

    };
    // clang-format on
}

template <> void FXConfig<fxt_treemonster>::configSpecificParams(FX<fxt_treemonster> *m)
{
    typedef FX<fxt_treemonster> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 1, "Enable Low Cut");
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 1, 1, "Enable High Cut");

    // Default TM to polyphonic
    m->polyphonicMode = true;
}

template <> void FXConfig<fxt_treemonster>::processSpecificParams(FX<fxt_treemonster> *m)
{
    typedef FX<fxt_treemonster> fx_t;
    {
        auto drOff = m->fxstorage->p[TreemonsterEffect::tm_hp].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[TreemonsterEffect::tm_hp].deactivated = !parVa;
        }
    }
    {
        auto drOff = m->fxstorage->p[TreemonsterEffect::tm_lp].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[TreemonsterEffect::tm_lp].deactivated = !parVa;
        }
    }
}

template <>
void FXConfig<fxt_treemonster>::loadPresetOntoSpecificParams(
    FX<fxt_treemonster> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_treemonster> fx_t;
    typedef TreemonsterEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::tm_hp] ? 0 : 1);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.da[sx_t::tm_lp] ? 0 : 1);
}

template <>
bool FXConfig<fxt_treemonster>::isDirtyPresetVsSpecificParams(
    FX<fxt_treemonster> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_treemonster> fx_t;
    typedef TreemonsterEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    auto p1 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
    return !((p0 == !ps.da[sx_t::tm_hp]) && (p1 == !ps.da[sx_t::tm_lp]));
}

template <> void FXConfig<fxt_treemonster>::configExtraOutputs(FX<fxt_treemonster> *M)
{
    M->configOutput(FX<fxt_treemonster>::EXTRA_OUTPUT_0, "V/Oct Pitch Detection");
    M->configOutput(FX<fxt_treemonster>::EXTRA_OUTPUT_0 + 1, "Envelope Follower");
}

template <>
void FXConfig<fxt_treemonster>::populateExtraOutputs(FX<fxt_treemonster> *M, int c, Effect *fx)
{
    auto tm = static_cast<TreemonsterEffect *>(fx);
    for (int i = 0; i < BLOCK_SIZE; ++i)
    {
        M->extraOutputs[0][c][i] = (tm->smoothedPitch[0][i] + tm->smoothedPitch[1][i]) * 0.5 - 5.0;
        M->extraOutputs[1][c][i] = (tm->envelopeOut[0][i] + tm->envelopeOut[1][i]) * 0.5 * 10;
    }
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
