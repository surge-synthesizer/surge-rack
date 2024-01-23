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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_WAVETABLE_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_WAVETABLE_H

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_wavetable>::supportsUnison() { return true; }
template <> constexpr bool VCOConfig<ot_wavetable>::requiresWavetables() { return true; }
template <> VCOConfig<ot_wavetable>::layout_t VCOConfig<ot_wavetable>::getLayout()
{
    typedef VCO<ot_wavetable> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 0, "MORPH", 0, 1),
        LayoutItem::createVCOKnob(cp + 2, "SAT", 0, 2),
        LayoutItem::createVCOKnob(cp + 3, "FORMANT", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 1, "", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "", 1, 3),
        LayoutItem::createVCOSpanLabel("V - SKEW - H", 1, 2, 2),
        LayoutItem::createVCOLight(LayoutItem::EXTEND_LIGHT, M::EXTEND_UNISON, 1, 0)
        // clang-format on
    };
}

} // namespace sst::surgext_rack::vco

#endif
