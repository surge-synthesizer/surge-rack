//
// Created by Paul Walker on 9/27/22.
//

// Because of JUCE we can't include this here currently
#include "dsp/effects/PhaserEffect.h"

#ifndef RACK_HACK_PHASER_H
#define RACK_HACK_PHASER_H

namespace sst::surgext_rack::fx
{

/*
 */
template <> constexpr bool FXConfig<fxt_phaser>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_phaser>::specificParamCount() { return 1; }

template <> FXConfig<fxt_phaser>::layout_t FXConfig<fxt_phaser>::getLayout()
{
    const auto &col = layout::LayoutConstants::columnCenters_MM;
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_phaser> fx_t;
    typedef PhaserEffect ph_t;

    // clang-format off
    return {
        {LayoutItem::KNOB9, "RATE", ph_t::ph_mod_rate, col[0], row1},
        {LayoutItem::KNOB9, "DEPTH", ph_t::ph_mod_depth, col[1], row1},
        {LayoutItem::KNOB9, "STEREO", ph_t::ph_stereo, col[2], row1},
        {LayoutItem::KNOB9, "TONE", ph_t::ph_tone, col[3], row1},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[3], row1, 1},


        {LayoutItem::KNOB9, "SPREAD", ph_t::ph_spread, col[0], row2},
        {LayoutItem::KNOB9, "CENTER", ph_t::ph_center, col[1], row2},
        {LayoutItem::KNOB9, "SHARP", ph_t::ph_sharpness, col[2], row2},
        {LayoutItem::KNOB9, "FEEDBACK", ph_t::ph_feedback, col[3], row2},
        LayoutItem::createGrouplabel("STAGES", col[0], row2, 4),


        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK, col[0], row3 },
        {LayoutItem::KNOB9, "WIDTH", ph_t::ph_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", ph_t::ph_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createLeftMenuItem("WAVE", ph_t::ph_mod_wave),
        LayoutItem::createRightMenuItem("STAGES", ph_t::ph_stages)
    };
    // clang-format on
}

template <> void FXConfig<fxt_phaser>::configSpecificParams(FX<fxt_phaser> *m)
{
    typedef FX<fxt_phaser> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 1, "Enable Tone");
}

template <> void FXConfig<fxt_phaser>::processSpecificParams(FX<fxt_phaser> *m)
{
    typedef FX<fxt_phaser> fx_t;
    FXLayoutHelper::processDeactivate(m, PhaserEffect::ph_tone, fx_t::FX_SPECIFIC_PARAM_0);
}

template <>
void FXConfig<fxt_phaser>::loadPresetOntoSpecificParams(
    FX<fxt_phaser> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_phaser> fx_t;
    typedef PhaserEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::ph_tone] ? 0 : 1);
}

template <>
bool FXConfig<fxt_phaser>::isDirtyPresetVsSpecificParams(
    FX<fxt_phaser> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_phaser> fx_t;
    typedef PhaserEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::ph_tone]);
}
} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
