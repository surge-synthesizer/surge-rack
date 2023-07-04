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

#include "dsp/effects/Reverb1Effect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_REVERB1_H
#define SURGE_XT_RACK_SRC_FXCONFIG_REVERB1_H

namespace sst::surgext_rack::fx
{

/*
 * REVERB
 */
template <> constexpr int FXConfig<fxt_reverb>::numParams() { return 11; }
template <> constexpr int FXConfig<fxt_reverb>::specificParamCount() { return 2; }

template <> FXConfig<fxt_reverb>::layout_t FXConfig<fxt_reverb>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::knobGap16_MM;

    typedef FX<fxt_chorus4> fx_t;

    return {
        // clang-format off
        {LayoutItem::KNOB14, "SIZE", Reverb1Effect::rev1_roomsize, FXLayoutHelper::bigCol0, row1},
        {LayoutItem::KNOB14, "DECAY", Reverb1Effect::rev1_decaytime, FXLayoutHelper::bigCol1, row1},

        {LayoutItem::KNOB9, "LO CUT", Reverb1Effect::rev1_lowcut, col[0], row2},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], row2, -1},
        {LayoutItem::KNOB9, "", Reverb1Effect::rev1_freq1, col[1], row2},
        {LayoutItem::KNOB9, "", Reverb1Effect::rev1_gain1, col[2], row2},
        LayoutItem::createKnobSpanLabel("FREQ - PEAK - GAIN", col[1], row2, 2),

        {LayoutItem::KNOB9, "HI CUT", Reverb1Effect::rev1_highcut, col[3], row2},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0 + 1, col[3], row2, 1},
        LayoutItem::createGrouplabel("EQ", col[0], row2, 4).withExtra("SHORTLEFT",1).withExtra("SHORTRIGHT",1),

        {LayoutItem::KNOB9, "PRE DLY", Reverb1Effect::rev1_predelay, col[0], row3},
        {LayoutItem::KNOB9, "DAMPING", Reverb1Effect::rev1_damping, col[1], row3},
        {LayoutItem::KNOB9, "WIDTH", Reverb1Effect::rev1_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", Reverb1Effect::rev1_mix, col[3], row3},

        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createPresetLCDArea(),
        // clang-format on
    };
}

template <> void FXConfig<fxt_reverb>::configSpecificParams(FX<fxt_reverb> *m)
{
    typedef FX<fxt_reverb> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 1, "Enable Low Cut");
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 1, 1, "Enable High Cut");
}

template <> void FXConfig<fxt_reverb>::processSpecificParams(FX<fxt_reverb> *m)
{
    typedef FX<fxt_reverb> fx_t;
    FXLayoutHelper::processDeactivate(m, Reverb1Effect::rev1_lowcut, fx_t::FX_SPECIFIC_PARAM_0);
    FXLayoutHelper::processDeactivate(m, Reverb1Effect::rev1_highcut,
                                      fx_t::FX_SPECIFIC_PARAM_0 + 1);
}

template <>
void FXConfig<fxt_reverb>::loadPresetOntoSpecificParams(
    FX<fxt_reverb> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_reverb> fx_t;
    typedef Reverb1Effect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::rev1_lowcut] ? 0 : 1);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.da[sx_t::rev1_highcut] ? 0 : 1);
}

template <>
bool FXConfig<fxt_reverb>::isDirtyPresetVsSpecificParams(
    FX<fxt_reverb> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_reverb> fx_t;
    typedef Reverb1Effect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    auto p1 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::rev1_lowcut] && p1 == !ps.da[sx_t::rev1_highcut]);
}

/*
 * END OF REVERB2
 */
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_REVERB2_H
