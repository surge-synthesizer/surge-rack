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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_FM3_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_FM3_H

#include "dsp/oscillators/FM3Oscillator.h"

namespace sst::surgext_rack::vco
{

template <> VCOConfig<ot_FM3>::layout_t VCOConfig<ot_FM3>::getLayout()
{
    typedef VCO<ot_FM3> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 6, "FEEDBACK", 0, 1),
        LayoutItem::createVCOLight(LayoutItem::VINTAGE_LIGHT, M::ARBITRARY_SWITCH_0 + 3, 0, 1, false),
        LayoutItem::createVCOLight(LayoutItem::EXTEND_LIGHT, M::ARBITRARY_SWITCH_0 + 2, 0, 1, true),

        LayoutItem::createVCOKnob(cp + 0, "", 0, 2),
        LayoutItem::createVCOKnob(cp + 1, "", 0, 3),

        LayoutItem::createVCOSpanDynamicLabel(
                [](auto *m) -> std::string
                {
                    auto l0 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
                    if (l0)
                        return "AMT - M1 - FREQ";
                    return "AMT - M1 - RATIO";
                },
            0, 2, 2),

        LayoutItem::createVCOKnob(cp + 2, "", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "", 1, 1),
        LayoutItem::createVCOSpanDynamicLabel(
                [](auto *m) -> std::string
                {
                    auto l0 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
                    if (l0)
                        return "AMT - M2 - FREQ";
                    return "AMT - M2 - RATIO";
                },
            1, 0, 2),

        LayoutItem::createVCOKnob(cp + 4, "", 1, 2),
        LayoutItem::createVCOKnob(cp + 5, "", 1, 3),
        LayoutItem::createVCOSpanLabel("AMT - M3 - FREQ", 1, 2, 2),

        // clang-format on
    };
}

template <> inline void VCOConfig<ot_FM3>::configureVCOSpecificParameters(VCO<ot_FM3> *m)
{
    m->configOnOff(VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0, 0, "M1 Ratio Absolute");
    m->configOnOff(VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1, 0, "M2 Ratio Absolute");

    m->configOnOff(VCO<ot_FM2>::ARBITRARY_SWITCH_0 + 2, 0, "Extend Feedback");
    m->configOnOff(VCO<ot_FM2>::ARBITRARY_SWITCH_0 + 3, 0, "Enable Vintage Feedback");

    for (int i = 4; i < VCO<ot_FM2>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_FM2>::ARBITRARY_SWITCH_0 + i, 0, 1, 0, "Unused");
    }
}
template <> void VCOConfig<ot_FM3>::processVCOSpecificParameters(VCO<ot_FM3> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    auto l2 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 2].getValue() > 0.5);
    auto l3 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 3].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != s->p[FM3Oscillator::fm3_m1ratio].absolute)
            s->p[FM3Oscillator::fm3_m1ratio].absolute = l0;
        if (l1 != s->p[FM3Oscillator::fm3_m2ratio].absolute)
            s->p[FM3Oscillator::fm3_m2ratio].absolute = l1;

        s->p[FM3Oscillator::fm3_feedback].set_extend_range(l2);
        s->p[FM3Oscillator::fm3_feedback].deform_type = (l3 ? 1 : 0);
    }
}

template <> void VCOConfig<ot_FM3>::addMenuItems(VCO<ot_FM3> *m, rack::ui::Menu *toThis)
{
    toThis->addChild(new rack::ui::MenuSeparator);

    auto l0 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    toThis->addChild(rack::createMenuItem(("M1 Absolute"), CHECKMARK(l0), [m, l0]() {
        m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0].setValue(!l0);
    }));

    auto l1 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
    toThis->addChild(rack::createMenuItem(("M2 Absolute"), CHECKMARK(l1), [m, l1]() {
        m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1].setValue(!l1);
    }));
}

} // namespace sst::surgext_rack::vco

#endif
