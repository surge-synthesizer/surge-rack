#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/AdsrEnvelope.h"
#include "rack.hpp"
#include <cstring>
#include <simd/vector.hpp>


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
            pb[i]->setDeactivationAlways(false);
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
        lastStep = BLOCK_SIZE;

        for( int i=0; i<4; ++i )
        {
            output0[i] = rack::simd::float_4::zero();
            output1[i] = rack::simd::float_4::zero();
        }
    }

    std::vector<std::unique_ptr<AdsrEnvelope>> surge_envelopes;
    ADSRStorage *adsrstorage;

    std::vector<bool> wasGated, everGated;
    int lastStep;
    rack::simd::float_4 output0[4], output1[4];
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[GATE_IN].getChannels() );
        outputs[OUTPUT_ENV].setChannels(nChan);
        
        if( lastStep == 0 )
        {
            if( inputConnected(CLOCK_CV_INPUT) )
            {
                updateBPMFromClockCV(inputs[CLOCK_CV_INPUT].getVoltage(), args.sampleTime, args.sampleRate );
            }
            else
            {
                if( lastBPM == -1 )
                    updateBPMFromClockCV(1, args.sampleTime, args.sampleRate );
            }
        }

        setLight(DIGI_LIGHT, (getParam(MODE_PARAM) > 0.5) ? 1.0 : 0);

        if (lastStep == BLOCK_SIZE)
            lastStep = 0;
        
        if (lastStep == 0) {
            // copy output1 onto output0
            for( int i=0; i<4; ++i)
                output0[i] = output1[i];

            float ts[16];
            memset(ts, 0, 16 * sizeof(float));
            
            for( int i=0; i<nChan; ++i )
            {
                bool inNewAttack = false;
                if (envGateTrigger[i].process(inputs[GATE_IN].getVoltage(i))) {
                    surge_envelopes[i]->attack();
                    everGated[i] = true;
                    inNewAttack = true;
                }

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

                for(auto binding : pb)
                    if(binding)
                        binding->update(pc, i, this);

                copyScenedataSubset(0, storage_id_start, storage_id_end);

                if( everGated[i] )
                {
                    surge_envelopes[i]->process_block();
                    
                    if( inNewAttack )
                    {
                        // This is infrequent so do the painful direct addressing
                        output0[i/4].s[i%4] = surge_envelopes[i]->get_output();
                        surge_envelopes[i]->process_block();
                    }
                    
                    ts[i] = surge_envelopes[i]->get_output();
                }
            } // end channel loop

            for( int i=0; i<4; ++i )
                output1[i] = rack::simd::float_4::load(ts + i * 4);

            pc.update(this);
        }

        float frac = 1.0 * lastStep / BLOCK_SIZE;
        lastStep ++;
        for( int i=0; i<nChan; i += 4 )
        {
            rack::simd::float_4 outputI = ( output0[i/4] * (1.0-frac) + output1[i/4] * frac ) * 10.0;
            outputI.store(outputs[OUTPUT_ENV].getVoltages(i));
        }
    }
};
