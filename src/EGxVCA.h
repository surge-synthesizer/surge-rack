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

#include "sst/basic-blocks/modulators/ADSREnvelope.h"
#include "sst/basic-blocks/modulators/DAHDEnvelope.h"
#include "sst/basic-blocks/dsp/PanLaws.h"

namespace sst::surgext_rack::egxvca
{
struct EGxVCA : modules::XTModule
{
    static constexpr int n_mod_params{7};
    static constexpr int n_mod_inputs{4};

    typedef basic_blocks::modulators::ADSREnvelope<SurgeStorage, BLOCK_SIZE> envelopeAdsr_t;
    typedef basic_blocks::modulators::ADSREnvelope<SurgeStorage, BLOCK_SIZE,
                                                   basic_blocks::modulators::TwoMinuteRange>
        envelopeAdsrSlow_t;
    typedef basic_blocks::modulators::DAHDEnvelope<SurgeStorage, BLOCK_SIZE> envelopeDahd_t;
    typedef basic_blocks::modulators::DAHDEnvelope<SurgeStorage, BLOCK_SIZE,
                                                   basic_blocks::modulators::TwoMinuteRange>
        envelopeDahdSlow_t;

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
        ATTACK_FROM = MOD_PARAM_0 + n_mod_params * n_mod_inputs,
        FAST_OR_SLOW,
        NUM_PARAMS
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
        bool getMinString(std::string &s) override
        {
            if (paramId == EG_A)
            {
                s = "Delay Skipped";
                return true;
            }
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
                return "Attack";
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

    struct SetMinMaxPQFeature
    {
        virtual ~SetMinMaxPQFeature() = default;
        virtual void setMinMax(float min, float max) = 0;
    };

    struct TimePQ : modules::TypeSwappingParameterQuantity, SetMinMaxPQFeature
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
        void setMinMax(float min, float max) override
        {
            auto m0 = dynamic_cast<modules::CTEnvTimeParamQuantity *>(impls[0].get());
            if (m0)
            {
                m0->etMin = min;
                m0->etMax = max;
            }
            auto m1 = dynamic_cast<modules::CTEnvTimeParamQuantity *>(impls[1].get());
            if (m1)
            {
                m1->etMin = min;
                m1->etMax = max;
            }
        }
    };

    struct SustainOrTimePQ : modules::TypeSwappingParameterQuantity, SetMinMaxPQFeature
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
        void setMinMax(float min, float max) override
        {
            auto m1 = dynamic_cast<modules::CTEnvTimeParamQuantity *>(impls[1].get());
            if (m1)
            {
                m1->etMin = min;
                m1->etMax = max;
            }
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
        for (int i = 0; i < n_mod_inputs * n_mod_params; ++i)
        {
            auto pid = paramModulatedBy(i + MOD_PARAM_0);
            auto id = i % n_mod_inputs;
            auto pq = configParamNoRand<modules::ModulateFromToParamQuantity>(
                MOD_PARAM_0 + i, -1, 1, 0, "Mod", "%", 0, 100);
            pq->setup(id, pid);
        }

        for (int i = 0; i < n_mod_inputs; ++i)
            configInput(MOD_INPUT_0 + i, "Mod Input " + std::to_string(i + 1));

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        configInput(GATE_IN, "Gate/Trig");
        configInput(CLOCK_IN, "Clock");

        configOutput(OUTPUT_L, "Left");
        configOutput(OUTPUT_R, "Right");
        configOutput(ENV_OUT, "Envelope");
        configOutput(EOC_OUT, "End of Cycle");

        configSwitch(ATTACK_FROM, 0, 1, 1, "Attack From", {"Zero", "Current Value"})
            ->randomizeEnabled = false;
        configSwitch(FAST_OR_SLOW, 0, 1, 0, "Fast or Slow", {"Fast", "Slow"})->randomizeEnabled =
            false;

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

    std::array<std::unique_ptr<envelopeAdsr_t>, MAX_POLY> processorsAdsr;
    std::array<std::unique_ptr<envelopeAdsrSlow_t>, MAX_POLY> processorsAdsrSlow;
    std::array<std::unique_ptr<envelopeDahd_t>, MAX_POLY> processorsDahd;
    std::array<std::unique_ptr<envelopeDahdSlow_t>, MAX_POLY> processorsDahdSlow;
    std::array<rack::dsp::SchmittTrigger, MAX_POLY> triggers;
    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false, false);

        for (int i = 0; i < MAX_POLY; ++i)
        {
            processorsAdsr[i] = std::make_unique<envelopeAdsr_t>(storage.get());
            processorsAdsrSlow[i] = std::make_unique<envelopeAdsrSlow_t>(storage.get());
            processorsDahd[i] = std::make_unique<envelopeDahd_t>(storage.get());
            processorsDahdSlow[i] = std::make_unique<envelopeDahdSlow_t>(storage.get());
            doAttack[i] = false;

            level[i].target = 1.0;
            response[i].target = 0.0;
            pan[i][0].target = 1.0; // L
            pan[i][1].target = 1.0; // R
            pan[i][2].target = 0.0; // R in L
            pan[i][3].target = 0.0; // L in R
        }
    }

