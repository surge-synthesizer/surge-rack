//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_LFO_HPP
#define XTRACK_LFO_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"
#include "dsp/modulators/LFOModulationSource.h"

#include <array>
#include <memory>

namespace sst::surgext_rack::lfo
{
struct LFO : modules::XTModule
{
    static constexpr int n_lfo_params{10};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        RATE,
        PHASE,
        DEFORM,
        AMPLITUDE,

        E_DELAY,
        E_ATTACK,
        E_HOLD,
        E_DECAY,
        E_SUSTAIN,
        E_RELEASE,

        SHAPE,
        UNIPOLAR,

        LFO_MOD_PARAM_0,

        LFO_TYPE = LFO_MOD_PARAM_0 + n_lfo_params * n_mod_inputs,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_TRIGGER,
        INPUT_CLOCK_RATE,
        INPUT_CLOCK_ENV,
        INPUT_PHASE_DIRECT,
        LFO_MOD_INPUT,
        NUM_INPUTS = LFO_MOD_INPUT + n_mod_inputs,

    };
    enum OutputIds
    {
        OUTPUT_MIX,
        OUTPUT_ENV,
        OUTPUT_WAVE,
        OUTPUT_TRIGPHASE,
        OUTPUT_TRIGF,
        OUTPUT_TRIGA,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    LFO() : XTModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        setupSurge();
    }
    
    std::string getName() override { return "LFO"; }

    std::array<std::unique_ptr<LFOModulationSource>, MAX_POLY> surge_lfo;
    std::unique_ptr<StepSequencerStorage> surge_ss;
    std::unique_ptr<MSEGStorage> surge_ms;
    std::unique_ptr<FormulaModulatorStorage> surge_fs;
    LFOStorage *lfostorage;

    rack::simd::float_4 output0[3][4], output1[3][4];

    std::map<int, size_t> paramOffsetByID;

    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        for (int i = 0; i < MAX_POLY; ++i)
            surge_lfo[i] = std::make_unique<LFOModulationSource>();
        surge_ss = std::make_unique<StepSequencerStorage>();
        surge_ms = std::make_unique<MSEGStorage>();
        surge_fs = std::make_unique<FormulaModulatorStorage>();

        lfostorage = &(storage->getPatch().scene[0].lfo[0]);
        lfostorage->delay.deactivated = false;
        lfostorage->trigmode.val.i = lm_keytrigger;

        paramOffsetByID[RATE] = 0;
        paramOffsetByID[SHAPE] = 1;
        paramOffsetByID[PHASE] = 2;
        paramOffsetByID[AMPLITUDE] = 3;
        paramOffsetByID[DEFORM] = 4;
        // skip trigmode
        paramOffsetByID[UNIPOLAR] = 6;
        paramOffsetByID[E_DELAY] = 7;
        paramOffsetByID[E_HOLD] = 8; // yeah the storage object is mis-ordered
        paramOffsetByID[E_ATTACK] = 9;
        paramOffsetByID[E_DECAY] = 10;
        paramOffsetByID[E_SUSTAIN] = 11;
        paramOffsetByID[E_RELEASE] = 12;

        auto *par0 = &(lfostorage->rate);

        for (int p = RATE; p < LFO_MOD_PARAM_0; ++p)
        {
            auto *par = &par0[paramOffsetByID[p]];
            configParam<modules::SurgeParameterParamQuantity>(p, 0, 1, par->get_default_value_f01(),
                                                              par->get_name());
        }

        for (int i = 0; i < MAX_POLY; ++i)
        {
            surge_lfo[i]->assign(storage.get(), lfostorage, storage->getPatch().scenedata[0],
                                 nullptr, surge_ss.get(), surge_ms.get(), surge_fs.get());
            isGated[i] = false;
            isGateConnected[i] = false;
        }
        setupStorageRanges(&(lfostorage->rate), &(lfostorage->release));

        for (int s = 0; s < 3; ++s)
        {
            for (int i = 0; i < 4; ++i)
            {
                output0[s][i] = rack::simd::float_4::zero();
                output1[s][i] = rack::simd::float_4::zero();
            }
        }
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        if (paramOffsetByID.find(paramId) == paramOffsetByID.end())
            return nullptr;

