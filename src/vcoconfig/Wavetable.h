#ifndef SURGEXT_WAVETABLE_CONFIG_H
#define SURGEXT_WAVETABLE_CONFIG_H

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_wavetable>::supportsUnison() { return true; }
template <> constexpr bool VCOConfig<ot_wavetable>::requiresWavetables() { return true; }
template <> VCOConfig<ot_wavetable>::layout_t VCOConfig<ot_wavetable>::getLayout()
{
    typedef VCO<ot_wavetable> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 0, "MORPH", 0, 1),
        LayoutItem::createVCOKnob(cp + 2, "SAT", 0, 2),
        LayoutItem::createVCOKnob(cp + 3, "FORMANT", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 1, "", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "", 1, 3),
        LayoutItem::createVCOSpanLabel("V - SKEW - H", 1, 2, 2),

        // clang-format on
    };
}

} // namespace sst::surgext_rack::vco

#endif
