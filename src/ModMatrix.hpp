//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_MODMATRIX_HPP
#define XTRACK_MODMATRIX_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::modmatrix
{
struct ModMatrix : modules::XTModule
{
    static constexpr int n_matrix_params{8};
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        TARGET0,

        MATRIX_MOD_PARAM_0 = TARGET0 + n_matrix_params,

        NUM_PARAMS = MATRIX_MOD_PARAM_0 + n_matrix_params * n_mod_inputs,
    };
    enum InputIds
    {
        MATRIX_MOD_INPUT,
        NUM_INPUTS = MATRIX_MOD_INPUT + n_mod_inputs,

    };
    enum OutputIds
    {
        OUTPUT_0,

        NUM_OUTPUTS = OUTPUT_0 + n_matrix_params
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    ModMatrix() : XTModule()
    {
        std::lock_guard<std::mutex> ltg(xtSurgeCreateMutex);

        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = TARGET0; i <= TARGET0 + n_matrix_params; ++i)
        {
            configParam(i, -10, 10, 0);
        }
        for (int i = 0; i < n_matrix_params * n_mod_inputs; ++i)
        {
            configParam(MATRIX_MOD_PARAM_0 + i, -1, 1, 0);
        }

        modulationAssistant.initialize(this);
    }
    std::string getName() override { return "ModMatrix"; }

    float modulationDisplayValue(int paramId) override
    {
        return modulationAssistant.animValues[paramId - TARGET0];
    }
    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - TARGET0;
        return MATRIX_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    bool isBipolar(int paramId) override
    {
        if (paramId >= TARGET0 && paramId < TARGET0 + n_matrix_params)
            return true;
        return false;
    }

    modules::ModulationAssistant<ModMatrix, n_matrix_params, TARGET0, n_mod_inputs,
                                 MATRIX_MOD_INPUT>
        modulationAssistant;

    static constexpr int slowUpdate{8};
    int blockPos{slowUpdate};

    int polyDepth{1}, polyDepthBy4{1};

    int polyChannelCount() { return polyDepth; }

    void process(const ProcessArgs &args) override
    {
        if (blockPos == slowUpdate)
        {
            modulationAssistant.setupMatrix(this);
            blockPos = 0;

            auto npd = 1;
            for (int i = MATRIX_MOD_INPUT; i < MATRIX_MOD_INPUT + n_mod_inputs; i++)
            {
                auto pd = inputs[i].getChannels();
                npd = std::max(npd, pd);
            }

            polyDepth = npd;
            polyDepthBy4 = (polyDepth - 1) / 4 + 1;

            blockPos = 0;
            for (int i = OUTPUT_0; i < OUTPUT_0 + n_matrix_params; ++i)
                outputs[i].setChannels(polyDepth);
        }

        modulationAssistant.updateValues(this);

        for (int p = 0; p < polyDepthBy4; ++p)
        {
            for (int oi = OUTPUT_0; oi < OUTPUT_0 + n_matrix_params; oi++)
            {
                _mm_storeu_ps(outputs[oi].getVoltages(p * 4),
                              modulationAssistant.valuesSSE[oi - OUTPUT_0][p]);
            }
        }
        blockPos++;
    }
};
} // namespace sst::surgext_rack::modmatrix
#endif // RACK_HACK_mixer_HPP
