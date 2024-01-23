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

// Because of JUCE we can't include this here currently
// #include "dsp/effects/chowdsp/NeuronEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_NEURON_H
#define SURGE_XT_RACK_SRC_FXCONFIG_NEURON_H

namespace sst::surgext_rack::fx
{

// So because of that include copy the enum
enum n_t
{
    neuron_drive_wh = 0,
    neuron_squash_wf,
    neuron_stab_uf,
    neuron_asym_uh,
    neuron_bias_bf,

    neuron_comb_freq,
    neuron_comb_sep,

    neuron_lfo_wave,
    neuron_lfo_rate,
    neuron_lfo_depth,

    neuron_width,
    neuron_gain,

    neuron_num_params,
};

/*
 */
template <> constexpr bool FXConfig<fxt_neuron>::usesClock() { return true; }
template <> FXConfig<fxt_neuron>::layout_t FXConfig<fxt_neuron>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_neuron> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB9, "DRIVE", n_t::neuron_drive_wh, col[0], row1},
        {LayoutItem::KNOB9, "SQUASH", n_t::neuron_squash_wf, col[1], row1},
        {LayoutItem::KNOB9, "STAB", n_t::neuron_stab_uf, col[2], row1},
        {LayoutItem::KNOB9, "ASYM", n_t::neuron_asym_uh, col[3], row1},

        {LayoutItem::PORT, "CLOCK", fx_t::INPUT_CLOCK, col[0], row2},
        {LayoutItem::KNOB9, "RATE", n_t::neuron_lfo_rate, col[1], row2},
        {LayoutItem::KNOB9, "DEPTH", n_t::neuron_lfo_depth, col[2], row2},
        LayoutItem::createGrouplabel("MOD", col[0], row2, 3),
        {LayoutItem::KNOB9, "BIAS", n_t::neuron_bias_bf, col[3], row2},

        {LayoutItem::KNOB9, "FREQ", n_t::neuron_comb_freq, col[0], row3},
        {LayoutItem::KNOB9, "SPACING", n_t::neuron_comb_sep, col[1], row3},
        LayoutItem::createGrouplabel("COMB", col[0], row3, 2),

        {LayoutItem::KNOB9, "WIDTH", n_t::neuron_width, col[2], row3},
        {LayoutItem::KNOB9, "GAIN", n_t::neuron_gain, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),

        LayoutItem::createPresetPlusOneArea(),
        LayoutItem::createSingleMenuItem("WAVE", n_t::neuron_lfo_wave)
    };
    // clang-format on
}

} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
