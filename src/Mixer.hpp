//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_mixer_HPP
#define XTRACK_mixer_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::mixer
{
struct Mixer : modules::XTModule
{
    static constexpr int n_mixer_params{8};
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        OSC1_LEV,
        OSC2_LEV,
        OSC3_LEV,
        NOISE_LEV,
        RM1X2_LEV,
        RM2X3_LEV,
        NOISE_COL,
        GAIN,

        OSC1_MUTE, // important these orders are same as LEV
        OSC2_MUTE,
        OSC3_MUTE,
        NOISE_MUTE,
        RM1X2_MUTE,
        RM2x3_MUTE,

        OSC1_SOLO, // important these orders are same as LEV
        OSC2_SOLO,
        OSC3_SOLO,
        NOISE_SOLO,
        RM1X2_SOLO,
        RM2x3_SOLO,

        MIXER_MOD_PARAM_0,

        NUM_PARAMS = MIXER_MOD_PARAM_0 + n_mixer_params * n_mod_inputs,
    };
    enum InputIds
    {
        INPUT_OSC1_L,
        INPUT_OSC1_R,

        INPUT_OSC2_L,
        INPUT_OSC2_R,

        INPUT_OSC3_L,
        INPUT_OSC3_R,

        MIXER_MOD_INPUT,
        NUM_INPUTS = MIXER_MOD_INPUT + n_mod_inputs,

    };
    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    std::array<bool, 6> routes;

    Mixer() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Config
        for (int i = OSC1_LEV; i <= RM2X3_LEV; ++i)
        {
            configParam(i, 0, 1, i == OSC1_LEV ? 1 : 0);
        }
        configParam(NOISE_COL, -1, 1, 0);
        configParam(GAIN, 0, 1, 0);

        for (int i = OSC1_SOLO; i <= RM2x3_SOLO; ++i)
        {
            configParam(i, 0, 1, 0);
        }

        for (int i = OSC1_MUTE; i <= RM2x3_MUTE; ++i)
        {
            configParam(i, 0, 1, i == OSC1_MUTE ? 0 : 1);
        }

        for (int i = 0; i < n_mixer_params * n_mod_inputs; ++i)
        {
            configParam(MIXER_MOD_PARAM_0 + i, -1, 1, 0);
        }
    }

    void updateRoutes()
    {
        bool anySolo{false};
        for (int i = 0; i < 6; ++i)
            routes[i] = false;

        for (int i = OSC1_SOLO; i <= RM2x3_SOLO; ++i)
        {
            anySolo = anySolo || (params[i].getValue() > 0.5);
        }

        if (anySolo)
        {
            for (int i = OSC1_SOLO; i <= RM2x3_SOLO; ++i)
            {
                if (params[i].getValue() > 0.5)
                    routes[i - OSC1_SOLO] = true;
            }
        }
        else
        {
            for (int i = OSC1_MUTE; i <= RM2x3_MUTE; ++i)
            {
                if (params[i].getValue() < 0.5)
                    routes[i - OSC1_MUTE] = true;
            }
        }
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - OSC1_LEV;
        return MIXER_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }
    std::string getName() override { return "Mixer"; }
};
} // namespace sst::surgext_rack::mixer
#endif // RACK_HACK_mixer_HPP
