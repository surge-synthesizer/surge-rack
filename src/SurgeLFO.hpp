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

    rack::dsp::SchmittTrigger envGateTrigger[MAX_POLY], envRetrig[MAX_POLY];


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

        surge_lfo.resize(MAX_POLY);
        for( int i=0; i<MAX_POLY; ++i )
            surge_lfo[i].reset(new LfoModulationSource());
        
        surge_ss.reset(new StepSequencerStorage());
        
        lfostorage = &(storage->getPatch().scene[0].lfo[0]);

        for( int i=0; i<MAX_POLY; ++i )
            surge_lfo[i]->assign(storage.get(), lfostorage,
                                 storage->getPatch().scenedata[0], nullptr, surge_ss.get());

        Parameter *p0 = &(lfostorage->rate);
        for( int i=RATE_PARAM; i<= R_PARAM; ++i )
        {
            p0->temposync = false;
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

        for( int i=0; i<MAX_POLY; ++i )
        {
            lastStep[i] = BLOCK_SIZE;
            wasGated[i]= true;
            wasGateConnected[i] = false;
        }
    }

    std::vector<std::unique_ptr<LfoModulationSource>> surge_lfo;
    std::unique_ptr<StepSequencerStorage> surge_ss;
    LFOStorage *lfostorage;

    bool wasGated[MAX_POLY], wasGateConnected[MAX_POLY]; // assume we run open
    int lastStep[MAX_POLY];
    float output0[MAX_POLY], output1[MAX_POLY];
    int lastNChan = -1;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[GATE_IN].getChannels());
        outputs[OUTPUT_ENV].setChannels(nChan);
        if( nChan != lastNChan )
        {
            lastNChan = nChan;
            for( int i=nChan; i < MAX_POLY; ++i )
                lastStep[i] = BLOCK_SIZE;
        }
        
        if( inputConnected(CLOCK_CV_INPUT) )
        {
            updateBPMFromClockCV(inputs[CLOCK_CV_INPUT].getVoltage(), args.sampleTime, args.sampleRate );
        }
        else
        {
            // FIXME - only once please
            updateBPMFromClockCV(1, args.sampleTime, args.sampleRate );
        }

        for( int c=0; c<nChan; ++c)
        {
            if (lastStep[c] == BLOCK_SIZE)
                lastStep[c] = 0;
            
            if (lastStep[c] == 0) {
                bool inNewAttack = false;
                if (inputConnected(GATE_IN) && envGateTrigger[c].process(inputs[GATE_IN].getVoltage(c))) {
                    lfostorage->trigmode.val.i = lm_keytrigger;
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    surge_lfo[c]->attack();
                    inNewAttack = true;
                }
                
                if (inputConnected(RETRIG_IN) && envRetrig[c].process(inputs[RETRIG_IN].getPolyVoltage(c))) {
                    surge_lfo[c]->retrigger_EG = true;
                }
                

                for(auto binding : pb)
                    if(binding)
                        binding->update(pc, c, this);
            
                /*
                ** OK so now there's a couple of things in the gate state
                **
                ** wasGated -> was the last step gated
                ** isGateConnected -> is there anyone hooked up to the gate input 
                ** isGated  -> Is the gate input true
                */
                
                bool isGateConnected = inputConnected(GATE_IN);
                bool isGated = inputs[GATE_IN].getVoltage(c) >= 1.f;
                
                if( isGateConnected )
                {
                    if( c == 0 )
                        setLight(ENV_LIGHT, 10.0);
                    /*
                    ** We have to undertake no action if:
                    **   isGateConnected && wasGated && isGated -> hooked up and open still
                    **   isGateConnected && !wasGated && !isGated -> hooked up and closed still
                    */
                    if( isGated /* && !wasGated */ )
                        wasGated[c] = true;
                    if( wasGated[c] && !isGated )
                    {
                        wasGated[c] = false;
                        surge_lfo[c]->release();
                    }
                    wasGateConnected[c] = true;
                }
                else
                {
                    if( c == 0 )
                        setLight(ENV_LIGHT, 0.0);
                    /*
                    ** In this case we want to act as if we are always gated.
                    ** So if wasGated is false we need to attack and never release.
                    */
                    // Trickily we want to verride sustain here
                    lfostorage->sustain.set_value_f01(1.0);
                    
                    if( ! wasGated[c] )
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
                if( inNewAttack )
                {
                    output0[c] = surge_lfo[c]->get_output();
                    surge_lfo[c]->process_block();
                    output1[c] = surge_lfo[c]->get_output();
                }
                else
                {
                    output0[c] = output1[c];
                    output1[c] = surge_lfo[c]->get_output();
                }
                
            }
            
            float frac = 1.0 * lastStep[c] / BLOCK_SIZE;
            float outputI = output0[c] * (1.0-frac) + output1[c] * frac;
            outputs[OUTPUT_ENV].setVoltage(outputI * SURGE_TO_RACK_OSC_MUL, c);
            lastStep[c]++;
        }
        pc.update(this);
    }
};
