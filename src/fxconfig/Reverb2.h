//
// Created by Paul Walker on 9/27/22.
//

#include "dsp/effects/Reverb2Effect.h"

#ifndef RACK_HACK_REVERB2_H
#define RACK_HACK_REVERB2_H

namespace sst::surgext_rack::fx
{

/*
 * REVERB2
 */
template <> FXConfig<fxt_reverb2>::layout_t FXConfig<fxt_reverb2>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::knobGap16_MM;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "SIZE", Reverb2Effect::rev2_room_size, FXLayoutHelper::bigCol0, row1},
        {LayoutItem::KNOB16, "DECAY", Reverb2Effect::rev2_decay_time, FXLayoutHelper::bigCol1, row1},

        {LayoutItem::KNOB9, "PRE-D", Reverb2Effect::rev2_predelay, col[0], row2},
        {LayoutItem::KNOB9, "DIFFUSE", Reverb2Effect::rev2_diffusion, col[1], row2},
        {LayoutItem::KNOB9, "MOD", Reverb2Effect::rev2_modulation, col[2], row2},
        {LayoutItem::KNOB9, "BUILDUP", Reverb2Effect::rev2_buildup, col[3], row2},
        LayoutItem::createGrouplabel("REVERB", col[1], row2, 3),

        {LayoutItem::KNOB9, "LO", Reverb2Effect::rev2_lf_damping, col[0], row3},
        {LayoutItem::KNOB9, "HI", Reverb2Effect::rev2_hf_damping, col[1], row3},
        LayoutItem::createGrouplabel("EQ", col[0], row3, 2),
        {LayoutItem::KNOB9, "WIDTH", Reverb2Effect::rev2_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", Reverb2Effect::rev2_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),


        LayoutItem::createPresetLCDArea(),
        // clang-format on
    };
}
/*
 * END OF REVERB2
 */
}
#endif // RACK_HACK_REVERB2_H
