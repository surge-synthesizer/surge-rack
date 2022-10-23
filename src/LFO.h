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

#ifndef XTRACK_LFO_HPP
#define XTRACK_LFO_HPP

#include "SurgeXT.h"
#include "XTModule.h"
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
        DEFORM_TYPE,
        WHICH_TEMPOSYNC,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_TRIGGER,
        INPUT_CLOCK_RATE,
        INPUT_PHASE_DIRECT,
        LFO_MOD_INPUT,
        NUM_INPUTS = LFO_MOD_INPUT + n_mod_inputs,

    };
    enum OutputIds
    {
        OUTPUT_MIX,
        OUTPUT_WAVE,
        OUTPUT_ENV,
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
    LFOStorage *lfostorageDisplay;

    modules::ModulationAssistant<LFO, n_lfo_params, RATE, n_mod_inputs, LFO_MOD_INPUT> modAssist;

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

        lfostorageDisplay = &(storage->getPatch().scene[0].lfo[1]);
        lfostorageDisplay->delay.deactivated = false;
        lfostorageDisplay->trigmode.val.i = lm_keytrigger;

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

        for (int p = LFO_MOD_PARAM_0; p < LFO_TYPE; ++p)
        {
            auto name =
                std::string("Mod ") + std::to_string((p - LFO_MOD_PARAM_0) % n_mod_inputs + 1);
            configParam<modules::SurgeParameterModulationQuantity>(p, -1, 1, 0, name);
        }

        configParam(DEFORM_TYPE, 0, 4, 0, "Deform Type");
        configParam(WHICH_TEMPOSYNC, 0, 3, 1, "Which Temposync");

        for (int i = 0; i < MAX_POLY; ++i)
        {
            surge_lfo[i]->assign(storage.get(), lfostorage, storage->getPatch().scenedata[0],
                                 nullptr, surge_ss.get(), surge_ms.get(), surge_fs.get());
            isGated[i] = false;
            isGateConnected[i] = false;
            isTriggered[i] = false;
            priorIntPhase[i] = -1;
            endPhaseCountdown[i] = 0;
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

        modAssist.initialize(this);
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        if (paramOffsetByID.find(paramId) == paramOffsetByID.end())
        {
            std::cout << "ERROR: NOT FOUND PARAM ID " << paramId << std::endl;
            return nullptr;
        }

        auto po = paramOffsetByID[paramId];
        auto *par0 = &(lfostorage->rate);
        auto *par = &par0[po];
        return par;
    }

    Parameter *surgeDisplayParameterForModulatorParamId(int modParamId) override
    {
        auto paramId = paramModulatedBy(modParamId);
        if (paramId < RATE || paramId >= RATE + n_lfo_params)
            return nullptr;

        auto po = paramOffsetByID[paramId];
        auto *par0 = &(lfostorage->rate);
        auto *par = &par0[po];
        return par;
    }

    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - LFO_MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_lfo_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - RATE;
        if (idx < 0 || idx >= n_lfo_params + 1)
            return 0;
        return modAssist.animValues[idx];
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - RATE;
        return LFO_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    int polyChannelCount() { return std::max(1, lastNChan); }

    int lastStep = BLOCK_SIZE;
    int lastNChan = -1;
    bool firstProcess{true};

    rack::dsp::SchmittTrigger envGateTrigger[MAX_POLY], envRetrig[MAX_POLY];
    bool isGated[MAX_POLY], isGateConnected[MAX_POLY], isTriggered[MAX_POLY];
    int priorIntPhase[MAX_POLY], endPhaseCountdown[MAX_POLY];

    modules::ClockProcessor<LFO> clockProc;

    void activateTempoSync()
    {
        auto wts = (int)std::round(paramQuantities[LFO::WHICH_TEMPOSYNC]->getValue());
        auto wtR = (bool)(wts & 1);
        auto wtE = (bool)(wts & 2);

        for (auto ls : {lfostorage, lfostorageDisplay})
        {
            ls->rate.temposync = wtR;
            auto *par0 = &(ls->rate);

            for (int p = E_DELAY; p < LFO_MOD_PARAM_0; ++p)
            {
                auto *par = &par0[paramOffsetByID[p]];
                if (par->can_temposync())
                    par->temposync = wtE;
            }
        }
    }
    void deactivateTempoSync()
    {
        for (auto ls : {lfostorage, lfostorageDisplay})
        {
            auto *par0 = &(ls->rate);
            for (int p = RATE; p < LFO_MOD_PARAM_0; ++p)
            {
                auto *par = &par0[paramOffsetByID[p]];
                if (par->can_temposync())
                    par->temposync = false;
            }
        }
    }

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

        for (int c = 0; c < nChan; ++c)
            if (inputs[INPUT_TRIGGER].isConnected() &&
                envGateTrigger[c].process(inputs[INPUT_TRIGGER].getVoltage(c)))
                isTriggered[c] = true;

        if (inputs[INPUT_CLOCK_RATE].isConnected())
            clockProc.process(this, INPUT_CLOCK_RATE);
        else
            clockProc.disconnect(this);

        if (lastStep == BLOCK_SIZE)
            lastStep = 0;

        if (lastStep == 0)
        {
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);

            for (int s = 0; s < 3; ++s)
                for (int i = 0; i < 4; ++i)
                    output0[s][i] = output1[s][i];

            float ts[3][16];

            {
                // Setup the display storage
                auto *par0 = &(lfostorageDisplay->rate);
                for (int p = RATE; p < LFO_MOD_PARAM_0; ++p)
                {
                    auto *par = &par0[paramOffsetByID[p]];
                    par->set_value_f01(params[p].getValue());
                }

                auto dt = (int)std::round(params[LFO::DEFORM_TYPE].getValue());
                auto nd = lt_num_deforms[lfostorageDisplay->shape.val.i];
                auto dto = std::clamp(dt, 0, nd ? (nd - 1) : 0);
                if (dto != dt)
                {
                    params[LFO::DEFORM_TYPE].setValue(dto);
                }
                lfostorageDisplay->deform.deform_type = dto;
            }

            for (int c = 0; c < nChan; ++c)
            {
                // FIX obvs replace this with the mod matrix
                auto *par0 = &(lfostorage->rate);
                for (int p = RATE; p < LFO_MOD_PARAM_0; ++p)
                {
                    auto *par = &par0[paramOffsetByID[p]];

                    if (p < n_lfo_params)
                        par->set_value_f01(modAssist.values[p - RATE][c]);
                    else
                        par->set_value_f01(params[p].getValue());
                }
                auto dt = (int)std::round(params[LFO::DEFORM_TYPE].getValue());
                dt = std::clamp(dt, 0, lt_num_deforms[lfostorage->shape.val.i]);
                lfostorage->deform.deform_type = dt;

                bool inNewAttack = firstProcess;
                // move this to every sample and record it eliminating the first process thing too
                if (isTriggered[c])
                {
                    lfostorage->trigmode.val.i = lm_keytrigger;
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    isGated[c] = true;
                    inNewAttack = true;
                    isGateConnected[c] = true;
                    isTriggered[c] = false;
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
                    priorIntPhase[c] = -1;
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
                // repeat for env stage also
                if (surge_lfo[c]->getIntPhase() != priorIntPhase[c])
                {
                    priorIntPhase[c] = surge_lfo[c]->getIntPhase();
                    endPhaseCountdown[c] = 32;
                }
                for (int p = 0; p < 3; ++p)
                    ts[p][c] = surge_lfo[c]->get_output(p);
            }
            for (int p = 0; p < 3; ++p)
                for (int i = 0; i < 4; ++i)
                    output1[p][i] = rack::simd::float_4::load(&ts[p][i * 4]);

            firstProcess = false;
        }

        float frac = 1.0 * lastStep / BLOCK_SIZE;
        lastStep++;
        auto mul = SURGE_TO_RACK_OSC_MUL;
        if (lfostorage->unipolar.val.b)
            mul = SURGE_TO_RACK_CV_MUL;
        for (int c = 0; c < nChan; c += 4)
        {
            for (int p = 0; p < 3; ++p)
            {
                rack::simd::float_4 outputI =
                    (output0[p][c / 4] * (1.0 - frac) + output1[p][c / 4] * frac) * mul;
                outputI.store(outputs[OUTPUT_MIX + p].getVoltages(c));
            }
        }

        for (int c = 0; c < nChan; ++c)
        {
            if (endPhaseCountdown[c] > 0)
            {
                endPhaseCountdown[c]--;
                outputs[OUTPUT_TRIGPHASE].setVoltage(10.0, c);
            }
            else
                outputs[OUTPUT_TRIGPHASE].setVoltage(0.0, c);
        }
    }

    void moduleSpecificSampleRateChange() override
    {
        clockProc.setSampleRate(APP->engine->getSampleRate());
    }

    json_t *makeModuleSpecificJson() override
    {
        auto fx = json_object();
        json_object_set(fx, "clockStyle", json_integer((int)clockProc.clockStyle));
        return fx;
    }

    void readModuleSpecificJson(json_t *modJ) override
    {
        auto cs = json_object_get(modJ, "clockStyle");
        if (cs)
        {
            auto csv = json_integer_value(cs);
            clockProc.clockStyle =
                static_cast<typename modules::ClockProcessor<LFO>::ClockStyle>(csv);
        }
    }

    void setWhichTemposyc(bool doRate, bool doEnv)
    {
        auto val = (doRate ? 1 : 0) | (doEnv ? 2 : 0);
        paramQuantities[WHICH_TEMPOSYNC]->setValue(val);
        if (inputs[INPUT_CLOCK_RATE].isConnected())
            activateTempoSync();
    }
};
} // namespace sst::surgext_rack::lfo
#endif
