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

#ifndef SURGEXT_FM2_CONFIG_H
#define SURGEXT_FM2_CONFIG_H

namespace sst::surgext_rack::vco
{

template <> VCOConfig<ot_FM2>::layout_t VCOConfig<ot_FM2>::getLayout()
{
    typedef VCO<ot_FM2> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 6, "FEEDBACK", 0, 1),
        LayoutItem::createVCOKnob(cp + 0, "", 0, 2),
        LayoutItem::createVCOKnob(cp + 1, "", 0, 3),
        LayoutItem::createVCOSpanLabel("AMT - M1 - RATIO", 0, 2, 2),

        LayoutItem::createVCOKnob(cp + 4, "OFFSET", 1, 0),
        LayoutItem::createVCOKnob(cp + 5, "PHASE", 1, 1),

        LayoutItem::createVCOKnob(cp + 2, "", 1, 2),
        LayoutItem::createVCOKnob(cp + 3, "", 1, 3),
        LayoutItem::createVCOSpanLabel("AMT - M2 - RATIO", 1, 2, 2),
        // clang-format on
    };
}
} // namespace sst::surgext_rack::vco

#endif
