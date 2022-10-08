#ifndef SURGEXT_SHNOISE_CONFIG_H
#define SURGEXT_SHNOISE_CONFIG_H

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

        // clang-format on
    };
}

template <> void VCOConfig<ot_shnoise>::processLightParameters(VCO<ot_shnoise> *m)
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
