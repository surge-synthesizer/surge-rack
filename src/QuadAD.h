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

#ifndef SURGE_XT_RACK_SRC_QUADAD_H
#define SURGE_XT_RACK_SRC_QUADAD_H

#include "SurgeXT.h"
#include "dsp/Effect.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"
#include "FxPresetAndClipboardManager.h"

#include "LayoutEngine.h"

#include "sst/basic-blocks/modulators/ADAREnvelope.h"

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

    typedef basic_blocks::modulators::ADAREnvelope<SurgeStorage, BLOCK_SIZE> envelope_t;
    std::array<std::array<std::unique_ptr<envelope_t>, MAX_POLY>, n_ads> processors;

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
                processors[i][p] = std::make_unique<envelope_t>(storage.get());
                accumulatedOutputs[i][p] = 0.f;
                gated[i][p] = false;
                eocFromAway[i][p] = 0;
            }
            pushEOCOnto[i] = -1;
            isTriggerLinked[i] = false;
            isEnvLinked[i] = false;
            adPoly[i] = 1;
            coTriggersWith[i] = 0;
        }

        for (int i = 0; i < n_ads; ++i)
        {
            auto maxv = log2(10.0);
            configParam<ADParamQuantity>(ATTACK_0 + i, -8, maxv, -5, "Attack " + std::to_string(i));
            configParam<ADParamQuantity>(DECAY_0 + i, -8, maxv, -5, "Decay " + std::to_string(i));
            configSwitch(MODE_0 + i, 0, 1, 0, "Mode", {"Digital", "Analog"});
            configSwitch(A_SHAPE_0 + i, 0, 2, 1, "Attack Curve", {"Faster", "Standard", "Slower"});
            configSwitch(D_SHAPE_0 + i, 0, 2, 1, "Decay Curve", {"Faster", "Standard", "Slower"});
            configSwitch(ADAR_0 + i, 0, 1, 0, "AD vs AR", {"AD", "AR"});
            configSwitch(LINK_TRIGGER_0 + i, -1, 1, 0,
                         "Link " + std::to_string(i + 1) + " EOC to " +
                             std::to_string((i + 1) % n_ads + 1) + " Attack",
                         {"Cycle", "Off", "On"});
            configSwitch(LINK_ENV_0 + i, 0, 1, 0,
                         "Sum " + std::to_string(i + 1) + " ENV to " +
                             std::to_string((i + 1) % n_ads + 1) + " Output",
                         {"Off", "On"});
        }

        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
        {
            int modi = i % n_mod_inputs;
            int pari = i / n_mod_inputs;
            std::string tgt = "Attack";
            if (pari >= DECAY_0)
            {
                tgt = "Decay";
                pari -= n_ads;
            }
            auto name = fmt::format("Mod {} to {} {}", modi + 1, tgt, pari + 1);
            configParam(MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
        }

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

    void setupSurge() { setupSurgeCommon(NUM_PARAMS, false, false); }

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
    rack::dsp::SchmittTrigger inputTriggers[n_ads][MAX_POLY], linkTriggers[n_ads][MAX_POLY];
    bool gated[n_ads][MAX_POLY];
    float accumulatedOutputs[n_ads][MAX_POLY];

    bool isEnvLinked[n_ads];
    int adPoly[n_ads];

    bool isTriggerLinked[n_ads]; // this target is linked from away
    int pushEOCOnto[n_ads];      // this target pushes to that source
    bool anyLinked{false};
    float eocFromAway[n_ads][MAX_POLY];
    int lastTriggerLinkParaams[n_ads]{-1, -1, -1, -1};
    int coTriggersWith[n_ads]; // a bitmask of people i co-trigger with

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (processCount == BLOCK_SIZE)
        {
            processCount = 0;

            for (int i = 0; i < n_ads; ++i)
            {
                int envLinkIdx = (i + n_ads - 1) & (n_ads - 1);
                isEnvLinked[i] = params[LINK_ENV_0 + envLinkIdx].getValue() > 0.5;
            }

            // Only do this if the triggers have changed
            bool recalc{false};
            for (int i = 0; i < n_ads; ++i)
            {
                auto tn = (int)std::round(params[LINK_TRIGGER_0 + i].getValue());
                if (tn != lastTriggerLinkParaams[i])
                    recalc = true;
                lastTriggerLinkParaams[i] = tn;
            }
            if (recalc)
            {
                // burn it down and start again
                for (int i = 0; i < n_ads; ++i)
                {
                    pushEOCOnto[i] = -1;
                    isTriggerLinked[i] = false;
                    anyLinked = false;
                    coTriggersWith[i] = 0;
                }
                for (int i = 0; i < n_ads; ++i)
                {
                    auto tn = (int)std::round(params[LINK_TRIGGER_0 + i].getValue());
                    anyLinked = anyLinked || (tn != 0);
                    if (tn == 1)
                    {
                        pushEOCOnto[i] = (i + 1) & (n_ads - 1);
                        isTriggerLinked[pushEOCOnto[i]] = true;
                    }
                    if (tn == -1)
                    {
                        // Single cycle for now
                        int q = (i - 1 + n_ads) & (n_ads - 1);
                        int loopFrom{i};
                        while (q != i)
                        {
                            auto qn = (int)std::round(params[LINK_TRIGGER_0 + q].getValue());
                            if (qn != 1)
                                break;
                            loopFrom = q;
                            q = (q - 1 + n_ads) & (n_ads - 1);
                        }
                        pushEOCOnto[i] = loopFrom;
                        isTriggerLinked[i] = true;
                        isTriggerLinked[loopFrom] = true;
                    }
                }

                // I hate that this calculation is so convluted but I don't see a clever way
                // If you do, we love PRs! Anyway this makes the bitmask of people in a cycle
                for (int i = 0; i < n_ads; ++i)
                {
                    int iPushOnto{0};
                    for (int j = 0; j < n_ads; ++j)
                    {
                        auto pos = (i + j) & (n_ads - 1);
                        if (pushEOCOnto[pos] != (pos + 1) % n_ads)
                            break;
                        iPushOnto |= 1 << pushEOCOnto[pos];
                    }
                    int pushOntoMe{0};
                    for (int j = 1; j < n_ads; ++j)
                    {
                        auto pos = i - j;
                        if (pos < 0)
                            pos += n_ads;
                        if (pushEOCOnto[pos] != (pos + 1) % n_ads)
                            break;
                        pushOntoMe |= 1 << pos;
                    }

                    auto res = pushOntoMe | iPushOnto;
                    if (res)
                        res |= 1 << i;

                    coTriggersWith[i] = res;
                }
            }
            // TRIGGER POLY TODO
            nChan = 1;
            for (int i = 0; i < n_ads; ++i)
            {
                adPoly[i] =
                    inputs[TRIGGER_0 + i].isConnected() ? inputs[TRIGGER_0 + i].getChannels() : 1;
                nChan = std::max(nChan, adPoly[i]);
            }
            if (anyLinked)
            {
                for (int i = 0; i < n_ads; ++i)
                {
                    adPoly[i] = nChan;
                }
            }

            memset(eocFromAway, 0, n_ads * MAX_POLY * sizeof(float));
            for (int i = 0; i < n_ads; ++i)
            {
                if (pushEOCOnto[i] >= 0)
                {
                    for (int c = 0; c < MAX_POLY; ++c)
                    {
                        eocFromAway[pushEOCOnto[i]][c] += processors[i][c]->eoc_output;
                    }
                }
            }

            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
        }

        int tnc = 1;
        for (int i = 0; i < n_ads; ++i)
        {
            // who is my env neighbor?
            int envLinkIdx = (i + n_ads - 1) & (n_ads - 1);

            if (inputs[TRIGGER_0 + i].isConnected() || isTriggerLinked[i] || isEnvLinked[i] ||
                pushEOCOnto[i] >= 0)
            {
                int ch = adPoly[i];
                outputs[OUTPUT_0 + i].setChannels(ch);
                auto as = params[A_SHAPE_0 + i].getValue();
                auto ds = params[D_SHAPE_0 + i].getValue();
                for (int c = 0; c < ch; ++c)
                {
                    auto iv = inputs[TRIGGER_0 + i].getVoltage(c);
                    auto lv = (isTriggerLinked[i] && (eocFromAway[i][c] > 0)) ? 10.f : 0.f;

                    auto extTrig = inputTriggers[i][c].process(iv);
                    auto linkTrig = linkTriggers[i][c].process(lv);
                    float maxNukeValue{0.f};
                    if (extTrig)
                    {
                        auto nukeThese = coTriggersWith[i];
                        for (int k = 0; k < n_ads; ++k)
                        {
                            if (i != k && (nukeThese & (1 << k)))
                            {
                                maxNukeValue = std::max(maxNukeValue, processors[k][c]->output);
                                processors[k][c]->immediatelySilence();
                            }
                        }
                    }
                    if (extTrig || linkTrig)
                    {
                        gated[i][c] = true;
                        auto af =
                            attackFromZero ? 0 : std::max(processors[i][c]->output, maxNukeValue);
                        processors[i][c]->attackFrom(af, as, params[MODE_0 + i].getValue() < 0.5,
                                                     params[ADAR_0 + i].getValue() > 0.5);
                    }

                    if (gated[i][c] && iv < 1.f) // that's the default trigger threshold
                    {
                        gated[i][c] = false;
                    }

                    processors[i][c]->processScaledAD(modAssist.values[ATTACK_0 + i][c],
                                                      modAssist.values[DECAY_0 + i][c], as, ds,
                                                      gated[i][c]);

                    auto ov = processors[i][c]->output * 10;
                    if (isEnvLinked[i])
                        ov += accumulatedOutputs[envLinkIdx][c];

                    outputs[OUTPUT_0 + i].setVoltage(ov, c);
                    accumulatedOutputs[i][c] = ov;
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
        json_object_set_new(qv, "attackFromZero", json_boolean(attackFromZero));
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
