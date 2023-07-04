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

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_BONSAI_H
#define SURGE_XT_RACK_SRC_FXCONFIG_BONSAI_H

#include "dsp/effects/BonsaiEffect.h"

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_bonsai>::numParams() { return 11; }

template <> FXConfig<fxt_bonsai>::layout_t FXConfig<fxt_bonsai>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    const auto col15 = (col[0] + col[1]) * 0.5f;
    const auto col25 = (col[2] + col[3]) * 0.5f;

    // clang-format off
    auto res = FXConfig<fxt_bonsai>::layout_t{

        {LayoutItem::KNOB12, "SATURATION", BonsaiEffect::b_tape_sat, col15, row1},
        {LayoutItem::KNOB12, "DULL", BonsaiEffect::b_dull, col25, row1},

        {LayoutItem::KNOB9, "AMOUNT", BonsaiEffect::b_bass_boost, col[0], row2},
        {LayoutItem::KNOB9, "DISTORT", BonsaiEffect::b_bass_distort, col[1], row2},
        LayoutItem::createGrouplabel("BASS BOOST", col[0], row2, 2),
        {LayoutItem::KNOB9, "SENS", BonsaiEffect::b_noise_sensitivity, col[2], row2},
        {LayoutItem::KNOB9, "GAIN", BonsaiEffect::b_noise_gain, col[3], row2},
        LayoutItem::createGrouplabel("NOISE", col[2], row2, 2),

        {LayoutItem::KNOB9, "INPUT", BonsaiEffect::b_gain_in, col[0], row3},
        {LayoutItem::KNOB9, "OUTPUT", BonsaiEffect::b_gain_out, col[1], row3},

        {LayoutItem::KNOB9, "MIX", BonsaiEffect::b_mix, col[3], row3},
        LayoutItem::createGrouplabel("GAIN", col[0], row3, 2),


        LayoutItem::createPresetPlusTwoArea(),
        LayoutItem::createLeftMenuItem("BIAS", BonsaiEffect::b_tape_bias_mode),
        LayoutItem::createRightMenuItem("DISTORTION",BonsaiEffect::b_tape_dist_mode)
    };
    // clang-format on

    return res;
}

} // namespace sst::surgext_rack::fx

#endif // SURGEXTRACK_NIMBUS_H
