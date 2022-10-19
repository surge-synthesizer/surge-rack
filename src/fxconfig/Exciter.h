//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/chowdsp/ExciterEffect.h"

#ifndef SURGEXT_RACK_FX_EXCITER_H
#define SURGEXT_RACK_FX_EXCITER_H

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_exciter>::numParams() { return 5; }
template <> FXConfig<fxt_exciter>::layout_t FXConfig<fxt_exciter>::getLayout()
{
    const auto &col = layout::LayoutConstants::columnCenters_MM;
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto bottomRow = FXLayoutHelper::rowStart_MM;
    const auto driveRow = bottomRow - 34;
    const auto colC = (col[1] + col[2]) * 0.5f;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "DRIVE", chowdsp::ExciterEffect::exciter_drive, colC, driveRow},

        {LayoutItem::KNOB9, "ATTACK", chowdsp::ExciterEffect::exciter_att, col[0], bottomRow},
        {LayoutItem::KNOB9, "RELEASE", chowdsp::ExciterEffect::exciter_rel, col[1], bottomRow},
        LayoutItem::createGrouplabel("SHAPE", col[0], bottomRow, 2),

        {LayoutItem::KNOB9, "TONE", chowdsp::ExciterEffect::exciter_tone, col[2], bottomRow},
        {LayoutItem::KNOB9, "MIX", chowdsp::ExciterEffect::exciter_mix, col[3], bottomRow},
        LayoutItem::createPresetLCDArea()
        // clang-format on
    };
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
