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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_WINDOW_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_WINDOW_H

#include "dsp/oscillators/WindowOscillator.h"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_window>::supportsUnison() { return true; }
template <> constexpr bool VCOConfig<ot_window>::requiresWavetables() { return true; }
template <> VCOConfig<ot_window>::layout_t VCOConfig<ot_window>::getLayout()
{
    typedef VCO<ot_window> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 0, "MORPH", 0, 1),
        LayoutItem::createVCOKnob(cp + 1, "FORMANT", 0, 2),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "", 1, 2),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0, 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "", 1, 3),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0 + 1, 1, 3),
        LayoutItem::createVCOSpanLabel("LO - CUT - HI", 1, 2, 2),

        // clang-format on
    };
}

template <> int VCOConfig<ot_window>::rightMenuParamId() { return 2; }

template <> void VCOConfig<ot_window>::oscillatorSpecificSetup(VCO<ot_window> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[WindowOscillator::win_morph].set_extend_range(true);
    }
}

template <> inline void VCOConfig<ot_window>::configureVCOSpecificParameters(VCO<ot_window> *m)
{
    m->configOnOff(VCO<ot_window>::ARBITRARY_SWITCH_0 + 0, 0, "Enable Low Cut");
    m->configOnOff(VCO<ot_window>::ARBITRARY_SWITCH_0 + 1, 0, "Enable High Cut");

    for (int i = 2; i < VCO<ot_window>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_window>::ARBITRARY_SWITCH_0 + i, 0, 1, 0, "Unused");
    }
}

template <> void VCOConfig<ot_window>::processVCOSpecificParameters(VCO<ot_window> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_window>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_window>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != !s->p[WindowOscillator::win_lowcut].deactivated)
            s->p[WindowOscillator::win_lowcut].deactivated = !l0;
        if (l1 != !s->p[WindowOscillator::win_highcut].deactivated)
            s->p[WindowOscillator::win_highcut].deactivated = !l1;
    }
}

} // namespace sst::surgext_rack::vco

#endif
