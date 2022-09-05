#pragma once
#include "SurgeXT.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/modulators/LFOModulationSource.h"
#include "rack.hpp"
#include <cstring>

struct SurgeLFO : virtual public SurgeModuleCommon
{
    enum ParamIds
    {
        RATE_PARAM,
        SHAPE_PARAM,
        START_PHASE_PARAM,
        MAGNITUDE_PARAM,
        DEFORM_PARAM,
        TRIGMODE_PARAM,
        UNIPOLAR_PARAM,

        DEL_PARAM,
        H_PARAM, // that order comes from LFOStorage. I agree it is wierd but it
                 // is what it is
        A_PARAM,
        D_PARAM,
        S_PARAM,
        R_PARAM,

        RATE_TS,

        DEL_TS,
        H_TS,
        A_TS,
        D_TS,
        S_TS,
        R_TS,

        NUM_PARAMS
    };
    enum InputIds
    {
        GATE_IN,
        UNUSED_ONE,

        RATE_CV,
        SHAPE_CV,
        START_PHASE_CV,
        MAGNITUDE_CV,
        DEFORM_CV,
        TRIGMODE_CV,
        UNIPOLAR_CV,

        DEL_CV,
        H_CV,
        A_CV,
        D_CV,
        S_CV,
        R_CV,

        CLOCK_CV_INPUT,

        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_ENV,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENV_LIGHT,
        NUM_LIGHTS
    };

    rack::dsp::SchmittTrigger envGateTrigger[MAX_POLY], envRetrig[MAX_POLY];

    SurgeLFO() : SurgeModuleCommon()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<SurgeRackParamQuantity>(RATE_PARAM, 0, 1, 0.2);
        configParam<SurgeRackParamQuantity>(SHAPE_PARAM, 0, 1, 0);
        configParam<SurgeRackParamQuantity>(START_PHASE_PARAM, 0, 1, 0);
        configParam<SurgeRackParamQuantity>(MAGNITUDE_PARAM, 0, 1, 1);
        configParam<SurgeRackParamQuantity>(DEFORM_PARAM, 0, 1, 0.5);
        configParam<SurgeRackParamQuantity>(TRIGMODE_PARAM, 0, 1, 0);
        configParam<SurgeRackParamQuantity>(UNIPOLAR_PARAM, 0, 1, 0);

        configParam<SurgeRackParamQuantity>(DEL_PARAM, 0, 1, 0);
        configParam<SurgeRackParamQuantity>(A_PARAM, 0, 1, 0.2);
        configParam<SurgeRackParamQuantity>(H_PARAM, 0, 1, 0.1);
        configParam<SurgeRackParamQuantity>(D_PARAM, 0, 1, 0.2);
        configParam<SurgeRackParamQuantity>(S_PARAM, 0, 1, 0.7);
        configParam<SurgeRackParamQuantity>(R_PARAM, 0, 1, 0.3);

        for (int i = RATE_TS; i <= R_TS; ++i)
        {
            configParam<SurgeRackParamQuantity>(i, 0, 1, 0, "Activate TempoSync");
        }

