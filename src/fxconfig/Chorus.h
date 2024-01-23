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

#include "dsp/effects/ChorusEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_CHORUS_H
#define SURGE_XT_RACK_SRC_FXCONFIG_CHORUS_H

namespace sst::surgext_rack::fx
{
/*
 * CHORUS:
 * One extra input for a clock
 * Two extra params for power on the lo and hi cut
 */

template <> constexpr int FXConfig<fxt_chorus4>::numParams() { return 8; }
template <> constexpr bool FXConfig<fxt_chorus4>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_chorus4>::specificParamCount() { return 2; }

template <> FXConfig<fxt_chorus4>::layout_t FXConfig<fxt_chorus4>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();

    const auto outputRow = FXLayoutHelper::rowStart_MM;
    const auto delayRow = outputRow - FXLayoutHelper::labeledGap_MM;
    const auto bigRow = delayRow - FXLayoutHelper::knobGap16_MM;

    const auto endOfPanel = bigRow - 8;

    typedef FX<fxt_chorus4> fx_t;
    typedef ChorusEffect<4> sfx_t;

    // fixme use the enums
    // clang-format off
    return {
        {LayoutItem::KNOB14, "RATE", sfx_t::ch_rate, FXLayoutHelper::bigCol0, bigRow},
        {LayoutItem::KNOB14, "DEPTH", sfx_t::ch_depth, FXLayoutHelper::bigCol1, bigRow},

        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
                 (col[0]+col[1]) * 0.5f, delayRow },
        {LayoutItem::KNOB9, "TIME", sfx_t::ch_time, col[2], delayRow},
        {LayoutItem::KNOB9, "FEEDBACK", sfx_t::ch_feedback, col[3], delayRow},
        LayoutItem::createGrouplabel("DELAY", col[2], delayRow, 2),

        {LayoutItem::KNOB9, "", sfx_t::ch_lowcut, col[0], outputRow},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], outputRow, -1},

        {LayoutItem::KNOB9, "", sfx_t::ch_highcut, col[1], outputRow},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[1], outputRow, +1},
        LayoutItem::createGrouplabel("EQ", col[0], outputRow, 2).withExtra("SHORTLEFT",1).withExtra("SHORTRIGHT",1),
        LayoutItem::createKnobSpanLabel("LO - CUT - HI", col[0], outputRow, 2),

        {LayoutItem::KNOB9, "WIDTH", sfx_t::ch_width, col[2], outputRow},
        {LayoutItem::KNOB9, "MIX", sfx_t::ch_mix, col[3], outputRow},
        LayoutItem::createGrouplabel("OUTPUT", col[2], outputRow, 2),

        LayoutItem::createPresetLCDArea(),
    };

    // clang-format on
}

template <> void FXConfig<fxt_chorus4>::configSpecificParams(FX<fxt_chorus4> *m)
{
    typedef FX<fxt_chorus4> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 1, "Enable Low Cut");
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 1, 1, "Enable High Cut");
}

template <> void FXConfig<fxt_chorus4>::processSpecificParams(FX<fxt_chorus4> *m)
{
    typedef FX<fxt_chorus4> fx_t;
    FXLayoutHelper::processDeactivate(m, ChorusEffect<4>::ch_lowcut, fx_t::FX_SPECIFIC_PARAM_0);
    FXLayoutHelper::processDeactivate(m, ChorusEffect<4>::ch_highcut,
                                      fx_t::FX_SPECIFIC_PARAM_0 + 1);
}

template <>
void FXConfig<fxt_chorus4>::loadPresetOntoSpecificParams(
    FX<fxt_chorus4> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_chorus4> fx_t;
    typedef ChorusEffect<4> sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::ch_lowcut] ? 0 : 1);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.da[sx_t::ch_highcut] ? 0 : 1);
}

template <>
bool FXConfig<fxt_chorus4>::isDirtyPresetVsSpecificParams(
    FX<fxt_chorus4> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_chorus4> fx_t;
    typedef ChorusEffect<4> sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    auto p1 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::ch_lowcut] && p1 == !ps.da[sx_t::ch_highcut]);
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_CHORUS_H
