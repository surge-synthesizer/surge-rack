//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_LFO_HPP
#define XTRACK_LFO_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::lfo
{
struct LFO : modules::XTModule
{
    static constexpr int n_lfo_params{10};
    static constexpr int n_mod_inputs{5};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        RATE,
        PHASE,
        DEFORM,
        AMPLITUDE,

        E_DELAY,
        E_ATTACK,
        E_HOLD,
        E_DECAY,
        E_SUSTAIN,
        E_RELEASE,

        SHAPE,
        UNIPOLAR,

        LFO_MOD_PARAM_0,

        LFO_TYPE = LFO_MOD_PARAM_0 + n_lfo_params * n_mod_inputs,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_TRIGGER,
        INPUT_CLOCK_RATE,
        INPUT_CLOCK_ENV,
        INPUT_PHASE_DIRECT,
        LFO_MOD_INPUT,
        NUM_INPUTS = LFO_MOD_INPUT + n_mod_inputs,

    };
    enum OutputIds
    {
        OUTPUT_MIX,
        OUTPUT_ENV,
        OUTPUT_WAVE,
        OUTPUT_TRIGPHASE,
        OUTPUT_TRIGF,
        OUTPUT_TRIGA,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    LFO() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - RATE;
        return LFO_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    std::string getName() override { return "LFO"; }
};
} // namespace sst::surgext_rack::lfo
#endif // RACK_HACK_LFO_HPP
