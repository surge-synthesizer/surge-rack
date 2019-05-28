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

        NUM_LIGHTS
    };

    rack::dsp::SchmittTrigger envGateTrigger, envRetrig;

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

        surge_envelope.reset(new AdsrEnvelope());
        adsrstorage = &(storage->getPatch().scene[0].adsr[0]);
        surge_envelope->init(storage.get(), adsrstorage,
                             storage->getPatch().scenedata[0], nullptr);

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
        
    }

    std::unique_ptr<AdsrEnvelope> surge_envelope;
    ADSRStorage *adsrstorage;

    bool wasGated = false;
    int lastStep = 0;
    float output0, output1;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (lastStep == BLOCK_SIZE)
            lastStep = 0;

        bool inNewAttack = false;
        if (envGateTrigger.process(getInput(GATE_IN))) {
            lastStep = 0;
            surge_envelope->attack();
            inNewAttack = true;
        }

        if (lastStep == 0) {
            if (envRetrig.process(getInput(RETRIG_IN))) {
                surge_envelope->retrigger();
            }

            bool gated = getInput(GATE_IN) >= 1.f;
            if (gated)
                wasGated = true;
            if (wasGated && !gated) {
                wasGated = false;
                surge_envelope->release();
            }

            setLight(DIGI_LIGHT, (getParam(MODE_PARAM) > 0.5) ? 1.0 : 0);

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
            copyScenedataSubset(0, storage_id_start, storage_id_end);
            
            surge_envelope->process_block();
            
            if( inNewAttack )
            {
                output0 = surge_envelope->get_output();
                surge_envelope->process_block();
                output1 = surge_envelope->get_output();
            }
            else
            {
                output0 = output1;
                output1 = surge_envelope->get_output();
            }
                            
        }

        lastStep++;
        float frac = 1.0 * lastStep / BLOCK_SIZE;
        float outputI = output0 * (1.0-frac) + output1 * frac;
        setOutput(OUTPUT_ENV, outputI * 10.0);
    }
};
