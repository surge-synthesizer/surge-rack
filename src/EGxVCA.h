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
 * EGxVCA ToDos
 *
 * - Clock and TempoSynch
 * - Display area
 *    - Analog/Digital control as click-to-switch menu
 *    - Draw the waveform
 *    - Mode (DAHD vs ADSR) selector
 *    - Digital Shape Controls
 *    - Draw the meters
 * - DSP
 *    - Retrigger / Gate controls
 *    - DAHD mode v DASDR mode
 *    - Pan implementation (follow models from MixMaster probably)
 * - UI
 *    - LintBuddy/Label inputs outputs mods etc
 *    - Correct labels and param quantities for the modulator knobs
 */

#ifndef SURGE_XT_RACK_EGVCAHPP
#define SURGE_XT_RACK_EGVCAHPP

#include "SurgeXT.h"
#include "dsp/Effect.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"
#include "FxPresetAndClipboardManager.h"

#include "LayoutEngine.h"
#include "ADSRModulationSource.h"

namespace sst::surgext_rack::egxvca
{
struct EGxVCA : modules::XTModule
{
    static constexpr int n_mod_params{7};
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        LEVEL,
        PAN,
        RESPONSE,
        EG_A, // these morph based on mode
        EG_D,
        EG_S,
        EG_R,
        ANALOG_OR_DIGITAL,
        A_SHAPE,
        D_SHAPE,
        R_SHAPE,
        ENV_MODE,

