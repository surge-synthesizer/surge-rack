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

#include "dsp/SimpleLFO.h"

/*
 * ToDos
 *
 * Module
 *   - Code up the Modes
 *       - Independent
 *       - Temposync with C/M
 *       - Temposync with Phase
 *       - Interwoven / Spread
 *    - Config Param and Units
 *    - Some performance Especially SIMD the high poly cases
 *       - I bet we can do a better job with the simd-ized process
 *         for instance if we hand indeices and arrays to the process
 *         methods
 *       - Cache the offsets for uni / bipolar and do it SSE-wise
 *       - That sort of stuff
 * UI
 *   - Dynamic Laaels based on Mode
 *   - replace the t/k with the contents
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

namespace sst::surgext_rack::quadlfo
{
struct QuadLFO : modules::XTModule
{
    static constexpr int n_mod_params{8};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_lfos{4};

    enum ParamIds
    {
        RATE_0,
        DEFORM_0 = RATE_0 + n_lfos,
        SHAPE_0 = DEFORM_0 + n_lfos,
        BIPOLAR_0 = SHAPE_0 + n_lfos,
        MOD_PARAM_0 = BIPOLAR_0 + n_lfos,
        INTERPLAY_MODE = MOD_PARAM_0 + n_mod_params * n_mod_inputs,
        NUM_PARAMS
    };

    enum InputIds
    {
        TRIGGER_0,
        CLOCK_IN = TRIGGER_0 + n_lfos,

        MOD_INPUT_0,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs,
    };

    enum OutputIds
    {
        OUTPUT_0,
        NUM_OUTPUTS = OUTPUT_0 + n_lfos
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::ModulationAssistant<QuadLFO, n_mod_params, RATE_0, n_mod_inputs, MOD_INPUT_0>
        modAssist;

    QuadLFO() : XTModule()
    {
        {
            std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
            setupSurge();
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < n_lfos; ++i)
        {
            configParam(RATE_0 + i, -3, 6, 0);
            configParam(DEFORM_0 + i, -1, 1, 0);
            configSwitch(SHAPE_0 + i, 0, 5, 0, "Shape",
                         {"Sin", "Ramp", "Tri", "Pulse", "Rand", "S&H"});
            configSwitch(BIPOLAR_0 + i, 0, 1, 1, "Bipolar", {"Uni", "Bi"});
        }
        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
            configParam(MOD_PARAM_0 + i, -1, 1, 0);
        for (int i = 0; i < n_mod_inputs; ++i)
            configInput(MOD_INPUT_0 + i, "Mod " + std::to_string(i));

        configSwitch(INTERPLAY_MODE, 0, 3, 0, "LFO Inter-operation Mode",
                     {"Independent LFOs", "Quad Phase", "Ratio Rates", "Spread Rate"});

        modAssist.initialize(this);
        modAssist.setupMatrix(this);
        modAssist.updateValues(this);

        for (int i = 0; i < n_lfos; ++i)
        {
            for (int c = 0; c < MAX_POLY; ++c)
            {
                processors[i][c] = std::make_unique<dsp::modulators::SimpleLFO>(storage.get());
            }
        }

        snapCalculatedNames();
    }

    std::array<std::array<std::unique_ptr<dsp::modulators::SimpleLFO>, MAX_POLY>, n_lfos>
        processors;
    void setupSurge() { setupSurgeCommon(NUM_PARAMS, false); }

    int polyChannelCount() { return nChan; }
    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - RATE_0;
        if (offset >= n_mod_inputs * (n_mod_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs;
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - RATE_0;
        return MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - RATE_0;
        if (idx < 0 || idx >= n_mod_params)
            return 0;

        return modAssist.modvalues[idx][0];
    }

    bool isBipolar(int paramId) override
    {
        if (paramId >= DEFORM_0 && paramId < DEFORM_0 + n_lfos)
            return true;
        return false;
    }

    void moduleSpecificSampleRateChange() override
    {
        clockProc.setSampleRate(APP->engine->getSampleRate());
    }
    typedef modules::ClockProcessor<QuadLFO> clockProcessor_t;
    clockProcessor_t clockProc;

    std::string getName() override { return std::string("QuadLFO"); }

    int nChan{-1}, chanByLFO[n_lfos]{1, 1, 1, 1};
    std::atomic<int> forcePolyphony{-1};

    int processCount{BLOCK_SIZE};

    rack::dsp::SchmittTrigger triggers[n_lfos][MAX_POLY];
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (inputs[CLOCK_IN].isConnected())
            clockProc.process(this, CLOCK_IN);
        else
            clockProc.disconnect(this);

        if (processCount == BLOCK_SIZE)
        {
            processCount = 0;

            // auto currChan = std::max({inputs[INPUT_L].getChannels(),
            // inputs[INPUT_R].getChannels(), 1});
            if (forcePolyphony > 0)
            {
                for (int i = 0; i < n_lfos; ++i)
                {
                    // FIXME this is mode dependent
                    chanByLFO[i] = forcePolyphony;
                }
                nChan = forcePolyphony;
            }
            else
            {
                int cc = 1;
                for (int i = 0; i < n_lfos; ++i)
                {
                    // FIXME this is mode dependent
                    chanByLFO[i] = std::max(1, inputs[TRIGGER_0 + i].getChannels());
                    cc = std::max(cc, chanByLFO[i]);
                }
                nChan = cc;
            }

            for (int i = 0; i < n_lfos; ++i)
            {
                outputs[OUTPUT_0 + i].setChannels(chanByLFO[i]);
            }

            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
        }

        auto ip = (int)std::round(params[INTERPLAY_MODE].getValue());

        switch (ip)
        {
        case 0:
            processIndependentLFOs();
            break;
        case 1:
            processQuadPhaseLFOs();
        default:
            break;
        }
        processCount++;
    }

    void processQuadPhaseLFOs()
    {
        for (int i = 0; i < n_lfos; ++i)
        {
            auto uni = params[BIPOLAR_0 + i].getValue() < 0.5;
            auto off{0}, mul{5};
            if (uni)
                off = 1;
            bool ic = inputs[TRIGGER_0 + i].isConnected();
            auto shape = (int)std::round(params[SHAPE_0 + i].getValue());
            auto monoTrigger = ic && inputs[TRIGGER_0 + i].getChannels() == 1;
            for (int c = 0; c < chanByLFO[i]; ++c)
            {
                auto r = modAssist.values[RATE_0][c];
                // FIXME scales
                if (i != 0)
                {
                    auto dph = (modAssist.values[RATE_0 + i][c] + 3)/9;
                    processors[i][c]->applyPhaseOffset(dph);
                }
                if (ic && triggers[i][c].process(inputs[TRIGGER_0].getVoltage(c * (!monoTrigger))))
                {
                    processors[i][c]->attack(shape);
                }
                processors[i][c]->process(r, modAssist.values[DEFORM_0 + i][c], shape);
                outputs[OUTPUT_0 + i].setVoltage((processors[i][c]->output + off) * mul, c);
            }
        }
    }

    void processIndependentLFOs()
    {
        for (int i = 0; i < n_lfos; ++i)
        {
            auto uni = params[BIPOLAR_0 + i].getValue() < 0.5;
            auto off{0}, mul{5};
            if (uni)
                off = 1;
            bool ic = inputs[TRIGGER_0 + i].isConnected();
            auto shape = (int)std::round(params[SHAPE_0 + i].getValue());
            auto monoTrigger = ic && inputs[TRIGGER_0 + i].getChannels() == 1;
            for (int c = 0; c < chanByLFO[i]; ++c)
            {
                if (ic && triggers[i][c].process(inputs[TRIGGER_0].getVoltage(c * (!monoTrigger))))
                {
                    processors[i][c]->attack(shape);
                }
                processors[i][c]->process(modAssist.values[RATE_0 + i][c],
                                          modAssist.values[DEFORM_0 + i][c], shape);
                outputs[OUTPUT_0 + i].setVoltage((processors[i][c]->output + off) * mul, c);
            }
        }
    }

    void activateTempoSync()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }

    void deactivateTempoSync()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }

    json_t *makeModuleSpecificJson() override
    {
        auto vc = json_object();

        clockProc.toJson(vc);
        json_object_set(vc, "forcePolyphony", json_integer(forcePolyphony));

        return vc;
    }

    void readModuleSpecificJson(json_t *modJ) override
    {
        clockProc.fromJson(modJ);
        auto fp = json_object_get(modJ, "forcePolyphony");
        if (fp)
        {
            forcePolyphony = json_integer_value(fp);
        }
        else
        {
            forcePolyphony = -1;
        }
    }
};
} // namespace sst::surgext_rack::quadlfo
#endif
