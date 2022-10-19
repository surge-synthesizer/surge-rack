/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

// Because of JUCE we can't include this here currently
#include "dsp/effects/VocoderEffect.h"

#ifndef SURGEXT_RACK_FX_VOCODER_H
#define SURGEXT_RACK_FX_VOCODER_H

namespace sst::surgext_rack::fx
{

/*
 */
template <> constexpr bool FXConfig<fxt_vocoder>::usesSideband() { return true; }
template <> FXConfig<fxt_vocoder>::layout_t FXConfig<fxt_vocoder>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_vocoder> fx_t;
    typedef VocoderEffect v_t;

    // clang-format off
    return {
        {LayoutItem::KNOB9, "GAIN", v_t::voc_input_gain, col[0], row1},
        {LayoutItem::KNOB9, "GATE", v_t::voc_input_gate, col[1], row1},
        LayoutItem::createGrouplabel("IN", col[0], row1, 2),

        {LayoutItem::KNOB9, "", v_t::voc_minfreq, col[2], row1},
        {LayoutItem::KNOB9, "", v_t::voc_maxfreq, col[3], row1},
        LayoutItem::createGrouplabel("CARRIER", col[2], row1, 2),
        LayoutItem::createKnobSpanLabel("MIN - FREQ - MAX", col[2], row1, 2),
        // Label Carrier

        {LayoutItem::KNOB9, "RANGE", v_t::voc_mod_range, col[0], row2},
        {LayoutItem::KNOB9, "CENTER", v_t::voc_mod_center, col[1], row2},
        {LayoutItem::PORT, "L/MON", fx_t::SIDEBAND_L, col[2], row2},
        {LayoutItem::PORT, "RIGHT", fx_t::SIDEBAND_R, col[3], row2},
        LayoutItem::createGrouplabel("CARRIER", col[0], row2, 4),

        {LayoutItem::KNOB9, "FOLLOW", v_t::voc_envfollow, col[0], row3},
        {LayoutItem::KNOB9, "RES", v_t::voc_q, col[1], row3},
        {LayoutItem::KNOB9, "SHIFT", v_t::voc_shift, col[2], row3},
        LayoutItem::createGrouplabel("FILTER BANK", col[0], row3, 3),

        {LayoutItem::KNOB9, "MIX", v_t::voc_mix, col[3], row3},

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createSingleMenuItem("BANDS", v_t::voc_num_bands)
    };
    // clang-format on
}

} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