        setupSurge();
    }

    virtual std::string getName() override { return "LFO"; }

    virtual void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS);

        surge_lfo.resize(MAX_POLY);
        for (int i = 0; i < MAX_POLY; ++i)
            surge_lfo[i].reset(new LFOModulationSource());

        surge_ss.reset(new StepSequencerStorage());
        surge_ms.reset(new MSEGStorage());
        surge_fs.reset(new FormulaModulatorStorage());

        lfostorage = &(storage->getPatch().scene[0].lfo[0]);

        for (int i = 0; i < MAX_POLY; ++i)
            surge_lfo[i]->assign(storage.get(), lfostorage, storage->getPatch().scenedata[0],
                                 nullptr, surge_ss.get(), surge_ms.get(), surge_fs.get());

        Parameter *p0 = &(lfostorage->rate);
        for (int i = RATE_PARAM; i <= R_PARAM; ++i)
        {
            p0->temposync = false;
            pb[i] = std::shared_ptr<SurgeRackParamBinding>(
                new SurgeRackParamBinding(p0, i, RATE_CV + (i - RATE_PARAM)));
            pb[i]->setDeactivationAlways(false);
            p0++;
        }
        pb[RATE_PARAM]->setTemposync(RATE_TS, true);

        int tsSpread = DEL_TS - DEL_PARAM;
        for (int i = DEL_PARAM; i <= R_PARAM; ++i)
        {
            if (i == S_PARAM)
                tsSpread--;
            else
                pb[i]->setTemposync(i + tsSpread, false);
        }

        setupStorageRanges(&(lfostorage->rate), &(lfostorage->release));
        pc.resize(NUM_PARAMS);

        lastStep = BLOCK_SIZE;

        for (int i = 0; i < MAX_POLY; ++i)
        {
            wasGated[i] = true;
            wasGateConnected[i] = false;
        }

        for (int i = 0; i < 4; ++i)
        {
            output0[i] = rack::simd::float_4::zero();
            output1[i] = rack::simd::float_4::zero();
        }
    }

    std::vector<std::unique_ptr<LFOModulationSource>> surge_lfo;
    std::unique_ptr<StepSequencerStorage> surge_ss;
    std::unique_ptr<MSEGStorage> surge_ms;
    std::unique_ptr<FormulaModulatorStorage> surge_fs;
    LFOStorage *lfostorage;

    bool wasGated[MAX_POLY], wasGateConnected[MAX_POLY]; // assume we run open
    int lastStep = BLOCK_SIZE;
    rack::simd::float_4 output0[4], output1[4];
    int lastNChan = -1;

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[GATE_IN].getChannels());
        outputs[OUTPUT_ENV].setChannels(nChan);
        if (nChan != lastNChan)
        {
            lastNChan = nChan;
            for (int i = nChan; i < MAX_POLY; ++i)
                lastStep = BLOCK_SIZE;
        }

        if (inputConnected(CLOCK_CV_INPUT))
        {
            updateBPMFromClockCV(inputs[CLOCK_CV_INPUT].getVoltage(), args.sampleTime,
                                 args.sampleRate);
        }
        else
        {
            if (lastBPM == -1)
                updateBPMFromClockCV(1, args.sampleTime, args.sampleRate);
        }

        if (lastStep == BLOCK_SIZE)
            lastStep = 0;

        if (lastStep == 0)
        {
            for (int i = 0; i < 4; ++i)
                output0[i] = output1[i];

            float ts[16];

            for (int c = 0; c < nChan; ++c)
            {
                bool inNewAttack = false;
                if (inputConnected(GATE_IN) &&
                    envGateTrigger[c].process(inputs[GATE_IN].getVoltage(c)))
                {
                    lfostorage->trigmode.val.i = lm_keytrigger;
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    surge_lfo[c]->attack();
                    inNewAttack = true;
                }

                /*
                if (inputConnected(RETRIG_IN) &&
                envRetrig[c].process(inputs[RETRIG_IN].getPolyVoltage(c))) {
                    // This is wrong
                    surge_lfo[c]->retrigger_FEG = true;
                    surge_lfo[c]->retrigger_AEG = true;
                }
                */

                for (auto binding : pb)
                    if (binding)
                        binding->update(pc, c, this);

                /*
                ** OK so now there's a couple of things in the gate state
                **
                ** wasGated -> was the last step gated
                ** isGateConnected -> is there anyone hooked up to the gate
                *input
                ** isGated  -> Is the gate input true
                */

                bool isGateConnected = inputConnected(GATE_IN);
                bool isGated = inputs[GATE_IN].getVoltage(c) >= 1.f;

                if (isGateConnected)
                {
                    if (c == 0)
                        setLight(ENV_LIGHT, 10.0);
                    /*
                    ** We have to undertake no action if:
                    **   isGateConnected && wasGated && isGated -> hooked up and
                    *open still
                    **   isGateConnected && !wasGated && !isGated -> hooked up
                    *and closed still
                    */
                    if (isGated /* && !wasGated */)
                        wasGated[c] = true;
                    if (wasGated[c] && !isGated)
                    {
                        wasGated[c] = false;
                        surge_lfo[c]->release();
                    }
                    wasGateConnected[c] = true;
                }
                else
                {
                    if (c == 0)
                        setLight(ENV_LIGHT, 0.0);
                    /*
                    ** In this case we want to act as if we are always gated.
                    ** So if wasGated is false we need to attack and never
                    *release.
                    */
                    // Trickily we want to verride sustain here
                    lfostorage->sustain.set_value_f01(1.0);

                    if (!wasGated[c])
                    {
                        lfostorage->trigmode.val.i = lm_freerun;

                        copyScenedataSubset(0, storage_id_start, storage_id_end);
                        surge_lfo[c]->attack();
                        inNewAttack = true;
                        wasGated[c] = true;
                    }
                    wasGateConnected[c] = false;
                }

                copyScenedataSubset(0, storage_id_start, storage_id_end);
                surge_lfo[c]->process_block();
                if (inNewAttack)
                {
                    // Do the painful thing in the infrequent case
                    output0[c / 4].s[c % 4] = surge_lfo[c]->get_output(0);
                    surge_lfo[c]->process_block();
                }
                ts[c] = surge_lfo[c]->get_output(0);
            }
            for (int i = 0; i < 4; ++i)
                output1[i] = rack::simd::float_4::load(ts + i * 4);

            pc.update(this);
        }

        float frac = 1.0 * lastStep / BLOCK_SIZE;
        lastStep++;
        for (int c = 0; c < nChan; c += 4)
        {
            rack::simd::float_4 outputI =
                (output0[c / 4] * (1.0 - frac) + output1[c / 4] * frac) * SURGE_TO_RACK_OSC_MUL;
            outputI.store(outputs[OUTPUT_ENV].getVoltages(c));
        }
    }
};
