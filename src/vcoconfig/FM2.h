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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_FM2_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_FM2_H

#include "dsp/oscillators/FM2Oscillator.h"

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
        LayoutItem::createVCOLight(LayoutItem::VINTAGE_LIGHT, M::ARBITRARY_SWITCH_0 + 1, 0, 1, false),
        LayoutItem::createVCOLight(LayoutItem::EXTEND_LIGHT, M::ARBITRARY_SWITCH_0 + 0, 0, 1, true),

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

template <> inline void VCOConfig<ot_FM2>::configureVCOSpecificParameters(VCO<ot_FM2> *m)
{
    m->configOnOff(VCO<ot_FM2>::ARBITRARY_SWITCH_0 + 0, 0, "Extend Feedback");
    m->configOnOff(VCO<ot_FM2>::ARBITRARY_SWITCH_0 + 1, 0, "Enable Vintage Feedback");

    for (int i = 2; i < VCO<ot_FM2>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_FM2>::ARBITRARY_SWITCH_0 + i, 0, 1, 0, "Unused");
    }
}

template <> void VCOConfig<ot_FM2>::processVCOSpecificParameters(VCO<ot_FM2> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_FM2>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_FM2>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[FM2Oscillator::fm2_feedback].set_extend_range(l0);
        s->p[FM2Oscillator::fm2_feedback].deform_type = (l1 ? 1 : 0);
    }
}
} // namespace sst::surgext_rack::vco

#endif
