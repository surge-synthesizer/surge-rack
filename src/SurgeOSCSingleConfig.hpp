//
// Created by Paul Walker on 9/3/22.
//

#ifndef SURGEXT_RACK_SURGEOSCSINGLECONFIG_HPP
#define SURGEXT_RACK_SURGEOSCSINGLECONFIG_HPP

#include "SurgeOSCSingle.hpp"

namespace sst::surgext_rack::vco
{

template <> constexpr bool SingleConfig<ot_modern>::supportsUnison() { return true; }

template <> constexpr bool SingleConfig<ot_classic>::supportsUnison() { return true; }
template <> SingleConfig<ot_classic>::knobs_t SingleConfig<ot_classic>::getKnobs()
{
    typedef SurgeOSCSingle<ot_classic> M;
    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0, "SHAPE"},
            {M::OSC_CTRL_PARAM_0 + 1, "WIDTH1"},
            {M::OSC_CTRL_PARAM_0 + 2, "WIDTH2"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "SUBMIX"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> SingleConfig<ot_string>::knobs_t SingleConfig<ot_string>::getKnobs()
{
    typedef SurgeOSCSingle<ot_string> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 1, "LEV"},
            {M::OSC_CTRL_PARAM_0 + 2, "S1 DC-"},
            {M::OSC_CTRL_PARAM_0 + 3, "AY S2"},
            {KnobDef::INPUT, M::AUDIO_INPUT, "AUDIO"},
            {M::OSC_CTRL_PARAM_0 + 4, "DETUNE"},
            {M::OSC_CTRL_PARAM_0 + 5, "BALANCE"},
            {M::OSC_CTRL_PARAM_0 + 6, "STIFF"}};
}

template <> SingleConfig<ot_modern>::knobs_t SingleConfig<ot_modern>::getKnobs()
{
    typedef SurgeOSCSingle<ot_modern> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 0, "SAW"},
            {M::OSC_CTRL_PARAM_0 + 1, "PULSE"},
            {M::OSC_CTRL_PARAM_0 + 2, "MULTI"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "WIDTH"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

} // namespace sst::surgext_rack::vco

#endif // SURGEXT_RACK_SURGEOSCSINGLECONFIG_HPP
