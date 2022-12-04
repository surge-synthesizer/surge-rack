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
 *   - Code up the LFOs
 *   - Code up the Modes
 *       - Independent
 *       - Temposync with C/M
 *       - Temposync with Phase
 *       - Interwoven / Spread
 *    - Polyphony generally. Probably want both triggers and manual control
 *   - Config Param and Units
 * UI
 *   - Dynamic Laaels based on Mode
 *   - UI for modes
 *   - UI for each LFO with a shape selector/preview, a uni/bi, and
 *     a tempo display
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
        NUM_PARAMS = MOD_PARAM_0 + n_mod_params * n_mod_inputs
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
            configParam(RATE_0 + i, -3, 4, 0);
            configParam(DEFORM_0 + i, -1, 1, 0);
            configSwitch(SHAPE_0 + i, 0, 5, 0, "Shape",
                         {"Sin", "Ramp", "Tri", "Pulse", "Rand", "S&H"});
            configSwitch(BIPOLAR_0 + i, 0, 1, 1, "Bipolar", {"Uni", "Bi"});
        }
        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
            configParam(MOD_PARAM_0 + i, -1, 1, 0);
        for (int i = 0; i < n_mod_inputs; ++i)
            configInput(MOD_INPUT_0 + i, "Mod " + std::to_string(i));

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
    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, true); // get those presets. FIXME skip wt later
    }

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

    int nChan{-1};

    int processCount{BLOCK_SIZE};
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
            auto currChan = 1;
            if (currChan != nChan)
            {
                nChan = currChan;
            }

            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
        }

        for (int i = 0; i < n_lfos; ++i)
        {
            outputs[OUTPUT_0 + i].setChannels(nChan);
            for (int c = 0; c < nChan; ++c)
            {
                processors[i][c]->process(modAssist.values[RATE_0 + i][c],
                                          modAssist.values[DEFORM_0 + i][c], 0);
                outputs[OUTPUT_0 + i].setVoltage(processors[i][c]->output * 5);
            }
        }
        processCount++;
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

        return vc;
    }

    void readModuleSpecificJson(json_t *modJ) override { clockProc.fromJson(modJ); }
};
} // namespace sst::surgext_rack::quadlfo
#endif
