#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <cstring>

struct SurgeClock : virtual public SurgeModuleCommon {
    enum ParamIds { CLOCK_CV, PULSE_WIDTH, NUM_PARAMS };
    enum InputIds { NUM_INPUTS };
    enum OutputIds { CLOCK_CV_OUT, GATE_OUT, NUM_OUTPUTS };
    enum LightIds { NUM_LIGHTS };

#if RACK_V1
    SurgeClock() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CLOCK_CV, -2, 6, 1);
        configParam(PULSE_WIDTH, 0.01, 0.99, 0.5);
    }
#else
    SurgeClock()
        : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    }
#endif

    virtual std::string getName() override { return "CLOCK"; }
    
    StringCache bpmCache, pwCache;

    float phase = 0;
    float lastPulse = -1;

#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        bool newBPM = false;
#if RACK_V1                
        newBPM = updateBPMFromClockCV(getParam(CLOCK_CV), args.sampleTime, args.sampleRate );
#else
        newBPM = updateBPMFromClockCV(getParam(CLOCK_CV), rack::engineGetSampleTime(), rack::engineGetSampleRate() );
#endif
        if( newBPM )
        {
            char txt[256];
            snprintf(txt, 256, "%5.1lf", lastBPM );
            bpmCache.reset(txt);
        }

        if( getParam(PULSE_WIDTH) != lastPulse )
        {
            lastPulse = getParam(PULSE_WIDTH);
            char txt[256];
            snprintf(txt, 256, "%5.3lf", getParam(PULSE_WIDTH));
            pwCache.reset(txt);
        }
        
        phase += dPhase;
        if( phase > 1 )
            phase -= 1;
        float gate = ( phase > getParam(PULSE_WIDTH) ) ? 0 : 10;

        setOutput(CLOCK_CV_OUT,getParam(CLOCK_CV));
        setOutput(GATE_OUT, gate);
    }
};
