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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_SHNOISE_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_SHNOISE_H

#include "dsp/oscillators/SampleAndHoldOscillator.h"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_shnoise>::supportsUnison() { return true; }
template <> VCOConfig<ot_shnoise>::layout_t VCOConfig<ot_shnoise>::getLayout()
{
    typedef VCO<ot_shnoise> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 0, "CORREL", 0, 1),
        LayoutItem::createVCOKnob(cp + 1, "WIDTH", 0, 2),
        LayoutItem::createVCOKnob(cp + 4, "SYNC", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 2, "", 1, 2),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0, 1, 2),
        LayoutItem::createVCOKnob(cp + 3, "", 1, 3),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0 + 1, 1, 3),
        LayoutItem::createVCOSpanLabel("LO - CUT - HI", 1, 2, 2),
        LayoutItem::createVCOLight(LayoutItem::EXTEND_LIGHT, M::EXTEND_UNISON, 1, 0)
        // clang-format on
    };
}

template <> inline void VCOConfig<ot_shnoise>::configureVCOSpecificParameters(VCO<ot_shnoise> *m)
{
    m->configOnOff(VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + 0, 0, "Enable Low Cut");
    m->configOnOff(VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + 1, 0, "Enable High Cut");

    for (int i = 2; i < VCO<ot_shnoise>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + i, 0, 1, 0, "Unused");
    }
}

template <> void VCOConfig<ot_shnoise>::processVCOSpecificParameters(VCO<ot_shnoise> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != !s->p[SampleAndHoldOscillator::shn_lowcut].deactivated)
            s->p[SampleAndHoldOscillator::shn_lowcut].deactivated = !l0;
        if (l1 != !s->p[SampleAndHoldOscillator::shn_highcut].deactivated)
            s->p[SampleAndHoldOscillator::shn_highcut].deactivated = !l1;
    }
}
} // namespace sst::surgext_rack::vco

#endif
