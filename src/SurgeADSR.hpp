#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/AdsrEnvelope.h"
#include "rack.hpp"
#include <cstring>


struct SurgeADSR : virtual public SurgeModuleCommon {
    enum ParamIds {
        A_PARAM,
        D_PARAM,
        S_PARAM,
        R_PARAM,

        MODE_PARAM,

        A_S_PARAM,
        D_S_PARAM,
        R_S_PARAM,

        A_TEMPOSYNC,
        D_TEMPOSYNC,
        R_TEMPOSYNC,

        NUM_PARAMS
    };
    enum InputIds {
        GATE_IN,
        RETRIG_IN,

        A_CV,
        D_CV,
        S_CV,
        R_CV,

        CLOCK_CV_INPUT,

        NUM_INPUTS
    };
    enum OutputIds { OUTPUT_ENV, NUM_OUTPUTS };
    enum LightIds {
        DIGI_LIGHT,
        POLY_CHAN_LIGHT,
        NUM_LIGHTS
    };

    rack::dsp::SchmittTrigger envGateTrigger[MAX_POLY], envRetrig[MAX_POLY];

    SurgeADSR() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = A_PARAM; i <= R_PARAM; ++i)
            configParam<SurgeRackParamQuantity>(i, 0, 1, 0.5);
        configParam<SurgeRackParamQuantity>(MODE_PARAM, 0, 1, 0);
        configParam<SurgeRackParamQuantity>(A_S_PARAM, 0, 2, 0);
        configParam<SurgeRackParamQuantity>(D_S_PARAM, 0, 2, 0);
        configParam<SurgeRackParamQuantity>(R_S_PARAM, 0, 2, 0);

        for( int i= A_TEMPOSYNC; i <= R_TEMPOSYNC; ++i )
            configParam<SurgeRackParamQuantity>(i, 0, 1, 0 );
        setupSurge();
    }

    virtual std::string getName() override { return "ADSR"; }

    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);

        surge_envelopes.resize(MAX_POLY);
        adsrstorage = &(storage->getPatch().scene[0].adsr[0]);

        for( int i=0; i<MAX_POLY; ++i )
        {
            surge_envelopes[i].reset(new AdsrEnvelope());
            surge_envelopes[i]->init(storage.get(), adsrstorage,
                                 storage->getPatch().scenedata[0], nullptr);
        }

        adsrstorage->mode.val.b = false;

        setupStorageRanges(&(adsrstorage->a), &(adsrstorage->mode));

        /*
        ** I ordered params differently than storage so need 3 loops
        */
        Parameter *p0 = &(adsrstorage->a);
        for( int i=A_PARAM; i<=R_PARAM; ++i )
        {
            p0->temposync = false;
            pb[i] = std::shared_ptr<SurgeRackParamBinding>(new SurgeRackParamBinding(p0, i, A_CV + ( i - A_PARAM ) ) );
            if( i != S_PARAM )
            {
                pb[i]->setTemposync(i + A_TEMPOSYNC - A_PARAM - ( i == R_PARAM ? 1 : 0 ), false );
            }
            p0++;
        }

        p0 = &(adsrstorage->mode);
        pb[MODE_PARAM] = std::shared_ptr<SurgeRackParamBinding>(new SurgeRackParamBinding(SurgeRackParamBinding::BOOL_NOT,
                                                                                          p0, MODE_PARAM));
        
        p0 = &(adsrstorage->a_s);
        for( int i=A_S_PARAM; i<R_S_PARAM; ++i )
        {
            pb[i] = std::shared_ptr<SurgeRackParamBinding>(new SurgeRackParamBinding(SurgeRackParamBinding::INT,
                                                                                     p0, i));
            p0++;
        }
        
        wasGated.resize(MAX_POLY); std::fill(wasGated.begin(), wasGated.end(), false );
        everGated.resize(MAX_POLY); std::fill(everGated.begin(), everGated.end(), false );
        lastStep.resize(MAX_POLY); std::fill(lastStep.begin(), lastStep.end(), 0 );
        output0.resize(MAX_POLY); std::fill(output0.begin(), output0.end(), 0 );
        output1.resize(MAX_POLY); std::fill(output1.begin(), output1.end(), 0 );
    }

    std::vector<std::unique_ptr<AdsrEnvelope>> surge_envelopes;
    ADSRStorage *adsrstorage;

    std::vector<bool> wasGated, everGated;
    std::vector<int> lastStep;
    std::vector<float> output0, output1;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[GATE_IN].getChannels() );
        lights[POLY_CHAN_LIGHT].setBrightness(nChan);
        outputs[OUTPUT_ENV].setChannels(nChan);
        
        if( lastStep[0] == 0 )
        {
            if( inputConnected(CLOCK_CV_INPUT) )
            {
                updateBPMFromClockCV(getInput(CLOCK_CV_INPUT), args.sampleTime, args.sampleRate );
            }
            else
            {
                // FIXME - only once please
                updateBPMFromClockCV(1, args.sampleTime, args.sampleRate );
            }
        }


        for( int i=0; i<nChan; ++i )
        {
            if (lastStep[i] == BLOCK_SIZE)
                lastStep[i] = 0;
            
            bool inNewAttack = false;
            if (envGateTrigger[i].process(inputs[GATE_IN].getVoltage(i))) {
                lastStep[i] = 0;
                surge_envelopes[i]->attack();
                everGated[i] = true;
                inNewAttack = true;
            }
            
            if (lastStep[i] == 0) {
                if (envRetrig[i].process(inputs[RETRIG_IN].getPolyVoltage(i))) {
                    surge_envelopes[i]->retrigger();
                }
                
                bool gated = inputs[GATE_IN].getVoltage(i) >= 1.f;
                if (gated)
                    wasGated[i] = true;
                if (wasGated[i] && !gated) {
                    wasGated[i] = false;
                    surge_envelopes[i]->release();
                }
                
                setLight(DIGI_LIGHT, (getParam(MODE_PARAM) > 0.5) ? 1.0 : 0);
                
                for(auto binding : pb)
                    if(binding)
                        binding->update(pc, i, this);
                pc.update(this);
                copyScenedataSubset(0, storage_id_start, storage_id_end);
                
                surge_envelopes[i]->process_block();
                
                if( inNewAttack )
                {
                    output0[i] = surge_envelopes[i]->get_output();
                    surge_envelopes[i]->process_block();
                    output1[i] = surge_envelopes[i]->get_output();
                }
                else
                {
                    output0[i] = output1[i];
                    output1[i] = surge_envelopes[i]->get_output();
                }
                
            }

            lastStep[i]++;
            float frac = 1.0 * lastStep[i] / BLOCK_SIZE;
            float outputI = output0[i] * (1.0-frac) + output1[i] * frac;
            if( ! everGated[i] ) outputI = 0.f;
            
            outputs[OUTPUT_ENV].setVoltage(outputI * 10.0, i);
        }
    }
};
