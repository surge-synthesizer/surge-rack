#ifndef SURGEXT_CLASSIC_CONFIG_H
#define SURGEXT_CLASSIC_CONFIG_H

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_classic>::supportsUnison() { return true; }
template <> VCOConfig<ot_classic>::layout_t VCOConfig<ot_classic>::getLayout()
{
    typedef VCO<ot_classic> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 0, "SHAPE", 0, 1),
        LayoutItem::createVCOKnob(cp + 1, "WIDTH1", 0, 2),
        LayoutItem::createVCOKnob(cp + 2, "WIDTH2", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOKnob(cp + 3, "SUBMIX", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "SYNC", 1, 3),
        // clang-format on
    };
}
} // namespace sst::surgext_rack::vco

#endif
