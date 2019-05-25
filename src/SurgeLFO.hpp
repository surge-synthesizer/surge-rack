#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/LfoModulationSource.h"
#include "rack.hpp"
#include <cstring>

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
        H_PARAM,  // that order comes from LFOStorage. I agree it is wierd but it is what it is
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
        H_CV,
        A_CV,
        D_CV,
        S_CV,
        R_CV,

        CLOCK_CV_INPUT,
        
        NUM_INPUTS
    };
    enum OutputIds { OUTPUT_ENV, NUM_OUTPUTS };
    enum LightIds {
        ENV_LIGHT,
        NUM_LIGHTS
    };

    rack::dsp::SchmittTrigger envGateTrigger, envRetrig;


    SurgeLFO() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<SurgeRackParamQuantity>(RATE_PARAM,0,1,0.2);
        configParam<SurgeRackParamQuantity>(SHAPE_PARAM,0,1,0);
        configParam<SurgeRackParamQuantity>(START_PHASE_PARAM,0,1,0);
        configParam<SurgeRackParamQuantity>(MAGNITUDE_PARAM,0,1,1);
        configParam<SurgeRackParamQuantity>(DEFORM_PARAM,0,1,0.5);
        configParam<SurgeRackParamQuantity>(TRIGMODE_PARAM,0,1,0);
        configParam<SurgeRackParamQuantity>(UNIPOLAR_PARAM,0,1,0);
        
        configParam<SurgeRackParamQuantity>(DEL_PARAM,0,1,0);
        configParam<SurgeRackParamQuantity>(A_PARAM,0,1,0.2);
        configParam<SurgeRackParamQuantity>(H_PARAM,0,1,0.1);
        configParam<SurgeRackParamQuantity>(D_PARAM,0,1,0.2);
        configParam<SurgeRackParamQuantity>(S_PARAM,0,1,0.7);
        configParam<SurgeRackParamQuantity>(R_PARAM,0,1,0.3);

        for( int i=RATE_TS; i<=R_TS; ++i )
        {
            configParam<SurgeRackParamQuantity>(i,0,1,0, "Activate TempoSync" );
        }
        
        setupSurge();
    }

    virtual std::string getName() override { return "LFO"; }
    
    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);

        surge_lfo.reset(new LfoModulationSource());
        surge_ss.reset(new StepSequencerStorage());
        
        lfostorage = &(storage->getPatch().scene[0].lfo[0]);

        surge_lfo->assign(storage.get(), lfostorage,
                          storage->getPatch().scenedata[0], nullptr, surge_ss.get());

        Parameter *p0 = &(lfostorage->rate);
        for( int i=RATE_PARAM; i<= R_PARAM; ++i )
        {
            p0->temposync = false;
            rack::INFO("Setting shared ptr at %d size=%d", i, pb.size() );
            pb[i] = std::shared_ptr<SurgeRackParamBinding>(new SurgeRackParamBinding(p0, i, RATE_CV + (i-RATE_PARAM)));
            p0++;
        }
        pb[RATE_PARAM]->setTemposync(RATE_TS, true);

        int tsSpread = DEL_TS - DEL_PARAM;
        for( int i=DEL_PARAM; i<=R_PARAM; ++i )
        {
            if( i == S_PARAM ) tsSpread--;
            else pb[i]->setTemposync(i+tsSpread, false);
        }
        
        setupStorageRanges(&(lfostorage->rate), &(lfostorage->release));
        pc.resize(NUM_PARAMS);
    }

    std::unique_ptr<LfoModulationSource> surge_lfo;
    std::unique_ptr<StepSequencerStorage> surge_ss;
    LFOStorage *lfostorage;

    bool wasGated = true, wasGateConnected=false; // assume we run open
    int lastStep = 0;
    float output0, output1;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (lastStep == BLOCK_SIZE)
            lastStep = 0;

        if (lastStep == 0) {
            bool inNewAttack = false;
            if (inputConnected(GATE_IN) && envGateTrigger.process(getInput(GATE_IN))) {
                lfostorage->trigmode.val.i = lm_keytrigger;
                copyScenedataSubset(0, storage_id_start, storage_id_end);
                surge_lfo->attack();
                inNewAttack = true;
            }

            if (inputConnected(RETRIG_IN) && envRetrig.process(getInput(RETRIG_IN))) {
                surge_lfo->retrigger_EG = true;
            }

            if( inputConnected(CLOCK_CV_INPUT) )
            {
                updateBPMFromClockCV(getInput(CLOCK_CV_INPUT), args.sampleTime, args.sampleRate );
            }
            else
            {
                // FIXME - only once please
                updateBPMFromClockCV(1, args.sampleTime, args.sampleRate );
            }
            

            for(auto binding : pb)
                if(binding)
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
                setLight(ENV_LIGHT, 10.0);
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
                wasGateConnected = true;
            }
            else
            {
                setLight(ENV_LIGHT, 0.0);
                /*
                ** In this case we want to act as if we are always gated.
                ** So if wasGated is false we need to attack and never release.
                */
                // Trickily we want to verride sustain here
                lfostorage->sustain.set_value_f01(1.0);
                
                if( ! wasGated )
                {
                    lfostorage->trigmode.val.i = lm_freerun;

                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    surge_lfo->attack();
                    inNewAttack = true;
                    wasGated = true;
                }
                wasGateConnected = false;
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
        setOutput(OUTPUT_ENV, outputI * SURGE_TO_RACK_OSC_MUL);
    }
};
