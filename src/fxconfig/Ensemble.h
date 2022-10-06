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
template <> FXConfig<fxt_ensemble>::layout_t FXConfig<fxt_ensemble>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    const auto col15 = (col[0] + col[1]) * 0.5f;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_ensemble> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB9, "RATE", BBDEnsembleEffect::ens_lfo_freq1, col[0], row1},
        {LayoutItem::KNOB9, "DEPTH", BBDEnsembleEffect::ens_lfo_depth1, col[1], row1},
        LayoutItem::createGrouplabel("LFO1", col[0], row1, 2),
        {LayoutItem::KNOB9, "RATE", BBDEnsembleEffect::ens_lfo_freq2, col[2], row1},
        {LayoutItem::KNOB9, "DEPTH", BBDEnsembleEffect::ens_lfo_depth2, col[3], row1},
        LayoutItem::createGrouplabel("LFO2", col[2], row1, 2),

        {LayoutItem::KNOB9, "FILTER", BBDEnsembleEffect::ens_input_filter, col[0], row2},
        LayoutItem::createGrouplabel("INPUT", col[0], row2, 1),
        {LayoutItem::KNOB9, "RATE", BBDEnsembleEffect::ens_delay_clockrate, col[1], row2},
        {LayoutItem::KNOB9, "SAT", BBDEnsembleEffect::ens_delay_sat, col[2], row2},
        {LayoutItem::KNOB9, "FDBACK", BBDEnsembleEffect::ens_delay_feedback, col[3], row2},
        LayoutItem::createGrouplabel("DELAY", col[1], row2, 3),

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