        auto po = paramOffsetByID[paramId];
        auto *par0 = &(lfostorage->rate);
        auto *par = &par0[po];
        return par;
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - RATE;
        return LFO_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    int lastStep = BLOCK_SIZE;
    int lastNChan = -1;
    bool firstProcess{true};

    rack::dsp::SchmittTrigger envGateTrigger[MAX_POLY], envRetrig[MAX_POLY];
    bool isGated[MAX_POLY], isGateConnected[MAX_POLY];

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[INPUT_TRIGGER].getChannels());
        outputs[OUTPUT_MIX].setChannels(nChan);
        outputs[OUTPUT_ENV].setChannels(nChan);
        outputs[OUTPUT_WAVE].setChannels(nChan);
        if (nChan != lastNChan)
        {
            firstProcess = true;
            lastNChan = nChan;
            for (int i = nChan; i < MAX_POLY; ++i)
                lastStep = BLOCK_SIZE;
        }

        if (lastStep == BLOCK_SIZE)
            lastStep = 0;

        if (lastStep == 0)
        {
            for (int s = 0; s < 3; ++s)
                for (int i = 0; i < 4; ++i)
                    output0[s][i] = output1[s][i];

            float ts[16];

            for (int c = 0; c < nChan; ++c)
            {
                // FIX obvs replace this with the mod matrix
                auto *par0 = &(lfostorage->rate);
                for (int p = RATE; p < LFO_MOD_PARAM_0; ++p)
                {
                    auto *par = &par0[paramOffsetByID[p]];
                    par->set_value_f01(params[p].getValue());
                }

                bool inNewAttack = firstProcess;
                if (inputs[INPUT_TRIGGER].isConnected() &&
                    envGateTrigger[c].process(inputs[INPUT_TRIGGER].getVoltage(c)))
                {
                    lfostorage->trigmode.val.i = lm_keytrigger;
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    isGated[c] = true;
                    inNewAttack = true;
                    isGateConnected[c] = true;
                }
                else if (inputs[INPUT_TRIGGER].isConnected() && isGated[c] &&
                         inputs[INPUT_TRIGGER].getVoltage(c) < 1.f)
                {
                    isGated[c] = false;
                    surge_lfo[c]->release();
                    isGateConnected[c] = true;
                }
                else if (!inputs[INPUT_TRIGGER].isConnected())
                {
                    if (isGateConnected[c])
                        inNewAttack = true;
                    if (!isGated[c])
                        inNewAttack = true;
                    isGated[c] = true;
                    isGateConnected[c] = false;
                }

                copyScenedataSubset(0, storage_id_start, storage_id_end);
                if (inNewAttack)
                {
                    surge_lfo[c]->attack();
                }
                surge_lfo[c]->process_block();
                if (inNewAttack)
                {
                    // Do the painful thing in the infrequent case
                    output0[0][c / 4].s[c % 4] = surge_lfo[c]->get_output(0);
                    output0[1][c / 4].s[c % 4] = surge_lfo[c]->get_output(1);
                    output0[2][c / 4].s[c % 4] = surge_lfo[c]->get_output(2);
                    surge_lfo[c]->process_block();
                }
                ts[c] = surge_lfo[c]->get_output(0);
            }
            for (int i = 0; i < 4; ++i)
                output1[0][i] = rack::simd::float_4::load(ts + i * 4);

            firstProcess = false;
        }

        float frac = 1.0 * lastStep / BLOCK_SIZE;
        lastStep++;
        for (int c = 0; c < nChan; c += 4)
        {
            rack::simd::float_4 outputI =
                (output0[0][c / 4] * (1.0 - frac) + output1[0][c / 4] * frac) *
                SURGE_TO_RACK_OSC_MUL;
            outputI.store(outputs[OUTPUT_MIX].getVoltages(c));
        }
    }

};
} // namespace sst::surgext_rack::lfo
#endif // RACK_HACK_LFO_HPP
