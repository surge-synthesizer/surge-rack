//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/RingModulatorEffect.h"

#ifndef RACK_HACK_RINGMOD_H
#define RACK_HACK_RINGMOD_H

namespace sst::surgext_rack::fx
{

/*
 *  RINGMOD
 * - two specific params for pre and post
 */
template <> constexpr int FXConfig<fxt_ringmod>::specificParamCount() { return 2; }
template <> FXConfig<fxt_ringmod>::layout_t FXConfig<fxt_ringmod>::getLayout()
{
    const auto &col = layout::LayoutConstants::columnCenters_MM;

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    const auto colC = (col[2] + col[1]) * 0.5f;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_ringmod> fx_t;
    typedef RingModulatorEffect rm_t;

    // clang-format off
    return {
        {LayoutItem::KNOB12, "FREQUENCY", rm_t::rm_carrier_freq, colC, row1},

        {LayoutItem::KNOB9, "DETUNE", rm_t::rm_unison_detune, col[0], row2},
        {LayoutItem::KNOB9, "VOICES", rm_t::rm_unison_voices, col[1], row2},
        LayoutItem::createGrouplabel("CARRIER", col[0], row2, 2),

        {LayoutItem::KNOB9, "BIAS", rm_t::rm_diode_fwdbias, col[2], row2},
        {LayoutItem::KNOB9, "LINEAR", rm_t::rm_diode_linregion, col[3], row2},
        LayoutItem::createGrouplabel("DIODE", col[2], row2, 2),

        {LayoutItem::KNOB9, "", rm_t::rm_lowcut, col[0], row3},
        {LayoutItem::KNOB9, "", rm_t::rm_highcut, col[1], row3},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], row3, -1},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[1], row3, 1},
        LayoutItem::createKnobSpanLabel("LO - CUT - HI", col[0], row3, 2),
        LayoutItem::createGrouplabel("EQ", col[0], row3, 2).withExtra("SHORTRIGHT", 1).withExtra("SHORTLEFT", 1),

        {LayoutItem::KNOB9, "MIX", rm_t::rm_mix, col[3], row3},

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createSingleMenuItem("SHAPE", rm_t::rm_carrier_shape)
    };
    // clang-format on
}

template <> void FXConfig<fxt_ringmod>::configSpecificParams(FX<fxt_ringmod> *m)
{
    typedef FX<fxt_ringmod> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 1, "Enable Lo Cut");
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0 + 1, 0, 1, 1, "Enable Hi Cut");
}

template <> void FXConfig<fxt_ringmod>::processSpecificParams(FX<fxt_ringmod> *m)
{
    typedef FX<fxt_ringmod> fx_t;
    {
        auto drOff = m->fxstorage->p[RingModulatorEffect::rm_lowcut].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[RingModulatorEffect::rm_lowcut].deactivated = !parVa;
        }
    }
    {
        auto drOff = m->fxstorage->p[RingModulatorEffect::rm_highcut].deactivated;
        auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
        if (parVa != (!drOff))
        {
            m->fxstorage->p[RingModulatorEffect::rm_highcut].deactivated = !parVa;
        }
    }
}

template <>
void FXConfig<fxt_ringmod>::loadPresetOntoSpecificParams(
    FX<fxt_ringmod> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_ringmod> fx_t;
    typedef RingModulatorEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::rm_lowcut] ? 0 : 1);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.da[sx_t::rm_highcut] ? 0 : 1);
}

template <>
bool FXConfig<fxt_ringmod>::isDirtyPresetVsSpecificParams(
    FX<fxt_ringmod> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_ringmod> fx_t;
    typedef RingModulatorEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    auto p1 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
    return !((p0 == !ps.da[sx_t::rm_lowcut]) && (p1 == !ps.da[sx_t::rm_highcut]));
}
} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
