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

    SurgeClock() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CLOCK_CV, -2, 6, 1);
        configParam(PULSE_WIDTH, 0.01, 0.99, 0.5);
    }

    virtual std::string getName() override { return "CLOCK"; }
    
    StringCache bpmCache, pwCache;

    float phase = 0;
    float lastPulse = -1;

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        bool newBPM = false;
        
        newBPM = updateBPMFromClockCV(getParam(CLOCK_CV), args.sampleTime, args.sampleRate );
        
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
        float gate = ( phase > getParam(PULSE_WIDTH) ) ? 0 : RACK_CV_MAX_LEVEL;

        outputs[CLOCK_CV_OUT].setVoltage(getParam(CLOCK_CV));
        outputs[GATE_OUT].setVoltage(gate);
    }
};
