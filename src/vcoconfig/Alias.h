#ifndef SURGEXT_ALIAS_CONFIG_H
#define SURGEXT_ALIAS_CONFIG_H

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_alias>::supportsUnison() { return true; }
template <> VCOConfig<ot_alias>::layout_t VCOConfig<ot_alias>::getLayout()
{
    typedef VCO<ot_alias> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 1, "WARP", 0, 2),
        LayoutItem::createVCOKnob(cp + 2, "MASK", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOPort(M::AUDIO_INPUT, "AUDIO", 1, 1),

        LayoutItem::createVCOKnob(cp + 3, "THRESH", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "CRUSH", 1, 3)
        // clang-format on
    };
}
template <> int VCOConfig<ot_alias>::rightMenuParamId() { return 0; }
template <> constexpr bool VCOConfig<ot_alias>::supportsAudioIn() { return true; }

} // namespace sst::surgext_rack::vco

#endif
