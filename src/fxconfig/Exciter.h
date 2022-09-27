//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/chowdsp/ExciterEffect.h"

#ifndef RACK_HACK_EXCITER_H
#define RACK_HACK_EXCITER_H

namespace sst::surgext_rack::fx
{
template <> FXConfig<fxt_exciter>::layout_t FXConfig<fxt_exciter>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto bottomRow = FXLayoutHelper::rowStart_MM - 6;
    const auto driveRow = bottomRow - 18;
    const auto topRow = driveRow - 18;
    // const auto topRow = bottomRow - FXLayoutHelper::unlabeledGap_MM;
    const auto bigRow = (bottomRow + topRow) * 0.5f;

    const auto colL = col[0] + 3;
    const auto colR = col[3] - 3;
    const auto colC = (col[1] + col[2]) * 0.5f;

    const auto endOfPanel = topRow - 14;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "DRIVE", chowdsp::ExciterEffect::exciter_drive, colC, topRow},

        {LayoutItem::KNOB12, "ATTACK", chowdsp::ExciterEffect::exciter_att, colL, driveRow},
        {LayoutItem::KNOB12, "RELEASE", chowdsp::ExciterEffect::exciter_rel, colR, driveRow},
        //LayoutItem::createGrouplabel("SHAPE", col[2], topRow, 2),

        {LayoutItem::KNOB12, "TONE", chowdsp::ExciterEffect::exciter_tone, col[1]-3, bottomRow +3},
        {LayoutItem::KNOB12, "MIX", chowdsp::ExciterEffect::exciter_mix, col[2]+3, bottomRow +3},
        LayoutItem::createPresetLCDArea()
        // clang-format on
    };
}
} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
