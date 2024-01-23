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

#include "dsp/effects/Reverb2Effect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_REVERB2_H
#define SURGE_XT_RACK_SRC_FXCONFIG_REVERB2_H

namespace sst::surgext_rack::fx
{

/*
 * REVERB2
 */
template <> constexpr int FXConfig<fxt_reverb2>::numParams() { return 10; }
template <> FXConfig<fxt_reverb2>::layout_t FXConfig<fxt_reverb2>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::knobGap16_MM;

    return {
        // clang-format off
        {LayoutItem::KNOB14, "SIZE", Reverb2Effect::rev2_room_size, FXLayoutHelper::bigCol0, row1},
        {LayoutItem::KNOB14, "DECAY", Reverb2Effect::rev2_decay_time, FXLayoutHelper::bigCol1, row1},

        {LayoutItem::KNOB9, "DIFFUSE", Reverb2Effect::rev2_diffusion, col[0], row2},
        {LayoutItem::KNOB9, "MOD", Reverb2Effect::rev2_modulation, col[1], row2},
        {LayoutItem::KNOB9, "PRE DLY", Reverb2Effect::rev2_predelay, col[0], row3},
        {LayoutItem::KNOB9, "BUILDUP", Reverb2Effect::rev2_buildup, col[1], row3},

        {LayoutItem::KNOB9, "LO", Reverb2Effect::rev2_lf_damping, col[2], row2},
        {LayoutItem::KNOB9, "HI", Reverb2Effect::rev2_hf_damping, col[3], row2},
        LayoutItem::createGrouplabel("DAMP", col[2], row2, 2),
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
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_REVERB2_H
