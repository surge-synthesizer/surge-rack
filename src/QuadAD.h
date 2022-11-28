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
 * - DSP
 *    - Digital Slopes
 *  UI
 *    - Render
 *    - Pick digital slopes
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

        MOD_PARAM_0 = D_SHAPE_0 + n_ads,
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

    struct ADEnvelope
    {
        SurgeStorage *storage;
        ADEnvelope(SurgeStorage *s) : storage(s)
        {
            for (int i = 0; i < BLOCK_SIZE; ++i)
                outputCache[i] = 0;
        }
        enum Stage
        {
            s_attack,
            s_decay,
            s_complete
        } stage{s_complete};

        bool isDigital{true};

        float phase{0}, start{0};

        float output;
        float outputCache[BLOCK_SIZE], outBlock0{0.f};
        int current{BLOCK_SIZE};

        // Analog Mode
        float v_c1{0}, v_c1_delayed{0.f};
        bool discharge{false};

        void attackFrom(float f, bool id)
        {
            phase = 0;
            stage = s_attack;
            current = BLOCK_SIZE;
            isDigital = id;

            v_c1 = 0.f;
            v_c1_delayed = 0.f;
            discharge = false;
        }

        inline void process(const float a, const float d)
        {
            if (stage == s_complete)
            {
                output = 0;
                return;
            }
            if (current == BLOCK_SIZE)
            {
                float target = 0;
                if (isDigital)
                {
                    switch (stage)
                    {
                    case s_complete:
                        target = 0;
                        break;
                    case s_attack:
                    {
                        phase += storage->envelope_rate_linear(a);
                        if (phase >= 1)
                        {
                            phase = 1;
                            stage = s_decay;
                        }
                        target = phase;
                        break;
                    }
                    case s_decay:
                    {
                        phase -= storage->envelope_rate_linear(d);
                        if (phase <= 0)
                        {
                            phase = 0;
                            stage = s_complete;
                        }
                        target = phase;
                    }
                    }
                }
                else
                {
                    const float coeff_offset =
                        2.f - log(storage->samplerate / BLOCK_SIZE) / log(2.f);

                    discharge = (v_c1_delayed >= 0.99999f) || discharge;
                    v_c1_delayed = v_c1;

                    const float v_gate = 1.02f;
                    auto v_attack = discharge ? 0 : v_gate;
                    auto v_decay = discharge ? 0 : v_gate;

                    // In this case we only need the coefs in their stage
                    float coef_A = !discharge ? powf(2.f, std::min(0.f, coeff_offset - a)) : 0;
                    float coef_D = discharge ? powf(2.f, std::min(0.f, coeff_offset - d)) : 0;

                    auto diff_v_a = std::max(0.f, v_attack - v_c1);
                    auto diff_v_d = std::min(0.f, v_decay - v_c1);

                    v_c1 = v_c1 + diff_v_a * coef_A + diff_v_d * coef_D;
                    target = v_c1;
                    if (v_c1 < 1e-6 && discharge)
                    {
                        v_c1 = 0;
                        v_c1_delayed = 0;
                        discharge = false;
                        target = 0;
                        stage = s_complete;
                    }
                }
                float dO = (target - outBlock0) * BLOCK_SIZE_INV;
                for (int i = 0; i < BLOCK_SIZE; ++i)
                {
                    outputCache[i] = outBlock0 + dO * i;
                }
                outBlock0 = target;

                current = 0;
            }

            output = outputCache[current];
            current++;
        }
    };

    std::array<std::array<std::unique_ptr<ADEnvelope>, MAX_POLY>, n_ads> processors;

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
                processors[i][p] = std::make_unique<ADEnvelope>(storage.get());
            }
        }

        for (int i = 0; i < n_ads; ++i)
        {
            configParam<ADParamQuantity>(ATTACK_0 + i, -8, 2, -5, "Attack " + std::to_string(i));
            configParam<ADParamQuantity>(DECAY_0 + i, -8, 2, -5, "Decay " + std::to_string(i));
            configSwitch(MODE_0 + i, 0, 1, 0, "Mode", {"Digital", "Analog"});
            configSwitch(A_SHAPE_0 + i, 0, 2, 1, "Attack Curve", {"Slow", "Linear", "Fast"});
            configSwitch(D_SHAPE_0 + i, 0, 2, 1, "Decay Curve", {"Slow", "Linear", "Fast"});
        }

        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
            configParam(MOD_PARAM_0 + i, -1, 1, 0);

        modAssist.initialize(this);
        modAssist.setupMatrix(this);
        modAssist.updateValues(this);

        snapCalculatedNames();
    }

    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, true); // get those presets. FIXME skip wt later
    }

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

    int processCount{BLOCK_SIZE};
    rack::dsp::SchmittTrigger inputTriggers[n_ads][MAX_POLY];
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (processCount == BLOCK_SIZE)
        {
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);

            processCount = 0;
        }

        int tnc = 1;
        for (int i = 0; i < n_ads; ++i)
        {
            if (inputs[TRIGGER_0 + i].isConnected())
            {
                int ch = inputs[TRIGGER_0 + i].getChannels();
                tnc = std::max(tnc, ch);
                outputs[OUTPUT_0 + i].setChannels(ch);

                for (int c = 0; c < ch; ++c)
                {
                    if (inputTriggers[i][c].process(inputs[TRIGGER_0 + i].getVoltage(c)))
                    {
                        processors[i][c]->attackFrom(0.f, params[MODE_0 + i].getValue() < 0.5);
                    }
                    processors[i][c]->process(modAssist.values[ATTACK_0 + i][c],
                                              modAssist.values[DECAY_0 + i][c]);
                    outputs[OUTPUT_0 + i].setVoltage(processors[i][c]->output * 10, c);
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

    json_t *makeModuleSpecificJson() override { return nullptr; }

    void readModuleSpecificJson(json_t *modJ) override {}
};
} // namespace sst::surgext_rack::quadad
#endif
