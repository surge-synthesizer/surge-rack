#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/Oscillator.h"
#include "rack.hpp"
#include <cstring>

struct SurgeOSC : virtual public SurgeModuleCommon {
    enum ParamIds {
        OUTPUT_GAIN,

        OSC_TYPE,
        PITCH_0,
        
        NUM_PARAMS
    };
    enum InputIds {
        PITCH_CV,
        
        NUM_INPUTS 
    };
    enum OutputIds {
        OUTPUT_L,
        OUTPUT_R,
        NUM_OUTPUTS
    };
    enum LightIds { NUM_LIGHTS };

    ParamCache pc;
    
#if RACK_V1
    SurgeOSC() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(OUTPUT_GAIN,0,1,1);
        configParam(OSC_TYPE,0,4,0);
        configParam(PITCH_0, 1, 127, 72);
        setupSurge();
    }
#else
    SurgeOSC() : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    std::vector<std::pair<int, std::string>> oscConfigurations;
    StringCache oscNameCache;
    
    virtual void setupSurge() {
        setupSurgeCommon();
        
        pc.resize(NUM_PARAMS);
        pc.update(this);
        
        oscConfigurations.push_back(std::pair<int,std::string>(0, "Classic"));
        oscConfigurations.push_back(std::pair<int,std::string>(1, "Sine"));
        oscConfigurations.push_back(std::pair<int,std::string>(6, "FM2 (fixed)"));
        oscConfigurations.push_back(std::pair<int,std::string>(5, "FM3 (free)"));
        oscConfigurations.push_back(std::pair<int,std::string>(3, "SH Noise"));
        
        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        auto config = oscConfigurations[0];
        surge_osc.reset(spawn_osc(config.first, storage.get(), oscstorage, storage->getPatch().scenedata[0]));
        oscNameCache.reset(config.second);
        surge_osc->init(72.0);

        for( auto i=0; i<n_osc_params; ++i )
            INFO( "  %d -> %s", i, oscstorage->p[i].get_name() );
    }

    int processPosition = BLOCK_SIZE_OS + 1;
    
#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if( (int)getParam(OSC_TYPE) != (int)pc.get(OSC_TYPE) )
        {
            auto conf = oscConfigurations[(int)getParam(OSC_TYPE)];
            surge_osc.reset(spawn_osc(conf.first, storage.get(), oscstorage, storage->getPatch().scenedata[0]));
            surge_osc->init(72.0);
            processPosition = BLOCK_SIZE_OS + 1;
            oscNameCache.reset(conf.second);
        }
        pc.update(this);
        
        if( processPosition >= BLOCK_SIZE_OS )
        {
            processPosition = 0;
            storage->getPatch().copy_scenedata(storage->getPatch().scenedata[0], 0);
            surge_osc->process_block(getParam(PITCH_0) + getInput(PITCH_CV) * 12.0, 0, true);
        }

        setOutput(OUTPUT_L, surge_osc->output[processPosition] * 10 * getParam(OUTPUT_GAIN));
        setOutput(OUTPUT_R, surge_osc->outputR[processPosition] * 10 * getParam(OUTPUT_GAIN));

        processPosition ++;
    }

    std::unique_ptr<Oscillator> surge_osc;
    OscillatorStorage *oscstorage;
    
};
