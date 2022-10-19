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

#ifndef SURGEXT_CLASSIC_CONFIG_H
#define SURGEXT_CLASSIC_CONFIG_H

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
        LayoutItem::createVCOKnob(cp + 1, "WIDTH1", 0, 2),
        LayoutItem::createVCOKnob(cp + 2, "WIDTH2", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "SUBMIX", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "SYNC", 1, 3),
        // clang-format on
    };
}
} // namespace sst::surgext_rack::vco

#endif
