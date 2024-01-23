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

#include "dsp/effects/RotarySpeakerEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_ROTARYSPEAKER_H
#define SURGE_XT_RACK_SRC_FXCONFIG_ROTARYSPEAKER_H

namespace sst::surgext_rack::fx
{

/*
 * ROTARY SPEAKER
 * - uses clock
 * - one specific param which is 'enables drive'
 */
template <> constexpr int FXConfig<fxt_rotaryspeaker>::numParams() { return 8; }
template <> constexpr bool FXConfig<fxt_rotaryspeaker>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_rotaryspeaker>::specificParamCount() { return 1; }
template <> FXConfig<fxt_rotaryspeaker>::layout_t FXConfig<fxt_rotaryspeaker>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    const auto col15 = (col[0] + col[1]) * 0.5f;
    const auto col25 = (col[2] + col[3]) * 0.5f;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_rotaryspeaker> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB12, "HORN", RotarySpeakerEffect::rot_horn_rate, col15, row1},
        {LayoutItem::KNOB12, "ROTOR", RotarySpeakerEffect::rot_rotor_rate, col25, row1},

        {LayoutItem::PORT, "CLOCK", fx_t ::INPUT_CLOCK, col15, row2},

        {LayoutItem::KNOB9, "DRIVE", RotarySpeakerEffect::rot_drive, col25, row2},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col25, row2, 1},

        {LayoutItem::KNOB9, "DOPPLER", RotarySpeakerEffect::rot_doppler, col[0], row3},
        {LayoutItem::KNOB9, "TREM", RotarySpeakerEffect::rot_tremolo, col[1], row3},
        LayoutItem::createGrouplabel("MOD", col[0], row3, 2),
        {LayoutItem::KNOB9, "WIDTH", RotarySpeakerEffect::rot_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", RotarySpeakerEffect::rot_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createSingleMenuItem("MODEL", RotarySpeakerEffect::rot_waveshape)
    };
    // clang-format on
}

template <> void FXConfig<fxt_rotaryspeaker>::configSpecificParams(FX<fxt_rotaryspeaker> *m)
{
    typedef FX<fxt_rotaryspeaker> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 1, "Enable Drive");
}

template <> void FXConfig<fxt_rotaryspeaker>::processSpecificParams(FX<fxt_rotaryspeaker> *m)
{
    typedef FX<fxt_rotaryspeaker> fx_t;
    auto drOff = m->fxstorage->p[RotarySpeakerEffect::rot_drive].deactivated;
    auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    if (parVa != (!drOff))
    {
        m->fxstorage->p[RotarySpeakerEffect::rot_drive].deactivated = !parVa;
    }
}

template <>
void FXConfig<fxt_rotaryspeaker>::loadPresetOntoSpecificParams(
    FX<fxt_rotaryspeaker> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_rotaryspeaker> fx_t;
    typedef RotarySpeakerEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::rot_drive] ? 0 : 1);
}

template <>
bool FXConfig<fxt_rotaryspeaker>::isDirtyPresetVsSpecificParams(
    FX<fxt_rotaryspeaker> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_rotaryspeaker> fx_t;
    typedef RotarySpeakerEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::rot_drive]);
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
