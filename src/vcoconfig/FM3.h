#ifndef SURGEXT_FM3_CONFIG_H
#define SURGEXT_FM3_CONFIG_H

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
        LayoutItem::createVCOKnob(cp + 0, "", 0, 2),
        LayoutItem::createVCOKnob(cp + 1, "", 0, 3),
        LayoutItem::createVCOLight(LayoutItem::ABSOLUTE_LIGHT, M::ARBITRARY_SWITCH_0, 0, 3),

        LayoutItem::createVCOSpanLabel("AMT - M1 - RATIO", 0, 2, 2),

        LayoutItem::createVCOKnob(cp + 2, "", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "", 1, 1),
        LayoutItem::createVCOLight(LayoutItem::ABSOLUTE_LIGHT, M::ARBITRARY_SWITCH_0 + 1, 1, 1),
        LayoutItem::createVCOSpanLabel("AMT - M2 - RATIO", 1, 2, 2),

        LayoutItem::createVCOKnob(cp + 4, "", 1, 2),
        LayoutItem::createVCOKnob(cp + 5, "", 1, 3),
        LayoutItem::createVCOSpanLabel("AMT - M3 - RATIO", 1, 2, 2),

        // clang-format on
    };
}

template <> void VCOConfig<ot_FM3>::processLightParameters(VCO<ot_FM3> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != s->p[FM3Oscillator::fm3_m1ratio].absolute)
            s->p[FM3Oscillator::fm3_m1ratio].absolute = l0;
        if (l1 != s->p[FM3Oscillator::fm3_m2ratio].absolute)
            s->p[FM3Oscillator::fm3_m2ratio].absolute = l1;
    }
}

} // namespace sst::surgext_rack::vco

#endif
