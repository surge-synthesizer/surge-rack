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

    const auto bottomRow = FXLayoutHelper::rowStart_MM;
    const auto topRow = bottomRow - FXLayoutHelper::unlabeledGap_MM;
    const auto bigRow = (bottomRow + topRow) * 0.5f;

    const auto endOfPanel = topRow - 14;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "DRIVE", chowdsp::ExciterEffect::exciter_drive, (col[1]+col[0]) * 0.5, bigRow},

        {LayoutItem::KNOB9, "ATTACK", chowdsp::ExciterEffect::exciter_att, col[2], topRow},
        {LayoutItem::KNOB9, "RELEASE", chowdsp::ExciterEffect::exciter_rel, col[3], topRow},
        LayoutItem::createGrouplabel("SHAPE", col[2], topRow, 2),

        {LayoutItem::KNOB9, "TONE", chowdsp::ExciterEffect::exciter_tone, col[2], bottomRow},
        {LayoutItem::KNOB9, "MIX", chowdsp::ExciterEffect::exciter_mix, col[3], bottomRow},
        LayoutItem::createLCDArea(endOfPanel)
        // clang-format on
    };
}
} // namespace sst::surgext_rack::fx
#endif // RACK_HACK_ROTARYSPEAKER_H
