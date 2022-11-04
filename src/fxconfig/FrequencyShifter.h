/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "dsp/effects/FrequencyShifterEffect.h"

#ifndef SURGEXT_RACK_FX_FREQUENCYSHIFTER_H
#define SURGEXT_RACK_FX_FREQUENCYSHIFTER_H

namespace sst::surgext_rack::fx
{
template <> constexpr int FXConfig<fxt_freqshift>::numParams() { return 5; }
template <> constexpr int FXConfig<fxt_freqshift>::specificParamCount() { return 1; }
template <> FXConfig<fxt_freqshift>::layout_t FXConfig<fxt_freqshift>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto bottomRow = FXLayoutHelper::rowStart_MM;
    const auto driveRow = bottomRow - 34;

    typedef FX<fxt_freqshift> fx_t;

    return {
        // clang-format off
        {LayoutItem::KNOB14, "LEFT", FrequencyShifterEffect::freq_shift, FXLayoutHelper::bigCol0, driveRow},
        {LayoutItem::EXTEND_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, FXLayoutHelper::bigCol0, driveRow, +2.75},
        {LayoutItem::KNOB14, "RIGHT", FrequencyShifterEffect::freq_rmult, FXLayoutHelper::bigCol1, driveRow},

        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
            col[0], bottomRow },
        {LayoutItem::KNOB9, "TIME", FrequencyShifterEffect::freq_delay, col[1], bottomRow},
        {LayoutItem::KNOB9, "FEEDBACK", FrequencyShifterEffect::freq_feedback, col[2], bottomRow},
        LayoutItem::createGrouplabel("DELAY", col[0], bottomRow, 3),
        {LayoutItem::KNOB9, "MIX", FrequencyShifterEffect::freq_mix, col[3], bottomRow},

        LayoutItem::createPresetLCDArea()
        // clang-format on
    };
}
template <> constexpr bool FXConfig<fxt_freqshift>::usesClock() { return true; }
template <> void FXConfig<fxt_freqshift>::configSpecificParams(FX<fxt_freqshift> *m)
{
    typedef FX<fxt_freqshift> fx_t;
    m->configParam<modules::OnOffParamQuantity>(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 0,
                                                "Extend Frequency");

    // Limit max delay to 1second by setting the upper bound to '0' in 2^x land
    auto &p = m->fxstorage->p[FrequencyShifterEffect::freq_delay];
    m->paramQuantities[fx_t::FX_PARAM_0 + FrequencyShifterEffect::freq_delay]->maxValue =
        -p.val_min.f / (p.val_max.f - p.val_min.f);
}

template <> void FXConfig<fxt_freqshift>::processSpecificParams(FX<fxt_freqshift> *m)
{
    typedef FX<fxt_freqshift> fx_t;
    auto ext = m->fxstorage->p[FrequencyShifterEffect::freq_shift].extend_range;
    auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;

    if (parVa != ext)
    {
        m->fxstorage->p[FrequencyShifterEffect::freq_shift].set_extend_range(parVa);
    }
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_FREQUENCYSHIFTER_H
