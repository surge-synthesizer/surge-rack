#pragma once
#include "Surge.hpp"
#include "dsp/AdsrEnvelope.h"
#include "rack.hpp"
#include <cstring>

#if !RACK_V1
#include "dsp/digital.hpp"
#endif

#define NUM_ENV_PARAMS 19
template <typename TBase> struct SurgeADSR : virtual TBase {
    enum ParamIds {
        A_PARAM,
        D_PARAM,
        S_PARAM,
        R_PARAM,

        MODE_PARAM,

        A_S_PARAM,
        D_S_PARAM,
        R_S_PARAM,
        
        NUM_PARAMS
    };
    enum InputIds {
        GATE_IN,
        RETRIG_IN,

        A_CV,
        D_CV,
        S_CV,
        R_CV,
        
        NUM_INPUTS 
    };
    enum OutputIds {
        OUTPUT_ENV,
        NUM_OUTPUTS
    };
    enum LightIds { NUM_LIGHTS };


    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

#if RACK_V1    
    rack::dsp::SchmittTrigger envGateTrigger, envRetrig;
#else
    rack::SchmittTrigger envGateTrigger, envRetrig;
#endif    

#if RACK_V1
    SurgeADSR() : TBase() {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for( int i=A_PARAM; i<=R_PARAM; ++i )
            TBase::configParam(i, 0, 1, 0.5);
        TBase::configParam(MODE_PARAM, 0, 1, 0);
        TBase::configParam(A_S_PARAM,0,2,0);
        TBase::configParam(D_S_PARAM,0,2,0);
        TBase::configParam(R_S_PARAM,0,2,0);
        setupSurge();
    }
#else
    SurgeADSR() : TBase(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif
    
    inline float getParam(int id) {
#if RACK_V1
        return params[id].getValue();
#else
        return params[id].value;
#endif
    }

    inline float getInput(int id) {
#if RACK_V1
        return inputs[id].getVoltage();
#else
        return inputs[id].value;
#endif
    }

    inline void setOutput(int id, float v) {
#if RACK_V1
        outputs[id].setVoltage(v);
#else
        outputs[id].value = v;
#endif
    }


    void setupSurge() {
        std::string dataPath;
#if RACK_V1
        dataPath = rack::asset::plugin( pluginInstance, "surge-data/" );
#else
        dataPath = "";
#endif
        
        // TODO: Have a mode where these paths come from res/
        storage.reset(new SurgeStorage(dataPath));

        // FIX THIS of course
#if RACK_V1        
        float sr = rack::APP->engine->getSampleRate();
#else
        float sr = rack::engineGetSampleRate();
#endif
        samplerate = sr;
        dsamplerate = sr;
        samplerate_inv = 1.0 / sr;
        dsamplerate_inv = 1.0 / sr;
        dsamplerate_os = dsamplerate * OSC_OVERSAMPLING;
        dsamplerate_os_inv = 1.0 / dsamplerate_os;
        storage->init_tables();

        surge_envelope.reset(new AdsrEnvelope());
        adsrstorage = &(storage->getPatch().scene[0].adsr[0]);
        surge_envelope->init(storage.get(), adsrstorage, storage->getPatch().scenedata[0], nullptr );

        adsrstorage->mode.val.b = false;
        adsrstorage->a_s.val.i = 0;
        adsrstorage->d_s.val.i = 0;
        adsrstorage->r_s.val.i = 0;
    }

    virtual void onSampleRateChange() override {
        INFO( "SAMPLE RATE CHANGE" );
    }
    
    std::unique_ptr<AdsrEnvelope> surge_envelope;
    std::unique_ptr<SurgeStorage> storage;
    ADSRStorage *adsrstorage;
    std::vector<int> orderToParam;

    bool wasGated = false;
    
#if RACK_V1
    void process(const typename TBase::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if (envGateTrigger.process(getInput(GATE_IN))) {
            surge_envelope->attack();
        }
        if (envRetrig.process(getInput(RETRIG_IN))) {
            surge_envelope->retrigger();
        }

        bool gated = getInput(GATE_IN) >= 1.f;
        if( gated ) wasGated = true;
        if( wasGated && ! gated)
        {
            wasGated = false;
            surge_envelope->release();
        }

        adsrstorage->mode.val.b = (getParam(MODE_PARAM)<0.5);
        adsrstorage->a_s.val.i = (int)getParam(A_S_PARAM);
        adsrstorage->d_s.val.i = (int)getParam(D_S_PARAM);
        adsrstorage->r_s.val.i = (int)getParam(R_S_PARAM);
        
        adsrstorage->a.set_value_f01(getParam(A_PARAM) + getInput(A_CV)/10.0);
        adsrstorage->d.set_value_f01(getParam(D_PARAM) + getInput(D_CV)/10.0);
        adsrstorage->s.set_value_f01(getParam(S_PARAM) + getInput(S_CV)/10.0);
        adsrstorage->r.set_value_f01(getParam(R_PARAM) + getInput(R_CV)/10.0);

        storage->getPatch().copy_scenedata(storage->getPatch().scenedata[0], 0);
        surge_envelope->process_block();
        
        setOutput(OUTPUT_ENV, surge_envelope->get_output() * 10.0);
    }
    
};
