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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_CLASSIC_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_CLASSIC_H

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_classic>::supportsUnison() { return true; }
template <> VCOConfig<ot_classic>::layout_t VCOConfig<ot_classic>::getLayout()
{
    typedef VCO<ot_classic> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 0, "SHAPE", 0, 1),
        LayoutItem::createVCOKnob(cp + 1, "", 0, 2),
        LayoutItem::createVCOKnob(cp + 2, "", 0, 3),
        LayoutItem::createVCOSpanLabel("1- WIDTH - 2", 0, 2, 2),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "SUB MIX", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "SYNC", 1, 3),
        // clang-format on
    };
}
} // namespace sst::surgext_rack::vco

#endif
