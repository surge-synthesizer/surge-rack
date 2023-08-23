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

namespace sst::surgext_rack::unisonhelper
{
struct UnisonHelper : modules::XTModule
{
    static constexpr int n_mod_params{3};
    static constexpr int n_mod_inputs{4};

    static constexpr int n_sub_vcos{4};

    enum ParamIds
    {
        VOICE_COUNT,
        DETUNE,
        DRIFT,
        DETUNE_EXTEND,
        CHARACTER,

        MOD_PARAM_0,
        NUM_PARAMS = MOD_PARAM_0 + n_mod_params * n_mod_inputs
    };

    enum InputIds
    {
        INPUT_VOCT,
        INPUT_SUB1,

        MOD_INPUT_0 = INPUT_SUB1 + n_sub_vcos,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs,
    };

    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,

        OUTPUT_VOCT_SUB1,
        NUM_OUTPUTS = OUTPUT_VOCT_SUB1 + n_sub_vcos
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::ModulationAssistant<UnisonHelper, n_mod_params, VOICE_COUNT, n_mod_inputs, MOD_INPUT_0> modAssist;

    UnisonHelper() : XTModule()
    {
        {
            std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
            setupSurge();
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        modAssist.initialize(this);
        modAssist.setupMatrix(this);
        modAssist.updateValues(this);

        snapCalculatedNames();
    }

    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false, false);
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        return nullptr;
    }

    int polyChannelCount()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        return 1;
    }
    static int paramModulatedBy(int modIndex)
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        return -1;
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        return 0;
    }

    float modulationDisplayValue(int paramId) override
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        int idx = paramId - VOICE_COUNT;
        if (idx < 0 || idx >= n_mod_params)
            return 0;

        return modAssist.modvalues[idx][0];
    }

    bool isBipolar(int paramId) override { return false; }

    void moduleSpecificSampleRateChange() override
    {

    }

    std::string getName() override { return std::string("UnisonHelper"); }

    int nChan{-1};

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        auto currChan = std::max({inputs[INPUT_VOCT].getChannels(), 1});
        if (currChan != nChan)
        {
            nChan = currChan;
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
};
} // namespace sst::surgext_rack::unisonhelper
#endif
