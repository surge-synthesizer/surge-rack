#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/AdsrEnvelope.h"
#include "rack.hpp"
#include <cstring>

#if !RACK_V1
#include "dsp/digital.hpp"
#endif

#define NUM_ENV_PARAMS 19
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
    enum OutputIds { OUTPUT_ENV, NUM_OUTPUTS };
    enum LightIds {
        DIGI_LIGHT,

        NUM_LIGHTS
    };

#if RACK_V1
    rack::dsp::SchmittTrigger envGateTrigger, envRetrig;
#else
    rack::SchmittTrigger envGateTrigger, envRetrig;
#endif

    StringCache adsrStrings[4];
    ParamCache pc;

#if RACK_V1
    SurgeADSR() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = A_PARAM; i <= R_PARAM; ++i)
            configParam(i, 0, 1, 0.5);
        configParam(MODE_PARAM, 0, 1, 0);
        configParam(A_S_PARAM, 0, 2, 0);
        configParam(D_S_PARAM, 0, 2, 0);
        configParam(R_S_PARAM, 0, 2, 0);
        setupSurge();
    }
#else
    SurgeADSR()
        : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    virtual void setupSurge() {
        setupSurgeCommon();

        surge_envelope.reset(new AdsrEnvelope());
        adsrstorage = &(storage->getPatch().scene[0].adsr[0]);
        surge_envelope->init(storage.get(), adsrstorage,
                             storage->getPatch().scenedata[0], nullptr);

        adsrstorage->mode.val.b = false;

        adsrstorage->a.temposync = false;
        adsrstorage->d.temposync = false;
        adsrstorage->s.temposync = false;
        adsrstorage->r.temposync = false;

        adsrstorage->a_s.val.i = 0;
        adsrstorage->d_s.val.i = 0;
        adsrstorage->r_s.val.i = 0;

        setupStorageRanges(&(adsrstorage->a), &(adsrstorage->mode));
        pc.resize(NUM_PARAMS);
    }

    std::unique_ptr<AdsrEnvelope> surge_envelope;
    ADSRStorage *adsrstorage;

    bool wasGated = false;
    int lastStep = 0;
    float output0, output1;
    
#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if (lastStep == 32)
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

            for (int i = 0; i < 4; ++i) {
                Parameter *adsr = &(adsrstorage->a);
                Parameter *p = &(adsr[i]);
                if (pc.changed(A_PARAM + i, this)) {
                    char txt[1024];
                    p->set_value_f01(getParam(A_PARAM + i));
                    p->get_display(txt, false, 0);
                    adsrStrings[i].reset(txt);
                }
            }
            pc.update(this);

            bool gated = getInput(GATE_IN) >= 1.f;
            if (gated)
                wasGated = true;
            if (wasGated && !gated) {
                wasGated = false;
                surge_envelope->release();
            }

            setLight(DIGI_LIGHT, (getParam(MODE_PARAM) > 0.5) ? 1.0 : 0);

            adsrstorage->mode.val.b = (getParam(MODE_PARAM) < 0.5);
            adsrstorage->a_s.val.i = (int)getParam(A_S_PARAM);
            adsrstorage->d_s.val.i = (int)getParam(D_S_PARAM);
            adsrstorage->r_s.val.i = (int)getParam(R_S_PARAM);

            adsrstorage->a.set_value_f01(getParam(A_PARAM) +
                                         getInput(A_CV) / 10.0);
            adsrstorage->d.set_value_f01(getParam(D_PARAM) +
                                         getInput(D_CV) / 10.0);
            adsrstorage->s.set_value_f01(getParam(S_PARAM) +
                                         getInput(S_CV) / 10.0);
            adsrstorage->r.set_value_f01(getParam(R_PARAM) +
                                         getInput(R_CV) / 10.0);

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
        float frac = lastStep / 32.0;
        float outputI = output0 * (1.0-frac) + output1 * frac;
        setOutput(OUTPUT_ENV, outputI * 10.0);
    }
};
