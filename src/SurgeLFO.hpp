#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/LfoModulationSource.h"
#include "rack.hpp"
#include <cstring>

#if !RACK_V1
#include "dsp/digital.hpp"
#endif

struct SurgeLFO : virtual public SurgeModuleCommon {
    enum ParamIds {
        RATE_PARAM,
        SHAPE_PARAM,
        START_PHASE_PARAM,
        MAGNITUDE_PARAM,
        DEFORM_PARAM,
        TRIGMODE_PARAM,
        UNIPOLAR_PARAM,
        
        DEL_PARAM,
        A_PARAM,
        H_PARAM,
        D_PARAM,
        S_PARAM,
        R_PARAM,

        NUM_PARAMS
    };
    enum InputIds {
        GATE_IN,
        RETRIG_IN,

        RATE_CV,
        SHAPE_CV,
        START_PHASE_CV,
        MAGNITUDE_CV,
        DEFORM_CV,
        TRIGMODE_CV,
        UNIPOLAR_CV,
        
        DEL_CV,
        A_CV,
        H_CV,
        D_CV,
        S_CV,
        R_CV,

        NUM_INPUTS
    };
    enum OutputIds { OUTPUT_ENV, NUM_OUTPUTS };
    enum LightIds {

        NUM_LIGHTS
    };

#if RACK_V1
    rack::dsp::SchmittTrigger envGateTrigger, envRetrig;
#else
    rack::SchmittTrigger envGateTrigger, envRetrig;
#endif

    ParamCache pc;

    std::vector<std::shared_ptr<RackSurgeParamBinding>> pb;
    
#if RACK_V1
    SurgeLFO() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = DEL_PARAM; i <= R_PARAM; ++i)
            configParam(i, 0, 1, 0.5);
        setupSurge();
    }
#else
    SurgeLFO()
        : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    
    virtual void setupSurge() {
        setupSurgeCommon();

        surge_lfo.reset(new LfoModulationSource());
        surge_ss.reset(new StepSequencerStorage());
        lfostorage = &(storage->getPatch().scene[0].lfo[0]);
        surge_lfo->assign(storage.get(), lfostorage,
                          storage->getPatch().scenedata[0], nullptr, surge_ss.get());

        Parameter *p0 = &(lfostorage->rate);
        for( int i=RATE_PARAM; i<= R_PARAM; ++i )
        {
            p0->temposync = false;
            pb.push_back(std::shared_ptr<RackSurgeParamBinding>(new RackSurgeParamBinding(p0, i, RATE_CV + (i-RATE_PARAM))));
            p0++;
        }
           
        setupStorageRanges(&(lfostorage->rate), &(lfostorage->release));
        pc.resize(NUM_PARAMS);
    }

    std::unique_ptr<LfoModulationSource> surge_lfo;
    std::unique_ptr<StepSequencerStorage> surge_ss;
    LFOStorage *lfostorage;

    bool wasGated = true; // assume we run open
    int lastStep = 0;
    float output0, output1;
    
#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if (lastStep == BLOCK_SIZE)
            lastStep = 0;

        if (lastStep == 0) {
            bool inNewAttack = false;
            if (inputConnected(GATE_IN) && envGateTrigger.process(getInput(GATE_IN))) {
                copyScenedataSubset(0, storage_id_start, storage_id_end);
                surge_lfo->attack();
                inNewAttack = true;
            }

            if (inputConnected(RETRIG_IN) && envRetrig.process(getInput(RETRIG_IN))) {
                surge_lfo->retrigger_EG = true;
            }

            for(auto binding : pb)
                binding->update(pc, this);
            
            pc.update(this);

            /*
            ** OK so now there's a couple of things in the gate state
            **
            ** wasGated -> was the last step gated
            ** isGateConnected -> is there anyone hooked up to the gate input 
            ** isGated  -> Is the gate input true
            */

            bool isGateConnected = inputConnected(GATE_IN);
            bool isGated = getInput(GATE_IN) >= 1.f;

            if( isGateConnected )
            {
                /*
                ** We have to undertake no action if:
                **   isGateConnected && wasGated && isGated -> hooked up and open still
                **   isGateConnected && !wasGated && !isGated -> hooked up and closed still
                */
                if( isGated /* && !wasGated */ )
                    wasGated = true;
                if( wasGated && !isGated )
                {
                    wasGated = false;
                    surge_lfo->release();
                }
            }
            else
            {
                /*
                ** In this case we want to act as if we are always gated.
                ** So if wasGated is false we need to attack and never release.
                */
                if( ! wasGated )
                {
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    surge_lfo->attack();
                    inNewAttack = true;
                    wasGated = true;
                }
            }

            copyScenedataSubset(0, storage_id_start, storage_id_end);
            surge_lfo->process_block();
            if( inNewAttack )
            {
                output0 = surge_lfo->get_output();
                surge_lfo->process_block();
                output1 = surge_lfo->get_output();
            }
            else
            {
                output0 = output1;
                output1 = surge_lfo->get_output();
            }
                            
        }

        lastStep++;
        float frac = 1.0 * lastStep / BLOCK_SIZE;
        float outputI = output0 * (1.0-frac) + output1 * frac;
        setOutput(OUTPUT_ENV, outputI * 10.0);
    }
};
