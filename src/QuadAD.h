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

/*
 * ToDos
 *
 *  DSP
 *     - Maybe a bit more profiling?
 *  Rack
 *     - Mod Param Param Quanities - display, typein, etc...
 *  UI
 *    - Slopes Glpyohs etc on the A=/A- etc...
 *    - Curves on the display
 *    - Layout D/A and Trig/Hold controls
 *    - Remove Alpha Label
 *
 */

#ifndef SURGE_XT_RACK_QUADADHPP
#define SURGE_XT_RACK_QUADADHPP

#include "SurgeXT.h"
#include "dsp/Effect.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"
#include "FxPresetAndClipboardManager.h"

#include "LayoutEngine.h"
#include "ADSRModulationSource.h"
#include "dsp/ADAREnvelope.h"

namespace sst::surgext_rack::quadad
{
struct QuadAD : modules::XTModule
{
    static constexpr int n_ads{4};
    static constexpr int n_mod_params{n_ads * 2};
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        // Important modulation targets are contiguous and first
        ATTACK_0,
        DECAY_0 = ATTACK_0 + n_ads,

        MODE_0 = DECAY_0 + n_ads,
        A_SHAPE_0 = MODE_0 + n_ads,
        D_SHAPE_0 = A_SHAPE_0 + n_ads,
        ADAR_0 = D_SHAPE_0 + n_ads,
        LINK_TRIGGER_0 = ADAR_0 + n_ads,
        LINK_ENV_0 = LINK_TRIGGER_0 + n_ads,
        MOD_PARAM_0 = LINK_ENV_0 + n_ads,
        NUM_PARAMS = MOD_PARAM_0 + n_mod_params * n_mod_inputs
    };

    enum InputIds
    {
        TRIGGER_0,

        MOD_INPUT_0 = TRIGGER_0 + n_ads,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs,
    };

    enum OutputIds
    {
        OUTPUT_0,
        // EOC_0 = OUTPUT_0 + n_ads,
        NUM_OUTPUTS = OUTPUT_0 + n_ads
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::ModulationAssistant<QuadAD, n_mod_params, ATTACK_0, n_mod_inputs, MOD_INPUT_0>
        modAssist;

    struct ADParamQuantity : rack::ParamQuantity
    {
        std::string getDisplayValueString() override
        {
            auto v = getValue();
            auto time = pow(2.0, v);
            if (time < 1)
                return fmt::format("{:6.1f}ms", time * 1000);
            else
                return fmt::format("{:6.3f}s", time);
        }
        void setDisplayValueString(std::string s) override
        {
            auto v = std::atof(s.c_str());

            auto fac = 1;
            if (s.find("ms") != std::string::npos || v > 8)
                fac = 1000;

            if (v <= 0)
            {
                setValue(getMinValue());
                return;
            }
            float t = log2(v / fac);
            setValue(std::clamp(t, getMinValue(), getMaxValue()));
        }
    };

    std::array<std::array<std::unique_ptr<dsp::envelopes::ADAREnvelope>, MAX_POLY>, n_ads>
        processors;

    QuadAD() : XTModule()
    {
        {
            std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
            setupSurge();
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < n_ads; ++i)
        {
            for (int p = 0; p < MAX_POLY; ++p)
            {
                processors[i][p] = std::make_unique<dsp::envelopes::ADAREnvelope>(storage.get());
                accumulatedOutputs[i][p] = 0.f;
            }
            isTriggerLinked[i] = false;
            isEnvLinked[i] = false;
            adPoly[i] = 1;
        }

        for (int i = 0; i < n_ads; ++i)
        {
            configParam<ADParamQuantity>(ATTACK_0 + i, -8, 2, -5, "Attack " + std::to_string(i));
            configParam<ADParamQuantity>(DECAY_0 + i, -8, 2, -5, "Decay " + std::to_string(i));
            configSwitch(MODE_0 + i, 0, 1, 0, "Mode", {"Digital", "Analog"});
            configSwitch(A_SHAPE_0 + i, 0, 2, 1, "Attack Curve", {"A <", "A -", "A >"});
            configSwitch(D_SHAPE_0 + i, 0, 2, 1, "Decay Curve", {"D <", "D -", "D >"});
            configSwitch(ADAR_0 + i, 0, 1, 0, "AD vs AR", {"AD Trig", "AR Gate"});
            configSwitch(LINK_TRIGGER_0 + i, 0, 1, 0,
                         "Link " + std::to_string(i + 1) + " EOC to " +
                             std::to_string((i + 1) % n_ads + 1) + " Attack",
                         {"Off", "On"});
            configSwitch(LINK_ENV_0 + i, 0, 1, 0,
                         "Sum " + std::to_string(i + 1) + " ENV to " +
                             std::to_string((i + 1) % n_ads + 1) + " Output",
                         {"Off", "On"});
        }

        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
            configParam(MOD_PARAM_0 + i, -1, 1, 0);

        for (int i = 0; i < n_ads; ++i)
        {
            configInput(TRIGGER_0 + i, "Trigger/Gate " + std::to_string(i));
            configOutput(OUTPUT_0 + i, "ENV " + std::to_string(i));
            configBypass(TRIGGER_0 + i, OUTPUT_0 + i);
        }

        for (int i = 0; i < n_mod_inputs; ++i)
            configInput(MOD_INPUT_0 + i, "Mod " + std::to_string(i));

        modAssist.initialize(this);
        modAssist.setupMatrix(this);
        modAssist.updateValues(this);

        snapCalculatedNames();
    }

