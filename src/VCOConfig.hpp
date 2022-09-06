//
// Created by Paul Walker on 9/3/22.
//

#ifndef SURGEXT_RACK_VCOCONFIG_HPP
#define SURGEXT_RACK_VCOCONFIG_HPP

#include "VCO.hpp"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_classic>::supportsUnison() { return true; }
template <> VCOConfig<ot_classic>::knobs_t VCOConfig<ot_classic>::getKnobs()
{
    typedef VCO<ot_classic> M;
    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0, "SHAPE"},
            {M::OSC_CTRL_PARAM_0 + 1, "WIDTH1"},
            {M::OSC_CTRL_PARAM_0 + 2, "WIDTH2"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "SUBMIX"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> VCOConfig<ot_string>::knobs_t VCOConfig<ot_string>::getKnobs()
{
    typedef VCO<ot_string> M;

    return {
        {M::PITCH_0, "PITCH"},
        {M::OSC_CTRL_PARAM_0 + 1, "EXCITER"},
        {KnobDef::PARAM, M::OSC_CTRL_PARAM_0 + 2, "S1 - DECAY - S2", 2},
        {M::OSC_CTRL_PARAM_0 + 3, ""},
        {KnobDef::INPUT, M::AUDIO_INPUT, "AUDIO"},
        {M::OSC_CTRL_PARAM_0 + 6, "STIFF"},
        {M::OSC_CTRL_PARAM_0 + 5, "BALANCE"},
        {M::OSC_CTRL_PARAM_0 + 4, "DETUNE"},
    };
}

template <> constexpr bool VCOConfig<ot_modern>::supportsUnison() { return true; }
template <> VCOConfig<ot_modern>::knobs_t VCOConfig<ot_modern>::getKnobs()
{
    typedef VCO<ot_modern> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 0, "SAW"},
            {M::OSC_CTRL_PARAM_0 + 1, "PULSE"},
            {M::OSC_CTRL_PARAM_0 + 2, "MULTI"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "WIDTH"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> VCOConfig<ot_FM2>::knobs_t VCOConfig<ot_FM2>::getKnobs()
{
    typedef VCO<ot_FM2> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 6, "FBACK"},
            {M::OSC_CTRL_PARAM_0 + 0, "M1 AMT"},
            {M::OSC_CTRL_PARAM_0 + 1, "M1 RATIO"},
            {M::OSC_CTRL_PARAM_0 + 4, "OFFSET"},
            {M::OSC_CTRL_PARAM_0 + 5, "PHASE"},
            {M::OSC_CTRL_PARAM_0 + 2, "M2 AMT"},
            {M::OSC_CTRL_PARAM_0 + 3, "M2 RATIO"}};
}

template <> constexpr bool VCOConfig<ot_shnoise>::supportsUnison() { return true; }
template <> VCOConfig<ot_shnoise>::knobs_t VCOConfig<ot_shnoise>::getKnobs()
{
    typedef VCO<ot_shnoise> M;

    return {
        {M::PITCH_0, "PITCH"},
        {M::OSC_CTRL_PARAM_0 + 0, "CORR"},
        {M::OSC_CTRL_PARAM_0 + 1, "WIDTH"},
        {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
        {KnobDef::BLANK},
        {M::OSC_CTRL_PARAM_0 + 2, "LOCUT"},
        {M::OSC_CTRL_PARAM_0 + 3, "HICUT"},
        {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"},
    };
}

} // namespace sst::surgext_rack::vco

#endif // SURGEXT_RACK_VCOCONFIG_HPP
