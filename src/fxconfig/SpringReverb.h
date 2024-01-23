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

// We could include this to get the mappings but that pulls in juce at the
// wrong level so for SR and SR only don't use the enums
// #include "dsp/effects/chowdsp/SpringReverbEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_SPRINGREVERB_H
#define SURGE_XT_RACK_SRC_FXCONFIG_SPRINGREVERB_H

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_spring_reverb>::numParams() { return 8; }
template <> constexpr int FXConfig<fxt_spring_reverb>::extraInputs() { return 1; }
template <> constexpr int FXConfig<fxt_spring_reverb>::extraSchmidtTriggers()
{
    return 1 + MAX_POLY;
}
template <> constexpr int FXConfig<fxt_spring_reverb>::specificParamCount() { return 1; }
template <> constexpr bool FXConfig<fxt_spring_reverb>::nanCheckOutput() { return true; }
template <> FXConfig<fxt_spring_reverb>::layout_t FXConfig<fxt_spring_reverb>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::knobGap16_MM;

    // fixme use the enums
    // clang-format off
    return {
        {LayoutItem::KNOB14, "SIZE", 0, FXLayoutHelper::bigCol0, row1},
        {LayoutItem::KNOB14, "DECAY", 1, FXLayoutHelper::bigCol1, row1},

        {LayoutItem::PORT, "TRIG", FX<fxt_spring_reverb>::INPUT_SPECIFIC_0, col[0], row2},
        {LayoutItem::MOMENTARY_PARAM, "KNOCK", FX<fxt_spring_reverb>::FX_SPECIFIC_PARAM_0, col[1], row2},
        LayoutItem::createGrouplabel("KNOCK", col[0], row2, 2),

        {LayoutItem::KNOB12, "REFLECT", 2, FXLayoutHelper::bigCol1, row2 - 1.0f},

        {LayoutItem::KNOB9, "SPIN", 4, col[0], row3},
        {LayoutItem::KNOB9, "CHAOS", 5, col[1], row3},
        LayoutItem::createGrouplabel("MODULATION", col[0], row3, 2),
        {LayoutItem::KNOB9, "DAMPING", 3, col[2], row3},
        {LayoutItem::KNOB9, "MIX", 7, col[3], row3},
        LayoutItem::createPresetLCDArea()
    };

    // clang-format on
}

template <> void FXConfig<fxt_spring_reverb>::configSpecificParams(FX<fxt_spring_reverb> *m)
{
    typedef FX<fxt_spring_reverb> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 0, "Interrupting Cow");
}

template <> void FXConfig<fxt_spring_reverb>::configExtraInputs(FX<fxt_spring_reverb> *m)
{
    m->configInput(FX<fxt_spring_reverb>::INPUT_SPECIFIC_0, "Trigger to Knock Spring");
}

/*
 * We don't actually process the param separately since the extra inputs check looks at both
 * template <> void FXConfig<fxt_spring_reverb>::processSpecificParams(FX<fxt_spring_reverb> *m)
 */

template <>
void FXConfig<fxt_spring_reverb>::processExtraInputs(FX<fxt_spring_reverb> *that, int channel)
{
    auto uc = channel * (that->inputs[FX<fxt_spring_reverb>::INPUT_SPECIFIC_0].getChannels() > 1);
    auto t = that->extraInputTriggers[1 + channel].process(
        that->inputs[FX<fxt_spring_reverb>::INPUT_SPECIFIC_0].getVoltage(uc));
    auto d = that->extraInputTriggers[0].process(
        that->params[FX<fxt_spring_reverb>::FX_SPECIFIC_PARAM_0].getValue(), 0.5);

    if (t || d)
    {
        that->fxstorage->p[6].set_value_f01(1);
    }
    else
    {
        that->fxstorage->p[6].set_value_f01(0);
    }
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
