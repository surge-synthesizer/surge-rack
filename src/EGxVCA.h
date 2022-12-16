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
#include "dsp/ADSRDAHDEnvelope.h"

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

        ADSR_OR_DAHD,

        MOD_PARAM_0,
        NUM_PARAMS = MOD_PARAM_0 + n_mod_params * n_mod_inputs
    };

    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        GATE_IN,
        CLOCK_IN,
        MOD_INPUT_0,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs,
    };

    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        ENV_OUT,
        EOC_OUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::ModulationAssistant<EGxVCA, n_mod_params, LEVEL, n_mod_inputs, MOD_INPUT_0> modAssist;

    static_assert(modules::CTEnvTimeParamQuantity::etMin ==
                  dsp::envelopes::ADSRDAHDEnvelope::etMin);
    static_assert(modules::CTEnvTimeParamQuantity::etMax ==
                  dsp::envelopes::ADSRDAHDEnvelope::etMax);

    struct DAHDPQ : modules::CTEnvTimeParamQuantity
    {
        std::string getCalculatedName() override
        {
            switch (paramId)
            {
            case EG_A:
                return "Delay";
            case EG_D:
                return "Attack";
            case EG_S:
                return "Hold";
            case EG_R:
                return "Release";
            }
            return {};
        }
        bool isTempoSync() override
        {
            auto m = dynamic_cast<EGxVCA *>(module);
            if (m)
                return m->tempoSynced;
            return false;
        }
    };

    struct ADSRPQ : modules::CTEnvTimeParamQuantity
    {
        std::string getCalculatedName() override
        {
            switch (paramId)
            {
            case EG_A:
                return "Attach";
            case EG_D:
                return "Decay";
            case EG_S:
                return "Sustain";
            case EG_R:
                return "Release";
            }
            return {};
        }
        bool isTempoSync() override
        {
            auto m = dynamic_cast<EGxVCA *>(module);
            if (m)
                return m->tempoSynced;
            return false;
        }
    };

    struct TimePQ : modules::TypeSwappingParameterQuantity
    {
        TimePQ()
        {
            addImplementer<ADSRPQ>(0);
            addImplementer<DAHDPQ>(1);
        }
        int mode() override
        {
            if (!module)
                return 0;
            return (int)std::round(module->paramQuantities[ADSR_OR_DAHD]->getValue());
        }
    };

    struct SustainOrTimePQ : modules::TypeSwappingParameterQuantity
    {
        SustainOrTimePQ()
        {
            addImplementer<rack::ParamQuantity>(0);
            addImplementer<DAHDPQ>(1);
            impls[0]->name = "Sustain";
            impls[0]->unit = "%"; // fixme - doesn't come through it seems?
            impls[0]->defaultValue = 0.5;
            impls[0]->displayMultiplier = 100.0;
        }
        int mode() override
        {
            if (!module)
                return 0;
            return (int)std::round(module->paramQuantities[ADSR_OR_DAHD]->getValue());
        }
    };

    EGxVCA() : XTModule()
    {
        {
            std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
            setupSurge();
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<modules::DecibelParamQuantity>(LEVEL, 0, 2, 1, "Level");
        configParam(PAN, -1, 1, 0, "Pan", "%", 0, 100);
        configParam<TimePQ>(EG_A, 0, 1, 0.1, "Attack");
        configParam<TimePQ>(EG_D, 0, 1, 0.1, "Decay");
        configParam<SustainOrTimePQ>(EG_S, 0, 1, 0.5, "Sustain");
        configParam<TimePQ>(EG_R, 0, 1, 0.1, "Release");
        configSwitch(ANALOG_OR_DIGITAL, 0, 1, 0, "Curve", {"Digital", "Analog"});
        configSwitch(ADSR_OR_DAHD, 0, 1, 0, "Mode", {"ADSR", "DAHD"});

        configParam(RESPONSE, 0, 1, 0, "Linear/Exponential", "%", 0, 100);

        configSwitch(A_SHAPE, 0, 2, 1, "Attack Curve", {"Faster", "Standard", "Slower"});
        configSwitch(D_SHAPE, 0, 2, 1, "Decay Curve", {"Faster", "Standard", "Slower"});
        configSwitch(R_SHAPE, 0, 2, 1, "Decay Curve", {"Faster", "Standard", "Slower"});

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
        modAssist.setupMatrix(this);
        modAssist.updateValues(this);

        for (int i = 0; i < MAX_POLY; ++i)
            meterLevels[i] = 0.f;

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
        snapCalculatedNames();
    }

    float meterLevels[MAX_POLY];

    std::array<std::unique_ptr<dsp::envelopes::ADSRDAHDEnvelope>, MAX_POLY> processors;
    std::array<rack::dsp::SchmittTrigger, MAX_POLY> triggers;
    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false);

        for (int i = 0; i < MAX_POLY; ++i)
        {
            processors[i] = std::make_unique<dsp::envelopes::ADSRDAHDEnvelope>(storage.get());
            doAttack[i] = false;

            level[i].target = 1.0;
            response[i].target = 0.0;
        }
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
        for (int i = 0; i < MAX_POLY; ++i)
        {
            if (processors[i])
                processors[i]->onSampleRateChanged();
        }
    }
    typedef modules::ClockProcessor<EGxVCA> clockProcessor_t;
    clockProcessor_t clockProc;

    std::string getName() override { return std::string("EGxVCA"); }
    int processCount{BLOCK_SIZE};
    int meterUpdateCount{0};

    int nChan{-1};

    bool doAttack[MAX_POLY];

    struct linterp
    {
        float target{0};
        float dtarget{0};
        inline void setTarget(float f) { dtarget = (f - target) * BLOCK_SIZE_INV; }
        inline void step() { target += dtarget; }
    };

    linterp level[MAX_POLY], response[MAX_POLY];

    float aTS{0}, dTS{0}, sTS{0}, rTS{0};

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
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
        }

        for (int c = 0; c < nChan; ++c)
        {
            if (triggers[c].process(inputs[GATE_IN].getVoltage(c)))
            {
                doAttack[c] = true;
            }
        }
        if (processCount == BLOCK_SIZE)
        {
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
            processCount = 0;

            outputs[OUTPUT_L].setChannels(nChan);
            outputs[OUTPUT_R].setChannels(nChan);
            outputs[EOC_OUT].setChannels(nChan);
            outputs[ENV_OUT].setChannels(nChan);

            if (tempoSynced)
            {
                auto r = [this](auto i) {
                    auto res = temposync_support::roundTemposync(
                        modAssist.basevalues[i] * dsp::envelopes::ADSRDAHDEnvelope::etScale +
                        dsp::envelopes::ADSRDAHDEnvelope::etMin);
                    res = (res - dsp::envelopes::ADSRDAHDEnvelope::etMin) /
                          dsp::envelopes::ADSRDAHDEnvelope::etScale;
                    return res;
                };

                // OK so what do we want. If temposycn ratio is 2 we want the rate twice as fast
                // so that means we subtract 1.
                auto tsr = storage->temposyncratio;
                auto diff = 1 - tsr;
                // but remember this is all scaled by etScale
                diff = diff / dsp::envelopes::ADSRDAHDEnvelope::etScale;

                aTS = r(EG_A) + diff;
                dTS = r(EG_D) + diff;
                sTS = r(EG_S) + diff;
                rTS = r(EG_R) + diff;
            }
        }
        int as = (int)std::round(params[A_SHAPE].getValue());
        int ds = (int)std::round(params[D_SHAPE].getValue());
        int rs = (int)std::round(params[R_SHAPE].getValue());

        for (int c = 0; c < nChan; ++c)
        {
            if (doAttack[c])
            {
                auto m = (dsp::envelopes::ADSRDAHDEnvelope::Mode)std::round(
                    params[ADSR_OR_DAHD].getValue());
                auto as = (int)std::round(params[EG_A].getValue());
                auto dig = params[ANALOG_OR_DIGITAL].getValue() < 0.5;
                processors[c]->attackFrom(m, processors[c]->output, as, dig);
                doAttack[c] = false;
            }
            if (tempoSynced)
            {
                auto m = (dsp::envelopes::ADSRDAHDEnvelope::Mode)std::round(
                    params[ADSR_OR_DAHD].getValue());

                auto av = aTS + modAssist.modvalues[EG_A][c];
                auto dv = dTS + modAssist.modvalues[EG_D][c];
                auto sv = sTS + modAssist.modvalues[EG_S][c];
                auto rv = rTS + modAssist.modvalues[EG_R][c];
                processors[c]->process(av, dv,
                                       m == dsp::envelopes::ADSRDAHDEnvelope::ADSR_MODE
                                           ? modAssist.values[EG_S][c]
                                           : sv,
                                       rv, as, ds, rs, inputs[GATE_IN].getVoltage(c) > 2);
            }
            else
            {
                processors[c]->process(modAssist.values[EG_A][c], modAssist.values[EG_D][c],
                                       modAssist.values[EG_S][c], modAssist.values[EG_R][c], as, ds,
                                       rs, inputs[GATE_IN].getVoltage(c) > 2);
            }
            auto nl = modules::DecibelParamQuantity::ampToLinear(modAssist.values[LEVEL][c]);
            level[c].setTarget(nl);
            response[c].setTarget(modAssist.values[RESPONSE][c]);
        }

        // ToDo - SIMDize
        if (meterUpdateCount++ == BLOCK_SIZE * 128)
        {
            for (int i = 0; i < nChan; ++i)
            {
                meterLevels[i] = processors[i]->output;
            }
            meterUpdateCount = 0;
        }

        for (int c = 0; c < nChan; ++c)
        {
            auto o1 = processors[c]->output;
            auto o3 = o1 * o1 * o1;
            auto r = response[c].target;
            auto o = (1 - r) * o1 + r * o3;

            auto l = level[c].target;
            auto ol = o * l;

            outputs[ENV_OUT].setVoltage(o1 * 10, c);
            outputs[EOC_OUT].setVoltage(processors[c]->eoc_output * 10, c);
            outputs[OUTPUT_L].setVoltage(inputs[INPUT_L].getVoltage(c) * ol, c);
            outputs[OUTPUT_R].setVoltage(inputs[INPUT_R].getVoltage(c) * ol, c);

            level[c].step();
            response[c].step();
        }
        processCount++;
    }

    bool tempoSynced{false};
    void activateTempoSync() { tempoSynced = true; }

    void deactivateTempoSync() { tempoSynced = false; }
    json_t *makeModuleSpecificJson() override
    {
        auto vc = json_object();

        clockProc.toJson(vc);

        return vc;
    }

    void readModuleSpecificJson(json_t *modJ) override { clockProc.fromJson(modJ); }
};
} // namespace sst::surgext_rack::egxvca
#endif
