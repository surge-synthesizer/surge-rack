#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/DspUtilities.h"
#include "rack.hpp"
#include <cstring>

struct SurgeNoise : virtual public SurgeModuleCommon {
    enum ParamIds { CORRELATION_PARAM, NUM_PARAMS };
    enum InputIds { CORRELATION_CV, NUM_INPUTS };
    enum OutputIds { SIGNAL_OUT, NUM_OUTPUTS };
    enum LightIds { NUM_LIGHTS };

    SurgeNoise() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CORRELATION_PARAM, -1.0, 1.0, -0.2 );
        lv = 0.0;
        lv2 = 0.0;
    }

    virtual std::string getName() override { return "WS"; }
    
    StringCache dbGainCache;

    float lv, lv2;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        float rawCorr = params[CORRELATION_PARAM].getValue() + inputs[CORRELATION_CV].getVoltage() / 5.0;
        float corr = rack::clamp(rawCorr, -0.9999f, 0.9999f);
        float res = correlated_noise_o2mk2(lv, lv2, corr);
        outputs[SIGNAL_OUT].setVoltage(res * SURGE_TO_RACK_OSC_MUL);
    }

};
