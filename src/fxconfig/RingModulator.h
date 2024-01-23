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

#include "dsp/effects/RingModulatorEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_RINGMODULATOR_H
#define SURGE_XT_RACK_SRC_FXCONFIG_RINGMODULATOR_H

namespace sst::surgext_rack::fx
{

/*
 *  RINGMOD
 * - two specific params for pre and post
 */

template <> constexpr bool FXConfig<fxt_ringmod>::usesSideband() { return true; }
template <> constexpr bool FXConfig<fxt_ringmod>::usesSidebandOversampled() { return true; }
template <> constexpr int FXConfig<fxt_ringmod>::numParams() { return 9; }
template <> constexpr int FXConfig<fxt_ringmod>::specificParamCount() { return 2; }
template <> FXConfig<fxt_ringmod>::layout_t FXConfig<fxt_ringmod>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    const auto colC = (col[2] + col[1]) * 0.5f;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_ringmod> fx_t;
    typedef RingModulatorEffect rm_t;

    // clang-format off
    FXConfig<fxt_ringmod>::layout_t res = {
        {LayoutItem::KNOB12, "FREQUENCY", rm_t::rm_carrier_freq, colC, row1},

        {LayoutItem::KNOB9, "BIAS", rm_t::rm_diode_fwdbias, col[0], row2},
        {LayoutItem::KNOB9, "LINEAR", rm_t::rm_diode_linregion, col[1], row2},
        LayoutItem::createGrouplabel("DIODE", col[0], row2, 2),

        {LayoutItem::PORT, "L/MON", fx_t::SIDEBAND_L, col[2], row2},
        {LayoutItem::PORT, "RIGHT", fx_t::SIDEBAND_R, col[3], row2},
        LayoutItem::createGrouplabel("EXT", col[2], row2, 2),


        {LayoutItem::KNOB9, "", rm_t::rm_lowcut, col[0], row3},
        {LayoutItem::KNOB9, "", rm_t::rm_highcut, col[1], row3},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], row3, -1},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[1], row3, 1},
        LayoutItem::createKnobSpanLabel("LO - CUT - HI", col[0], row3, 2),
        LayoutItem::createGrouplabel("EQ", col[0], row3, 2).withExtra("SHORTRIGHT", 1).withExtra("SHORTLEFT", 1),

        {LayoutItem::KNOB9, "DETUNE", rm_t::rm_unison_detune, col[2], row3},
        {LayoutItem::KNOB9, "MIX", rm_t::rm_mix, col[3], row3},

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createLeftMenuItem("SHAPE", rm_t::rm_carrier_shape),
        LayoutItem::createRightMenuItem("Voices", rm_t::rm_unison_voices)
    };
    // clang-format on

    // this indexing is a bit of a hack
    res[0].dynamicDeactivateFn = [](auto m) {
        auto fxm = static_cast<FX<fxt_ringmod> *>(m);
        auto &p = fxm->fxstorage->p[RingModulatorEffect::rm_carrier_shape];
        return p.val.i == p.val_max.i;
    };
    res[13].dynamicDeactivateFn = res[0].dynamicDeactivateFn;
    res[16].dynamicDeactivateFn = res[0].dynamicDeactivateFn;
    res[17].dynamicDeactivateFn = res[0].dynamicDeactivateFn;

    return res;
}

template <> void FXConfig<fxt_ringmod>::configSpecificParams(FX<fxt_ringmod> *m)
{
    typedef FX<fxt_ringmod> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 1, "Enable Low Cut");
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 1, 1, "Enable High Cut");
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

template <> void FXConfig<fxt_ringmod>::adjustParamsBasedOnState(FX<fxt_ringmod> *M)
{
    typedef FX<fxt_ringmod> fx_t;
    auto &p = M->fxstorage->p[RingModulatorEffect::rm_carrier_shape];

    if (M->inputs[fx_t::SIDEBAND_L].isConnected() || M->inputs[fx_t::SIDEBAND_R].isConnected())
    {
        p.val.i = p.val_max.i;
        auto pq = M->getParamQuantity(FX<fxt_ringmod>::FX_PARAM_0 +
                                      RingModulatorEffect::rm_carrier_shape);
        pq->setValue(1.0);
    }
    else
    {
        if (p.val.i == p.val_max.i)
        {
            p.val.i = 0;
            auto pq = M->getParamQuantity(FX<fxt_ringmod>::FX_PARAM_0 +
                                          RingModulatorEffect::rm_carrier_shape);
            pq->setValue(0.0);
        }
    }
}

} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
