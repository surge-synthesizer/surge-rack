//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/RotarySpeakerEffect.h"

#ifndef RACK_HACK_ROTARYSPEAKER_H
#define RACK_HACK_ROTARYSPEAKER_H

namespace sst::surgext_rack::fx
{

/*
 * ROTARY SPEAKER
 * - uses clock
 * - one specific param which is 'enables drive'
 */
template <> constexpr bool FXConfig<fxt_rotaryspeaker>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_rotaryspeaker>::specificParamCount() { return 1; }
template <> FXConfig<fxt_rotaryspeaker>::layout_t FXConfig<fxt_rotaryspeaker>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::unlabeledGap_MM;

    const auto bigRow = (row2 + row1) * 0.5f;

    const auto endOfPanel = row1 - 9;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_rotaryspeaker> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB16, "HORN", RotarySpeakerEffect::rot_horn_rate, col[1] - 10, bigRow},
        {LayoutItem::KNOB16, "ROTOR", RotarySpeakerEffect::rot_rotor_rate, col[1] + 10, bigRow},
        {LayoutItem::KNOB9, "DRIVE", RotarySpeakerEffect::rot_drive, col[3], row1},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[3], row1, 1},
        {LayoutItem::PORT, "CLOCK", fx_t ::INPUT_CLOCK, col[3], row2},

        {LayoutItem::KNOB9, "DOPPLER", RotarySpeakerEffect::rot_doppler, col[0], row3},
        {LayoutItem::KNOB9, "TREMOLO", RotarySpeakerEffect::rot_tremolo, col[1], row3},
        LayoutItem::createGrouplabel("HORN", col[0], row3, 2),
        {LayoutItem::KNOB9, "WIDTH", RotarySpeakerEffect::rot_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", RotarySpeakerEffect::rot_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createLCDArea(endOfPanel),
        {LayoutItem::LCD_MENU_ITEM, "MODEL", RotarySpeakerEffect::rot_waveshape, 0, endOfPanel}
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
#endif // RACK_HACK_ROTARYSPEAKER_H
