/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_SRC_MIXER_H
#define SURGE_XT_RACK_SRC_MIXER_H

#include "SurgeXT.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"
#include "DSPUtils.h"

#include "sst/basic-blocks/dsp/CorrelatedNoise.h"
#include "CXOR.h"
#include "sst/rackhelpers/neighbor_connectable.h"

namespace sst::surgext_rack::mixer
{
struct Mixer : modules::XTModule, sst::rackhelpers::module_connector::NeighborConnectable_V1
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
    std::array<bool, 3> everConnected;
    float noisegen[MAX_POLY][2][2];

    Mixer() : XTModule()
    {
        std::lock_guard<std::mutex> ltg(xtSurgeCreateMutex);

        setupSurgeCommon(NUM_PARAMS, false, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Config
        for (int i = OSC1_LEV; i <= RM2X3_LEV; ++i)
        {
            std::string name = inputName(i) + " Level";
            configParam<modules::DecibelParamQuantity>(i, 0, 1, i == OSC1_LEV ? 1 : 0, name);
        }
        configParam(NOISE_COL, -1, 1, 0, "Noise Color", "%", 0, 100);
        configParam<modules::DecibelParamQuantity>(GAIN, 0, 1, 1, "Gain");

        for (int i = OSC1_SOLO; i <= RM2x3_SOLO; ++i)
        {
            std::string name = inputName(i - OSC1_SOLO + OSC1_LEV) + " Solo";
            configSwitch(i, 0, 1, 0, name, {"Off", "On"});
        }

        for (int i = OSC1_MUTE; i <= RM2x3_MUTE; ++i)
        {
            std::string name = inputName(i - OSC1_MUTE + OSC1_LEV) + " Mute";
            configSwitch(i, 0, 1, i == OSC1_MUTE ? 0 : 1, name, {"Off", "On"});
        }

        for (int i = 0; i < n_mixer_params * n_mod_inputs; ++i)
        {
            auto name = std::string("Mod ") + std::to_string(i % 4 + 1);

            configParamNoRand<modules::DecibelModulatorParamQuantity<Mixer>>(MIXER_MOD_PARAM_0 + i,
                                                                             -1, 1, 0, name);
        }

        for (int i = 0; i < n_mod_inputs; ++i)
        {
            configInput(MIXER_MOD_INPUT + i, std::string("Modulator ") + std::to_string(i + 1));
        }

        for (int i = INPUT_OSC1_L; i <= INPUT_OSC3_L; i += 2)
        {
            std::string nbase = "Input " + std::to_string(i / 2 + 1);
            configInput(i, nbase + " Left");
            configInput(i + 1, nbase + " Right");
        }

        configOutput(OUTPUT_L, "Left");
        configOutput(OUTPUT_R, "Right");
        for (int i = 0; i < MAX_POLY; ++i)
        {
            for (int c = 0; c < 2; ++c)
            {
                noisegen[i][c][0] = 0.f;
                noisegen[i][c][1] = 0.f;
            }
        }

        modulationAssistant.initialize(this);

        vuLevel[0] = 0.f;
        vuLevel[1] = 0.f;

        for (int i = 0; i < 3; ++i)
            everConnected[i] = false;
    }

    modules::ModulationAssistant<Mixer, n_mixer_params, OSC1_LEV, n_mod_inputs, MIXER_MOD_INPUT>
        modulationAssistant;

    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - MIXER_MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_mixer_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs + OSC1_LEV;
    }

