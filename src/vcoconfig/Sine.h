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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_SINE_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_SINE_H

#include "dsp/oscillators/SineOscillator.h"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_sine>::supportsUnison() { return true; }
template <> VCOConfig<ot_sine>::layout_t VCOConfig<ot_sine>::getLayout()
{
    typedef VCO<ot_sine> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 1, "FEEDBACK", 0, 1),
        LayoutItem::createVCOLight(LayoutItem::VINTAGE_LIGHT, M::ARBITRARY_SWITCH_0 + 2, 0, 1),

        LayoutItem::createVCOKnob(cp + 4, "", 1, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "", 1, 2),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0, 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "", 1, 3),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0 + 1, 1, 3),
        LayoutItem::createVCOSpanLabel("LO - CUT - HI", 1, 2, 2),

        LayoutItem::createVCOLight(LayoutItem::EXTEND_LIGHT, M::EXTEND_UNISON, 1, 0)
        // clang-format on
    };
}

template <> int VCOConfig<ot_sine>::rightMenuParamId() { return 0; }

template <> inline void VCOConfig<ot_sine>::configureVCOSpecificParameters(VCO<ot_sine> *m)
{
    m->configOnOff(VCO<ot_sine>::ARBITRARY_SWITCH_0 + 0, 0, "Enable Low Cut");
    m->configOnOff(VCO<ot_sine>::ARBITRARY_SWITCH_0 + 1, 0, "Enable High Cut");
    m->configOnOff(VCO<ot_sine>::ARBITRARY_SWITCH_0 + 2, 0, "Enable Vintage Feedback");

    for (int i = 3; i < VCO<ot_sine>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_sine>::ARBITRARY_SWITCH_0 + i, 0, 1, 0, "Unused");
    }
}
template <> void VCOConfig<ot_sine>::oscillatorSpecificSetup(VCO<ot_sine> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[SineOscillator::sine_feedback].set_extend_range(true);
    }
}

template <> void VCOConfig<ot_sine>::processVCOSpecificParameters(VCO<ot_sine> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_sine>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_sine>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
    auto l2 = (bool)(m->params[VCO<ot_sine>::ARBITRARY_SWITCH_0 + 2].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != !s->p[SineOscillator::sine_lowcut].deactivated)
            s->p[SineOscillator::sine_lowcut].deactivated = !l0;
        if (l1 != !s->p[SineOscillator::sine_highcut].deactivated)
            s->p[SineOscillator::sine_highcut].deactivated = !l1;
        s->p[SineOscillator::sine_feedback].deform_type = (l2 ? 1 : 0);
    }
}
} // namespace sst::surgext_rack::vco

#endif
