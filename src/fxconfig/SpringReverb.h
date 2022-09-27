//
// Created by Paul Walker on 9/27/22.
//

// We could include this to get the mappings but that pulls in juce at the
// wrong level so for SR and SR only don't use the enums
// #include "dsp/effects/chowdsp/SpringReverbEffect.h"

#ifndef RACK_HACK_SPRING_REVERB_H
#define RACK_HACK_SPRING_REVERB_H

namespace sst::surgext_rack::fx
{
template <> constexpr int FXConfig<fxt_spring_reverb>::extraInputs() { return 1; }
template <> FXConfig<fxt_spring_reverb>::layout_t FXConfig<fxt_spring_reverb>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto thirdRow = FXLayoutHelper::rowStart_MM;
    const auto secondRow = thirdRow - FXLayoutHelper::labeledGap_MM;
    const auto firstRow = secondRow - FXLayoutHelper::unlabeledGap_MM;
    const auto bigRow = (secondRow + firstRow) * 0.5f;

    const auto endOfPanel = firstRow - 7;

    // fixme use the enums
    // clang-format off
    return {
        {LayoutItem::KNOB16, "SIZE", 0, col[1] - 10, bigRow},
        {LayoutItem::KNOB16, "DECAY", 1, col[1] + 10, bigRow},
        {LayoutItem::KNOB9, "REFL", 2, col[3], firstRow},
        {LayoutItem::KNOB9, "DAMP", 3, col[3], secondRow},

        {LayoutItem::PORT, "KNOCK", FX<fxt_spring_reverb>::INPUT_SPECIFIC_0, col[0], thirdRow},
        {LayoutItem::KNOB9, "SPIN", 4, col[1], thirdRow},
        {LayoutItem::KNOB9, "CHAOS", 5, col[2], thirdRow},
        LayoutItem::createGrouplabel("MODULATION", col[0], thirdRow, 3),
        {LayoutItem::KNOB9, "MIX", 7, col[3], thirdRow},
        LayoutItem::createLCDArea(endOfPanel),
    };

    // clang-format on
}

template <> void FXConfig<fxt_spring_reverb>::processExtraInputs(FX<fxt_spring_reverb> *that)
{
    if (that->inputs[FX<fxt_spring_reverb>::INPUT_SPECIFIC_0].isConnected())
    {
        if (that->inputs[FX<fxt_spring_reverb>::INPUT_SPECIFIC_0].getVoltage() > 3)
        {
            that->fxstorage->p[6].set_value_f01(1);
        }
        else
        {
            that->fxstorage->p[6].set_value_f01(0);
        }
    }
}
} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