    void updateRoutes()
    {
        bool anySolo{false};
        for (int i = 0; i < 6; ++i)
            routes[i] = false;

        for (int i = 0; i < 3; ++i)
        {
            if (!everConnected[i] && (inputs[INPUT_OSC1_L + 2 * i].isConnected() ||
                                      inputs[INPUT_OSC1_R + 2 * i].isConnected()))
            {
                everConnected[i] = true;
                params[OSC1_MUTE + i].setValue(0);
            }
        }

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

    std::string inputName(int p)
    {
        switch (p)
        {
        case OSC1_LEV:
            return "Input 1";
        case OSC2_LEV:
            return "Input 2";
        case OSC3_LEV:
            return "Input 3";
        case RM1X2_LEV:
            return "RingMod 1X2";
        case RM2X3_LEV:
            return "RingMod 2x3";
        case NOISE_LEV:
            return "Noise";
        }

        return "ERROR";
    }

    float modulationDisplayValue(int paramId) override
    {
        return modulationAssistant.animValues[paramId - OSC1_LEV];
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - OSC1_LEV;
        return MIXER_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    bool isBipolar(int paramId) override
    {
        if (paramId == NOISE_COL)
            return true;
        return false;
    }
    std::string getName() override { return "Mixer"; }

    static constexpr int slowUpdate{8};
    int blockPos{slowUpdate};

    int polyDepth{1}, polyDepthBy4{1};
    int polyDepthPerOsc[n_osc];
    int rightInputFrom[n_osc];

    int polyChannelCount() { return polyDepth; }

    float vuLevel[2];
    float vuFalloff{0.999};
    std::atomic<int> vuChannel{0};

    void moduleSpecificSampleRateChange() override
    {
        vuFalloff = exp(-2.0 * M_PI * 8 / APP->engine->getSampleRate());
    }

    void process(const ProcessArgs &args) override
    {
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        if (blockPos == slowUpdate)
        {
            updateRoutes();
            modulationAssistant.setupMatrix(this);
            blockPos = 0;

            polyDepth = 1;
            for (int i = INPUT_OSC1_L; i <= INPUT_OSC3_L; i += 2)
            {
                auto pd = inputs[i].getChannels();
                polyDepth = std::max(polyDepth, pd);
                polyDepthPerOsc[(i - INPUT_OSC1_L) / 2] = pd;
            }
            for (int i = 0; i < n_osc; ++i)
            {
                if (inputs[INPUT_OSC1_R + i * 2].isConnected())
                {
                    rightInputFrom[i] = INPUT_OSC1_R;
                }
                else
                {
                    rightInputFrom[i] = INPUT_OSC1_L;
                }
            }
            polyDepthBy4 = (polyDepth - 1) / 4 + 1;

            if (vuChannel > polyChannelCount())
                vuChannel = 0;
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
                    osc[i][1][p] = rack::simd::float_4::load(
                                       inputs[rightInputFrom[i] + i * 2].getVoltages(p * 4)) *
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
                        RACK_TO_SURGE_OSC_MUL * inputs[rightInputFrom[i] + i * 2].getVoltage();
                }
            }

            if (!routes[i])
                continue;

            for (int p = 0; p < polyDepthBy4; ++p)
            {
                oL[p] += osc[i][0][p] * modules::DecibelParamQuantity::ampToLinearSSE(
                                            modulationAssistant.valuesSSE[OSC1_LEV + i][p]);
                oR[p] += osc[i][1][p] * modules::DecibelParamQuantity::ampToLinearSSE(
                                            modulationAssistant.valuesSSE[OSC1_LEV + i][p]);
            }
        }

        if (routes[noise])
        {
            for (int p = 0; p < polyDepth; ++p)
            {
                auto col = std::clamp(modulationAssistant.values[NOISE_COL][p], -1.f, 1.f);
                oL[p >> 2][p % 4] +=
                    sst::basic_blocks::dsp::correlated_noise_o2mk2_supplied_value(
                        noisegen[p][0][0], noisegen[p][0][1], col, storage->rand_pm1()) *
                    modules::DecibelParamQuantity::ampToLinear(
                        modulationAssistant.values[NOISE_LEV][p]);
                oR[p >> 2][p % 4] +=
                    sst::basic_blocks::dsp::correlated_noise_o2mk2_supplied_value(
                        noisegen[p][1][0], noisegen[p][1][1], col, storage->rand_pm1()) *
                    modules::DecibelParamQuantity::ampToLinear(
                        modulationAssistant.values[NOISE_LEV][p]);
            }
        }

        if (routes[r1x2])
        {
            for (int p = 0; p < polyDepthBy4; ++p)
            {
                oL[p] += osc[osc1][0][p] * osc[osc2][0][p] *
                         modules::DecibelParamQuantity::ampToLinearSSE(
                             modulationAssistant.valuesSSE[RM1X2_LEV][p]);
                oR[p] += osc[osc1][1][p] * osc[osc2][1][p] *
                         modules::DecibelParamQuantity::ampToLinearSSE(
                             modulationAssistant.valuesSSE[RM1X2_LEV][p]);
            }
        }

        if (routes[r2x3])
        {
            for (int p = 0; p < polyDepthBy4; ++p)
            {
                oL[p] += osc[osc3][0][p] * osc[osc2][0][p] *
                         modules::DecibelParamQuantity::ampToLinearSSE(
                             modulationAssistant.valuesSSE[RM2X3_LEV][p]);
                oR[p] += osc[osc3][1][p] * osc[osc2][1][p] *
                         modules::DecibelParamQuantity::ampToLinearSSE(
                             modulationAssistant.valuesSSE[RM2X3_LEV][p]);
            }
        }

        for (int p = 0; p < polyDepthBy4; ++p)
        {
            oL[p] *= modules::DecibelParamQuantity::ampToLinearSSE(
                modulationAssistant.valuesSSE[GAIN][p]);
            oR[p] *= modules::DecibelParamQuantity::ampToLinearSSE(
                modulationAssistant.valuesSSE[GAIN][p]);
            oL[p] *= SURGE_TO_RACK_OSC_MUL;
            oR[p] *= SURGE_TO_RACK_OSC_MUL;

            oL[p].store(outputs[OUTPUT_L].getVoltages(p * 4));
            oR[p].store(outputs[OUTPUT_R].getVoltages(p * 4));
        }
        for (int i = 0; i < 2; ++i)
        {
            vuLevel[i] = std::min(10.f, vuFalloff * vuLevel[i]);
            vuLevel[i] = std::max(vuLevel[i], outputs[OUTPUT_L + i].getVoltage(vuChannel));
        }

        blockPos++;
    }

    json_t *makeModuleSpecificJson() override
    {
        auto vco = json_object();
        json_object_set_new(vco, "vuChannel", json_integer(vuChannel));
        return vco;
    }
    void readModuleSpecificJson(json_t *modJ) override
    {
        auto v = json_object_get(modJ, "vuChannel");
        if (!v)
            vuChannel = 0;
        else
            vuChannel = json_integer_value(v);

        // If we unstream from json let the json win and turn of the
        // auto-unmute-on-first-connect feature
        for (auto &ev : everConnected)
            ev = true;
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        return {{std::make_pair("Input 1", std::make_pair(INPUT_OSC1_L, INPUT_OSC1_R)),
                 std::make_pair("Input 2", std::make_pair(INPUT_OSC2_L, INPUT_OSC2_R)),
                 std::make_pair("Input 3", std::make_pair(INPUT_OSC3_L, INPUT_OSC3_R))}};
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{std::make_pair("Output", std::make_pair(OUTPUT_L, OUTPUT_R))}};
    }
};
} // namespace sst::surgext_rack::mixer
#endif
