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

#ifndef SURGEXT_MODERN_CONFIG_H
#define SURGEXT_MODERN_CONFIG_H

#include "dsp/oscillators/ModernOscillator.h"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_modern>::supportsUnison() { return true; }
template <> VCOConfig<ot_modern>::layout_t VCOConfig<ot_modern>::getLayout()
{
    typedef VCO<ot_modern> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + ModernOscillator::mo_saw_mix, "SAW", 0, 1),
        LayoutItem::createVCOKnob(cp + ModernOscillator::mo_pulse_mix, "PULSE", 0, 2),
        LayoutItem::createVCOKnob(cp + ModernOscillator::mo_tri_mix, "", 0, 3),
        LayoutItem::createVCOSpanDynamicLabel(
                [](auto *m) -> std::string
                {
                    auto l0 = (int)std::round(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 0].getValue());
                    auto l1 = (bool)(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
                    std::string res = "";
                    if (l1)
                        res = "SUB";
                    switch (l0)
                    {
                        case ModernOscillator::momt_triangle:
                            res += "TRI";
                            break;
                        case ModernOscillator::momt_square:
                            res += "SQR";
                            break;
                        case ModernOscillator::momt_sine:
                            res += "SINE";
                            break;
                        default:
                            res += "ERR";
                            break;
                    }
                    return res;
                },
            0, 3, 1),

        LayoutItem::createVCOKnob(cp + ModernOscillator::mo_unison_detune, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + ModernOscillator::mo_pulse_width, "WIDTH", 1, 2),
        LayoutItem::createVCOKnob(cp + ModernOscillator::mo_sync, "SYNC", 1, 3),
        // clang-format on
    };
}

template <> void VCOConfig<ot_modern>::oscillatorSpecificSetup(VCO<ot_modern> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        // All Saw
        s->p[0].set_value_f01(1.0);
    }
}

template <> void VCOConfig<ot_modern>::configureVCOSpecificParameters(VCO<ot_modern> *m)
{
    auto bs = VCO<ot_modern>::ARBITRARY_SWITCH_0;
    m->configParam(bs, ModernOscillator::momt_triangle, ModernOscillator::momt_sine,
                   ModernOscillator::momt_triangle, "Shape");
    m->configParam(bs + 1, 0, 1, 0, "Sub");
    m->configParam(bs + 2, 0, 1, 0, "SubSync");
    m->configParam(bs + 3, 0, 1, 0, "Unused");
}

template <> void VCOConfig<ot_modern>::processVCOSpecificParameters(VCO<ot_modern> *m)
{
    auto l0 = (int)std::round(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 0].getValue());
    auto l1 = (bool)(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
    auto l2 = (bool)(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 2].getValue() > 0.5);

    auto def =
        l0 | (l1 ? ModernOscillator::mo_subone : 0) | (l2 ? ModernOscillator::mo_subskipsync : 0);
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[ModernOscillator::mo_tri_mix].deform_type = def;
    }
}

template <> void VCOConfig<ot_modern>::addMenuItems(VCO<ot_modern> *m, rack::ui::Menu *toThis)
{
    toThis->addChild(new rack::ui::MenuSeparator);

    auto l0 = (int)std::round(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 0].getValue());

    for (int i = 0; i < 3; ++i)
    {
        std::string name = "Multi ERROR";
        switch (i)
        {
        case ModernOscillator::momt_triangle:
            name = "Multi: Triangle";
            break;
        case ModernOscillator::momt_square:
            name = "Multi: Square";
            break;
        case ModernOscillator::momt_sine:
            name = "Multi: Sine";
            break;
        }
        toThis->addChild(rack::createMenuItem(name, CHECKMARK(l0 == i), [m, i]() {
            m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 0].setValue(i);
        }));
    }

    toThis->addChild(new rack::MenuSeparator);
    auto l1 = (bool)(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
    toThis->addChild(rack::createMenuItem(("Multi: Sub-Oscillator"), CHECKMARK(l1), [m, l1]() {
        m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 1].setValue(!l1);
    }));
    auto l2 = (bool)(m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 2].getValue() > 0.5);
    toThis->addChild(rack::createMenuItem(("Multi: Sub Skips Sync"), CHECKMARK(l2), [m, l2]() {
        m->params[VCO<ot_modern>::ARBITRARY_SWITCH_0 + 2].setValue(!l2);
    }));
}
} // namespace sst::surgext_rack::vco

#endif
