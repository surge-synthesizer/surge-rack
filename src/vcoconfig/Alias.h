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

#ifndef SURGEXT_ALIAS_CONFIG_H
#define SURGEXT_ALIAS_CONFIG_H

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_alias>::supportsUnison() { return true; }
template <> constexpr int VCOConfig<ot_alias>::additionalVCOParameterCount() { return 16; }
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

template <> void VCOConfig<ot_alias>::configureVCOSpecificParameters(VCO<ot_alias> *m)
{
    for (int i = 0; i < additionalVCOParameterCount(); ++i)
    {
        m->configParam(VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i, -1, 1, 1.0 / (i + 1),
                       std::string("Additive Harmonic ") + std::to_string(i + 1));
    }

    for (int i = 0; i < VCO<ot_alias>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_alias>::ARBITRARY_SWITCH_0 + i, 0, 1, 0,
                       std::string("Unused Param ") + std::to_string(i + 1));
    }
}

} // namespace sst::surgext_rack::vco

#endif