    int polyChannelCount() { return nChan; }
    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - MOD_PARAM_0;
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
            if (processorsAdsr[i])
                processorsAdsr[i]->onSampleRateChanged();
            if (processorsDahd[i])
                processorsDahd[i]->onSampleRateChanged();
            if (processorsAdsrSlow[i])
                processorsAdsrSlow[i]->onSampleRateChanged();
            if (processorsDahdSlow[i])
                processorsDahdSlow[i]->onSampleRateChanged();
        }
    }
    typedef modules::ClockProcessor<EGxVCA> clockProcessor_t;
    clockProcessor_t clockProc;

    std::string getName() override { return std::string("EGxVCA"); }
    int processCount{BLOCK_SIZE};
    int meterUpdateCount{0};

    int nChan{-1};
    bool polyGate{false};

    bool doAttack[MAX_POLY];

    struct linterp
    {
        float target{0};
        float dtarget{0};
        inline void setTarget(float f) { dtarget = (f - target) * BLOCK_SIZE_INV; }
        inline void step() { target += dtarget; }
    };

    linterp level[MAX_POLY], response[MAX_POLY];
    linterp pan[MAX_POLY][4];

    float aTS{0}, dTS{0}, sTS{0}, rTS{0};

    template <typename ENVT>
    void processFastSlow(const typename rack::Module::ProcessArgs &args,
                         const std::array<std::unique_ptr<ENVT>, MAX_POLY> &procs)
    {
        if (inputs[CLOCK_IN].isConnected())
            clockProc.process(this, CLOCK_IN);
        else
            clockProc.disconnect(this);

        if (processCount == BLOCK_SIZE)
        {
            /*
             * Over the block is modulation, pan, level etc....
             */
            nChan = std::max({inputs[INPUT_L].getChannels(), inputs[INPUT_R].getChannels(),
                              inputs[GATE_IN].getChannels(), 1});
            polyGate = inputs[GATE_IN].getChannels() > 1;

            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
            processCount = 0;

            outputs[OUTPUT_L].setChannels(nChan);
            outputs[OUTPUT_R].setChannels(nChan);
            outputs[EOC_OUT].setChannels(nChan);
            outputs[ENV_OUT].setChannels(nChan);

            if (tempoSynced)
            {
                auto r = [this, &procs](auto i) {
                    auto res = temposync_support::roundTemposync(
                        procs[0]->rateFrom01(modAssist.basevalues[i]));
                    res = procs[0]->rateTo01(res);
                    return res;
                };

                // OK so what do we want. If temposycn ratio is 2 we want the rate twice as fast
                // so that means we subtract 1.
                auto tsr = storage->temposyncratio;
                auto diff = 1 - tsr;
                // but remember this is all scaled by etScale
                diff = procs[0]->deltaTo01(diff);

                aTS = r(EG_A) + diff;
                dTS = r(EG_D) + diff;
                sTS = r(EG_S) + diff;
                rTS = r(EG_R) + diff;
            }

            for (int c = 0; c < nChan; ++c)
            {
                auto nl = modules::DecibelParamQuantity::ampToLinear(modAssist.values[LEVEL][c]);
                level[c].setTarget(nl);
                response[c].setTarget(modAssist.values[RESPONSE][c]);

                if (inputs[INPUT_R].isConnected())
                {
                    // Assume stereo
                    basic_blocks::dsp::pan_laws::panmatrix_t pm;
                    basic_blocks::dsp::pan_laws::stereoEqualPower(
                        modAssist.values[PAN][c] * 0.5 + 0.5, pm);
                    for (int pl = 0; pl < 4; pl++)
                    {
                        pan[c][pl].setTarget(pm[pl]);
                    }
                }
                else
                {
                    // assume mono from L
                    basic_blocks::dsp::pan_laws::panmatrix_t pm;
                    basic_blocks::dsp::pan_laws::monoEqualPower(
                        modAssist.values[PAN][c] * 0.5 + 0.5, pm);
                    for (int pl = 0; pl < 4; pl++)
                    {
                        pan[c][pl].setTarget(pm[pl]);
                    }
                }
            }
        }

        for (int c = 0; c < nChan; ++c)
        {
            if (triggers[c].process(inputs[GATE_IN].getVoltage(polyGate * c)))
            {
                doAttack[c] = true;
            }
        }

        int as = (int)std::round(params[A_SHAPE].getValue());
        int ds = (int)std::round(params[D_SHAPE].getValue());
        int rs = (int)std::round(params[R_SHAPE].getValue());

        for (int c = 0; c < nChan; ++c)
        {
            if (doAttack[c])
            {
                auto as = (int)std::round(params[A_SHAPE].getValue());
                auto dig = params[ANALOG_OR_DIGITAL].getValue() < 0.5;
                auto az = (int)std::round(params[ATTACK_FROM].getValue());
                auto av = modAssist.values[EG_A][c];
                if (tempoSynced)
                {
                    av = aTS + modAssist.modvalues[EG_A][c];
                }
                procs[c]->attackFrom(az * procs[c]->output, av, as, dig);
                doAttack[c] = false;
            }
            if (tempoSynced)
            {
                auto av = aTS + modAssist.modvalues[EG_A][c];
                auto dv = dTS + modAssist.modvalues[EG_D][c];
                auto sv = sTS + modAssist.modvalues[EG_S][c];
                auto rv = rTS + modAssist.modvalues[EG_R][c];
                procs[c]->process(av, dv, getMode() == 0 ? modAssist.values[EG_S][c] : sv, rv, as,
                                  ds, rs, inputs[GATE_IN].getVoltage(polyGate * c) > 2);
            }
            else
            {
                procs[c]->process(modAssist.values[EG_A][c], modAssist.values[EG_D][c],
                                  modAssist.values[EG_S][c], modAssist.values[EG_R][c], as, ds, rs,
                                  inputs[GATE_IN].getVoltage(polyGate * c) > 2);
            }
        }

        // ToDo - SIMDize
        if (meterUpdateCount++ == BLOCK_SIZE * 128)
        {
            for (int i = 0; i < nChan; ++i)
            {
                meterLevels[i] = procs[i]->output;
            }
            meterUpdateCount = 0;
        }

        for (int c = 0; c < nChan; ++c)
        {
            auto o1 = procs[c]->output;
            auto o3 = procs[c]->outputCubed;
            auto r = response[c].target;
            auto o = (1 - r) * o1 + r * o3;

            auto l = level[c].target;
            auto ol = o * l;

            auto lV = inputs[INPUT_L].getVoltage(c) * ol;
            auto rV = inputs[INPUT_R].getVoltage(c) * ol;

            auto nlV = lV * pan[c][0].target + rV * pan[c][2].target;
            auto nrV = rV * pan[c][1].target + lV * pan[c][3].target;

            outputs[ENV_OUT].setVoltage(o1 * 10, c);
            outputs[EOC_OUT].setVoltage(procs[c]->eoc_output * 10, c);
            outputs[OUTPUT_L].setVoltage(nlV, c);
            outputs[OUTPUT_R].setVoltage(nrV, c);

            // even at 16-way egxvca this takes 1% of the cpu but still we could probably
            // make these steps simd operations to save a smidge one day
            level[c].step();
            response[c].step();

            for (int q = 0; q < 4; ++q)
                pan[c][q].step();
        }
        processCount++;
    }

    bool lastSlow{false};
    bool isSlow()
    {
        auto s = (bool)std::round(getParam(FAST_OR_SLOW).getValue());
        return s;
    }
    bool lastMode{0};
    bool getMode()
    {
        auto s = (bool)std::round(getParam(ADSR_OR_DAHD).getValue());
        return s;
    }
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        auto s = isSlow();
        if (s != lastSlow)
        {
            resetEnvelopes();
            lastSlow = s;
        }
        auto m = getMode();
        if (m != lastMode)
        {
            resetEnvelopes();
            lastMode = m;
        }
        if (m == 0)
        {
            if (s)
                processFastSlow(args, processorsAdsrSlow);
            else
                processFastSlow(args, processorsAdsr);
        }
        else
        {
            if (s)
                processFastSlow(args, processorsDahdSlow);
            else
                processFastSlow(args, processorsDahd);
        }
    }

    void resetEnvelopes()
    {
        for (const auto &p : processorsAdsr)
        {
            p->immediatelySilence();
        }
        for (const auto &p : processorsAdsrSlow)
        {
            p->immediatelySilence();
        }
        for (const auto &p : processorsDahd)
        {
            p->immediatelySilence();
        }
        for (const auto &p : processorsDahdSlow)
        {
            p->immediatelySilence();
        }
        auto s = isSlow();
        for (auto pqi : {EG_A, EG_D, EG_S, EG_R})
        {
            auto mm = dynamic_cast<SetMinMaxPQFeature *>(paramQuantities[pqi]);
            if (mm)
            {
                if (s)
                {
                    mm->setMinMax(envelopeAdsrSlow_t::etMin, envelopeAdsrSlow_t::etMax);
                }
                else
                {
                    mm->setMinMax(envelopeAdsr_t::etMin, envelopeAdsr_t::etMax);
                }
            }
        }
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