        MOD_PARAM_0,
        NUM_PARAMS = MOD_PARAM_0 + n_mod_params * n_mod_inputs
    };

    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        GATE_IN,
        RETRIG_IN,
        CLOCK_IN,
        MOD_INPUT_0,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs,
    };

    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        ENV_OUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::ModulationAssistant<EGxVCA, n_mod_params, LEVEL, n_mod_inputs, MOD_INPUT_0> modAssist;

    EGxVCA() : XTModule()
    {
        {
            std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
            setupSurge();
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<modules::DecibelParamQuantity>(LEVEL, 0, 2, 1, "Level");
        configParam(PAN, -1, 1, 0, "Pan", "%", 0, 100);
        configParam<modules::SurgeParameterParamQuantity>(EG_A, 0, 1, 0.1, "Attack");
        configParam<modules::SurgeParameterParamQuantity>(EG_D, 0, 1, 0.1, "Decay");
        configParam<modules::SurgeParameterParamQuantity>(EG_S, 0, 1, 0.5, "Sustain");
        configParam<modules::SurgeParameterParamQuantity>(EG_R, 0, 1, 0.1, "Release");
        configSwitch(ANALOG_OR_DIGITAL, 0, 1, 0, "Curve", {"Digital", "Analog"});

        configParam(RESPONSE, 0, 1, 0, "Linear/Exponential");

        // really need to configParam those mod params for this to work
        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
        {
            std::string name = std::string("Mod ") + std::to_string(i % 4 + 1);

            configParamNoRand(MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);

#if 0
            int tp = paramModulatedBy(i + MOD_PARAM_0);

            auto lb = paramQuantities[tp]->getLabel();

            if (tp == LEVEL || tp == PAN)
            {
                std::string name = std::string("Mod ") + std::to_string(i % 4 + 1) + " to " + lb;
                configParamNoRand(MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
            }
            else
            {
                std::string name = std::string("Mod ") + std::to_string(i % 4 + 1);
                auto r = configParamNoRand<modules::SurgeParameterModulationQuantity>(
                    MOD_PARAM_0 + i, -1, 1, 0, name);
                r->baseName = name;
            }
#endif
        }

        modAssist.initialize(this);

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
        snapCalculatedNames();
    }

    ADSRStorage *adsr{nullptr}, *adsr_display{nullptr};
    std::array<std::unique_ptr<ADSRModulationSource>, MAX_POLY> processors;
    std::array<rack::dsp::SchmittTrigger, MAX_POLY> triggers;
    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, true); // get those presets. FIXME skip wt later

        adsr = &(storage->getPatch().scene[0].adsr[0]);
        adsr_display = &(storage->getPatch().scene[0].adsr[1]);

        setupStorageRanges(&(adsr->a), &(adsr->mode));
        copyScenedataSubset(0, storage_id_start, storage_id_end);
        for (int i = 0; i < MAX_POLY; ++i)
        {
            processors[i] = std::make_unique<ADSRModulationSource>();
            processors[i]->init(storage.get(), adsr, storage->getPatch().scenedata[0], nullptr);
            doAttack[i] = false;
            doRelease[i] = false;

            level[i].target = 1.0;
            output[i].target = 0.0;
        }
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        switch (paramId)
        {
        case EG_A:
            return &adsr_display->a;
        case EG_D:
            return &adsr_display->d;
        case EG_S:
            return &adsr_display->s;
        case EG_R:
            return &adsr_display->r;
        case ANALOG_OR_DIGITAL:
            return &adsr_display->mode;
        }
        return nullptr;
    }

    int polyChannelCount() { return nChan; }
    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - LEVEL;
        if (offset >= n_mod_inputs * (n_mod_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs;
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - LEVEL;
        return MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - LEVEL;
        if (idx < 0 || idx >= n_mod_params)
            return 0;

        return modAssist.modvalues[idx][0];
    }

    bool isBipolar(int paramId) override
    {
        if (paramId == LEVEL || paramId == PAN)
        {
            return true;
        }
        return false;
    }

    void moduleSpecificSampleRateChange() override
    {
        clockProc.setSampleRate(APP->engine->getSampleRate());
    }
    modules::ClockProcessor<EGxVCA> clockProc;

    std::string getName() override { return std::string("EGxVCA"); }
    int processCount{BLOCK_SIZE - 1};

    int nChan{-1};

    bool doAttack[MAX_POLY], doRelease[MAX_POLY];

    struct linterp
    {
        float target{0};
        float dtarget{0};
        inline void setTarget(float f) { dtarget = (f - target) * BLOCK_SIZE_INV; }
        inline void step() { target += dtarget; }
    };

    linterp level[MAX_POLY], output[MAX_POLY], response[MAX_POLY];

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (inputs[CLOCK_IN].isConnected())
            clockProc.process(this, CLOCK_IN);
        else
            clockProc.disconnect(this);

        auto currChan = std::max({inputs[INPUT_L].getChannels(), inputs[INPUT_R].getChannels(),
                                  inputs[GATE_IN].getChannels(), 1});
        if (currChan != nChan)
        {
            nChan = currChan;
        }
        for (int c = 0; c < nChan; ++c)
        {
            processors[c]->correctAnalogMode = true; // use cprrected version

            if (triggers[c].process(inputs[GATE_IN].getVoltage(c)))
            {
                doAttack[c] = true;
            }
            if (inputs[GATE_IN].getVoltage(c) < 5 &&
                processors[c]->getEnvState() < ADSRState::s_release)
                doRelease[c] = true;
        }
        if (processCount == BLOCK_SIZE - 1)
        {
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);

            outputs[OUTPUT_L].setChannels(nChan);
            outputs[OUTPUT_R].setChannels(nChan);
            outputs[ENV_OUT].setChannels(nChan);

            adsr->mode.set_value_f01(params[ANALOG_OR_DIGITAL].getValue());

            for (int c = 0; c < nChan; ++c)
            {
                // FIXME - we need to do the tp / temposync thing here
                adsr->a.set_value_f01(modAssist.values[EG_A][c]);
                adsr->d.set_value_f01(modAssist.values[EG_D][c]);
                adsr->s.set_value_f01(modAssist.values[EG_S][c]);
                adsr->r.set_value_f01(modAssist.values[EG_R][c]);
                processors[c]->correctAnalogMode = true;
                copyScenedataSubset(0, storage_id_start, storage_id_end);

                if (doAttack[c])
                {
                    processors[c]->attack();
                    doAttack[c] = false;
                }
                if (doRelease[c])
                {
                    processors[c]->release();
                    doRelease[c] = false;
                }
                processors[c]->process_block();
                output[c].setTarget(processors[c]->get_output(0));

                auto nl = modules::DecibelParamQuantity::ampToLinear(modAssist.values[LEVEL][c]);
                level[c].setTarget(nl);

                response[c].setTarget(modAssist.values[RESPONSE][c]);
            }
            processCount = 0;
        }

        // ToDo - SIMDize
        for (int c = 0; c < nChan; ++c)
        {
            auto o1 = output[c].target;
            auto o3 = o1 * o1 * o1;
            auto r = response[c].target;
            auto o = (1 - r) * o1 + r * o3;

            auto l = level[c].target;
            auto ol = o * l;

            outputs[ENV_OUT].setVoltage(o1 * 10, c);
            outputs[OUTPUT_L].setVoltage(inputs[INPUT_L].getVoltage(c) * ol, c);
            outputs[OUTPUT_R].setVoltage(inputs[INPUT_R].getVoltage(c) * ol, c);
            output[c].step();
            level[c].step();
            response[c].step();
        }
        processCount++;
    }

    void activateTempoSync()
    {
        std::cout << "FIXME " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }

    void deactivateTempoSync()
    {
        std::cout << "FIXME " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }

    json_t *makeModuleSpecificJson() override { return nullptr; }

    void readModuleSpecificJson(json_t *modJ) override {}
};
} // namespace sst::surgext_rack::egxvca
#endif
