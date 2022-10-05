//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/DistortionEffect.h"

#ifndef RACK_HACK_DISTORTION_H
#define RACK_HACK_DISTORTION_H

namespace sst::surgext_rack::fx
{

/*
 *  DISTORTION
 * - two specific params for pre and post
 */
template <> constexpr int FXConfig<fxt_distortion>::specificParamCount() { return 2; }
template <> FXConfig<fxt_distortion>::layout_t FXConfig<fxt_distortion>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    const auto col15 = (col[0] + col[1]) * 0.5f;
    const auto col25 = (col[2] + col[3]) * 0.5f;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_distortion> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB12, "DRIVE", DistortionEffect::dist_drive, col15, row1},
        {LayoutItem::KNOB12, "FEEDBACK", DistortionEffect::dist_feedback, col25, row1},

        {LayoutItem::KNOB9, "GAIN", DistortionEffect::dist_preeq_gain, col[0], row2},
        {LayoutItem::KNOB9, "FREQ", DistortionEffect::dist_preeq_freq, col[1], row2},
        {LayoutItem::KNOB9, "BW", DistortionEffect::dist_preeq_bw, col[2], row2},
        {LayoutItem::KNOB9, "HI CUT", DistortionEffect::dist_preeq_highcut, col[3], row2},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[3], row2, 1},
        LayoutItem::createGrouplabel("PRE-EQ", col[0], row2, 4).withExtra("SHORTRIGHT", 1),

        {LayoutItem::KNOB9, "GAIN", DistortionEffect::dist_posteq_gain, col[0], row3},
        {LayoutItem::KNOB9, "FREQ", DistortionEffect::dist_posteq_freq, col[1], row3},
        {LayoutItem::KNOB9, "BW", DistortionEffect::dist_posteq_bw, col[2], row3},
        {LayoutItem::KNOB9, "HI CUT", DistortionEffect::dist_posteq_highcut, col[3], row3},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[3], row3, 1},
        LayoutItem::createGrouplabel("POST-EQ", col[0], row3, 4).withExtra("SHORTRIGHT", 1),

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createSingleMenuItem("MODEL", DistortionEffect::dist_model)
    };
    // clang-format on
}

template <> void FXConfig<fxt_distortion>::configSpecificParams(FX<fxt_distortion> *m)
{
    typedef FX<fxt_distortion> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 1, "Enable Pre Hi Cut");
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0 + 1, 0, 1, 1, "Enable Post Hi Cut");
}

template <> void FXConfig<fxt_distortion>::processSpecificParams(FX<fxt_distortion> *m)
{
    typedef FX<fxt_distortion> fx_t;
    {
        auto drOff = m->fxstorage->p[DistortionEffect::dist_preeq_highcut].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[DistortionEffect::dist_preeq_highcut].deactivated = !parVa;
        }
    }
    {
        auto drOff = m->fxstorage->p[DistortionEffect::dist_posteq_highcut].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[DistortionEffect::dist_posteq_highcut].deactivated = !parVa;
        }
    }
}

template <>
void FXConfig<fxt_distortion>::loadPresetOntoSpecificParams(
    FX<fxt_distortion> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_distortion> fx_t;
    typedef RotarySpeakerEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::rot_drive] ? 0 : 1);
}

template <>
bool FXConfig<fxt_distortion>::isDirtyPresetVsSpecificParams(
    FX<fxt_distortion> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_distortion> fx_t;
    typedef RotarySpeakerEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::rot_drive]);
}
} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
