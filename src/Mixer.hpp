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
#include "DSPUtils.h"

namespace sst::surgext_rack::mixer
{
struct Mixer : modules::XTModule
{
    static constexpr int n_mixer_params{8};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_osc{3};
    static constexpr int n_poly_quads{4};

    enum MixIdx
    {
        osc1 = 0,
        osc2,
        osc3,
        noise,
        r1x2,
        r2x3
    };

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
    std::array<bool, 3> needed;
    float noisegen[MAX_POLY][2][2];

    Mixer() : XTModule()
    {
        std::lock_guard<std::mutex> ltg(xtSurgeCreateMutex);

        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Config
        for (int i = OSC1_LEV; i <= RM2X3_LEV; ++i)
        {
            configParam(i, 0, 1, i == OSC1_LEV ? 1 : 0);
        }
        configParam(NOISE_COL, -1, 1, 0);
        configParam(GAIN, 0, 1, 1);

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

        for (int i = 0; i < MAX_POLY; ++i)
        {
            for (int c = 0; c < 2; ++c)
            {
                noisegen[i][c][0] = 0.f;
                noisegen[i][c][1] = 0.f;
            }
        }

        modulationAssistant.initialize(this);
    }

    modules::ModulationAssistant<Mixer, n_mixer_params, OSC1_LEV, n_mod_inputs, MIXER_MOD_INPUT>
        modulationAssistant;

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
        needed[osc1] = routes[osc1] || routes[r1x2];
        needed[osc2] = routes[osc2] || routes[r1x2] || routes[r2x3];
        needed[osc3] = routes[osc3] || routes[r2x3];
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - OSC1_LEV;
        return MIXER_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }
    std::string getName() override { return "Mixer"; }

    static constexpr int slowUpdate{8};
    int blockPos{slowUpdate};

    int polyDepth{1}, polyDepthBy4{1};
    int polyDepthPerOsc[n_osc];

    int polyChannelCount() { return polyDepth; }

    void process(const ProcessArgs &args) override
    {
        if (blockPos == slowUpdate)
        {
            updateRoutes();
            modulationAssistant.setupMatrix(this);
            blockPos = 0;

            for (int i = INPUT_OSC1_L; i <= INPUT_OSC3_L; i += 2)
            {
                auto pd = inputs[i].getChannels();
                polyDepth = std::max(polyDepth, pd);
                polyDepthPerOsc[(i - INPUT_OSC1_L) / 2] = pd;
            }
            polyDepthBy4 = (polyDepth - 1) / 4 + 1;
        }

        outputs[OUTPUT_L].setChannels(polyDepth);
        outputs[OUTPUT_R].setChannels(polyDepth);
        modulationAssistant.updateValues(this);

        rack::simd::float_4 oL[n_poly_quads]{0, 0, 0, 0};
        rack::simd::float_4 oR[n_poly_quads]{0, 0, 0, 0};

        rack::simd::float_4 osc[n_osc][2][n_poly_quads];
        for (int i = osc1; i <= osc3; ++i)
        {
            // We can be a bit more selective with which ones we skip
            // like if ring modulation is needed for instance
            if (!needed[i])
                continue;

            if (polyDepthPerOsc[i] > 1)
            {
                for (int p = 0; p < polyDepthBy4; ++p)
                {
                    osc[i][0][p] =
                        rack::simd::float_4::load(inputs[INPUT_OSC1_L + i * 2].getVoltages(p * 4)) *
                        RACK_TO_SURGE_OSC_MUL;
                    osc[i][1][p] =
                        rack::simd::float_4::load(inputs[INPUT_OSC1_R + i * 2].getVoltages(p * 4)) *
                        RACK_TO_SURGE_OSC_MUL;
                }
            }
            else
            {
                // this is basically loading up the mono signal across the braodcast set
                for (int p = 0; p < polyDepthBy4; ++p)
                {
                    osc[i][0][p] =
                        RACK_TO_SURGE_OSC_MUL * inputs[INPUT_OSC1_L + i * 2].getVoltage();
                    osc[i][1][p] =
                        RACK_TO_SURGE_OSC_MUL * inputs[INPUT_OSC1_R + i * 2].getVoltage();
                }
            }

            if (!routes[i])
                continue;

            for (int p = 0; p < polyDepthBy4; ++p)
            {
                oL[p] += osc[i][0][p] * modulationAssistant.valuesSSE[OSC1_LEV + i][p];
                oR[p] += osc[i][1][p] * modulationAssistant.valuesSSE[OSC1_LEV + i][p];
            }
        }

        if (routes[noise])
        {
            for (int p = 0; p < polyDepth; ++p)
            {
                auto col = std::clamp(modulationAssistant.values[NOISE_COL][p], -1.f, 1.f);
                oL[p >> 2][p % 4] += correlated_noise_o2mk2_storagerng(
                                         noisegen[p][0][0], noisegen[p][0][1], col, storage.get()) *
                                     modulationAssistant.values[NOISE_LEV][p];
                oR[p >> 2][p % 4] += correlated_noise_o2mk2_storagerng(
                                         noisegen[p][1][0], noisegen[p][1][1], col, storage.get()) *
                                     modulationAssistant.values[NOISE_LEV][p];
            }
        }

        if (routes[r1x2])
        {
            for (int p = 0; p < polyDepthBy4; ++p)
            {
                oL[p] +=
                    osc[osc1][0][p] * osc[osc2][0][p] * modulationAssistant.valuesSSE[RM1X2_LEV][p];
                oR[p] +=
                    osc[osc1][1][p] * osc[osc2][1][p] * modulationAssistant.valuesSSE[RM1X2_LEV][p];
            }
        }

        if (routes[r2x3])
        {
            for (int p = 0; p < polyDepthBy4; ++p)
            {
                oL[p] +=
                    osc[osc3][0][p] * osc[osc2][0][p] * modulationAssistant.valuesSSE[RM2X3_LEV][p];
                oR[p] +=
                    osc[osc3][1][p] * osc[osc2][1][p] * modulationAssistant.valuesSSE[RM2X3_LEV][p];
            }
        }

        for (int p = 0; p < polyDepthBy4; ++p)
        {
            oL[p] *= modulationAssistant.valuesSSE[GAIN][p];
            oR[p] *= modulationAssistant.valuesSSE[GAIN][p];
            oL[p] *= SURGE_TO_RACK_OSC_MUL;
            oR[p] *= SURGE_TO_RACK_OSC_MUL;

            oL[p].store(outputs[OUTPUT_L].getVoltages(p * 4));
            oR[p].store(outputs[OUTPUT_R].getVoltages(p * 4));
        }
        blockPos++;
    }
};
} // namespace sst::surgext_rack::mixer
#endif // RACK_HACK_mixer_HPP
