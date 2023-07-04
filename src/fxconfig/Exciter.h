/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
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

#include "dsp/effects/chowdsp/ExciterEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_EXCITER_H
#define SURGE_XT_RACK_SRC_FXCONFIG_EXCITER_H

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_exciter>::numParams() { return 5; }
template <> FXConfig<fxt_exciter>::layout_t FXConfig<fxt_exciter>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
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

// The exciter is super input sensitive to overflows and can generate big spikes if it
// gets them. In the VST the signal is attenuated by 6db by the half rate filter so take
// most of that out here (I don't want to go all the way to a half because...) and add
// a softclip to make sure outbound spikes dont emerge.
template <> constexpr float FXConfig<fxt_exciter>::rescaleInputFactor() { return 0.666666f; }
template <> constexpr bool FXConfig<fxt_exciter>::softclipOutput() { return true; }
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
