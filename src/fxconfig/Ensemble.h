//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/BBDEnsembleEffect.h"

#ifndef RACK_HACK_ENSEMBLE_H
#define RACK_HACK_ENSEMBLE_H

namespace sst::surgext_rack::fx
{

/*
 */

template <> constexpr int FXConfig<fxt_ensemble>::numParams() { return 11; }
template <> FXConfig<fxt_ensemble>::layout_t FXConfig<fxt_ensemble>::getLayout()
{
    const auto &col = layout::LayoutConstants::columnCenters_MM;
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    // clang-format off
    return {
        {LayoutItem::KNOB12, "LFO RATE 1",  BBDEnsembleEffect::ens_lfo_freq1, FXLayoutHelper::bigCol0, row1},
        {LayoutItem::KNOB12, "LFO RATE 2",  BBDEnsembleEffect::ens_lfo_freq2, FXLayoutHelper::bigCol1, row1},

        {LayoutItem::KNOB9, "IN FILTER", BBDEnsembleEffect::ens_input_filter, col[0], row2},
        {LayoutItem::KNOB9, "CLOCK", BBDEnsembleEffect::ens_delay_clockrate, col[1], row2},
        LayoutItem::createGrouplabel("BBD", col[0], row2, 2),

        {LayoutItem::KNOB9, "LFO1", BBDEnsembleEffect::ens_lfo_depth1, col[2], row2},
        {LayoutItem::KNOB9, "LFO2", BBDEnsembleEffect::ens_lfo_depth2, col[3], row2},
        LayoutItem::createGrouplabel("DEPTH", col[2], row2, 2),

        {LayoutItem::KNOB9, "FEEDBACK", BBDEnsembleEffect::ens_delay_feedback, col[0], row3},
        {LayoutItem::KNOB9, "SAT", BBDEnsembleEffect::ens_delay_sat, col[1], row3},
        LayoutItem::createGrouplabel("DELAY", col[0], row3, 2),

        {LayoutItem::KNOB9, "WIDTH", BBDEnsembleEffect::ens_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", BBDEnsembleEffect::ens_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createSingleMenuItem("TYPE", BBDEnsembleEffect::ens_delay_type)
    };
    // clang-format on
}

} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
