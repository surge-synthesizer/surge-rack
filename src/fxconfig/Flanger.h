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

#include "dsp/effects/FlangerEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_FLANGER_H
#define SURGE_XT_RACK_SRC_FXCONFIG_FLANGER_H

namespace sst::surgext_rack::fx
{

/*
 * FLANGER
 * - uses a clock but otherwise pretty basic
 */

template <> constexpr int FXConfig<fxt_flanger>::numParams() { return 11; }
template <> constexpr bool FXConfig<fxt_flanger>::usesClock() { return true; }
template <> FXConfig<fxt_flanger>::layout_t FXConfig<fxt_flanger>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    const auto col15 = (col[0] + col[1]) * 0.5f;
    const auto col25 = (col[2] + col[3]) * 0.5f;

    typedef FX<fxt_flanger> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB12, "RATE", FlangerEffect::fl_rate, col15, row1},
        {LayoutItem::KNOB12, "DEPTH", FlangerEffect::fl_depth, col25, row1},

        {LayoutItem::PORT, "CLOCK", fx_t::INPUT_CLOCK, col[0], row2},
        {LayoutItem::KNOB9, "COUNT", FlangerEffect::fl_voices, col[1], row2},
        {LayoutItem::KNOB9, "PITCH", FlangerEffect::fl_voice_basepitch, col[2], row2},
        {LayoutItem::KNOB9, "SPACING", FlangerEffect::fl_voice_spacing, col[3], row2},
        LayoutItem::createGrouplabel("COMB", col[1], row2, 3),

        {LayoutItem::KNOB9, "DEPTH", FlangerEffect::fl_feedback, col[0], row3},
        {LayoutItem::KNOB9, "DAMPING", FlangerEffect::fl_damping, col[1], row3},
        LayoutItem::createGrouplabel("FEEDBACK", col[0], row3, 2),

        {LayoutItem::KNOB9, "WIDTH", FlangerEffect::fl_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", FlangerEffect::fl_mix, col[3], row3},
        LayoutItem::createGrouplabel("MIX", col[2], row3, 2),

        LayoutItem::createPresetPlusTwoArea(),
        LayoutItem::createLeftMenuItem("MODE", FlangerEffect::fl_mode),
        LayoutItem::createRightMenuItem("WAVE", FlangerEffect::fl_wave)

    };
    // clang-format on
}

} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