    void setupSurge() { setupSurgeCommon(NUM_PARAMS, false); }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        return nullptr;
    }

    int polyChannelCount() { return nChan; }
    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - ATTACK_0;
        if (offset >= n_mod_inputs * (n_mod_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs;
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - ATTACK_0;
        return MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - ATTACK_0;
        if (idx < 0 || idx >= n_mod_params)
            return 0;

        return modAssist.modvalues[idx][0];
    }

    bool isBipolar(int paramId) override { return false; }

    void moduleSpecificSampleRateChange() override {}
    std::string getName() override { return {"QuadAD"}; }

    int nChan{-1};

    std::atomic<bool> attackFromZero{false};
    int processCount{BLOCK_SIZE};
    rack::dsp::SchmittTrigger inputTriggers[n_ads][MAX_POLY];
    float accumulatedOutputs[n_ads][MAX_POLY];

    bool isTriggerLinked[n_ads], isEnvLinked[n_ads];
    int adPoly[n_ads];

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (processCount == BLOCK_SIZE)
        {
            processCount = 0;

            for (int i = 0; i < n_ads; ++i)
            {
                int linkIdx = (i + n_ads - 1) & (n_ads - 1);
                isTriggerLinked[i] = params[LINK_TRIGGER_0 + linkIdx].getValue() > 0.5;
                isEnvLinked[i] = params[LINK_ENV_0 + linkIdx].getValue() > 0.5;
            }

            nChan = 1;
            for (int i = 0; i < n_ads; ++i)
            {
                if (isTriggerLinked[i])
                {
                    int chl = inputs[TRIGGER_0 + i].getChannels();
                    int j = (i + n_ads - 1) & (n_ads - 1);
                    while (j != i)
                    {
                        if (!isTriggerLinked[j])
                        {
                            chl = std::max(chl, inputs[TRIGGER_0 + j].getChannels());
                            break;
                        }
                        j = (j + n_ads - 1) & (n_ads - 1);
                    }
                    adPoly[i] = std::max(1, chl);
                }
                else
                {
                    adPoly[i] = inputs[TRIGGER_0 + i].isConnected()
                                    ? inputs[TRIGGER_0 + i].getChannels()
                                    : 1;
                }
                nChan = std::max(nChan, adPoly[i]);
            }

            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
        }

        int tnc = 1;
        for (int i = 0; i < n_ads; ++i)
        {
            // who is my trigger neighbor?
            int linkIdx = (i + n_ads - 1) & (n_ads - 1);
            float linkMul = isTriggerLinked[i] * 10.f;

            if (inputs[TRIGGER_0 + i].isConnected() || isTriggerLinked[i] || isEnvLinked[i])
            {
                int ch = adPoly[i];
                outputs[OUTPUT_0 + i].setChannels(ch);
                auto as = params[A_SHAPE_0 + i].getValue();
                auto ds = params[D_SHAPE_0 + i].getValue();
                for (int c = 0; c < ch; ++c)
                {
                    auto iv = inputs[TRIGGER_0 + i].getVoltage(c);
                    auto lv = processors[linkIdx][c]->eoc_output * linkMul;

                    if (inputTriggers[i][c].process(iv + lv))
                    {
                        processors[i][c]->attackFrom(attackFromZero ? 0 : processors[i][c]->output,
                                                     as, params[MODE_0 + i].getValue() < 0.5,
                                                     params[ADAR_0 + i].getValue() > 0.5);
                    }
                    processors[i][c]->process(modAssist.values[ATTACK_0 + i][c],
                                              modAssist.values[DECAY_0 + i][c], as, ds,
                                              (lv + iv) * 0.1);

                    auto ov = processors[i][c]->output * 10;
                    if (isEnvLinked[i])
                        ov += accumulatedOutputs[linkIdx][c];

                    outputs[OUTPUT_0 + i].setVoltage(ov, c);
                    accumulatedOutputs[i][c] = ov;
                    // outputs[OUTPUT_0 + i].setVoltage(
                    //     (processors[i][c]->output + processors[i][c]->eoc_output) * 10, c);
                }
            }
            else
            {
                outputs[OUTPUT_0 + i].setChannels(1);
                outputs[OUTPUT_0 + i].setVoltage(0);
            }
        }
        nChan = tnc;

        processCount++;
    }

    json_t *makeModuleSpecificJson() override
    {
        auto qv = json_object();
        json_object_set(qv, "attackFromZero", json_boolean(attackFromZero));
        return qv;
    }

    void readModuleSpecificJson(json_t *modJ) override
    {
        {
            auto az = json_object_get(modJ, "attackFromZero");
            if (az)
            {
                attackFromZero = json_boolean_value(az);
            }
        }
    }
};
} // namespace sst::surgext_rack::quadad
#endif
