#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/QuadFilterUnit.h"
#include "rack.hpp"
#include <cstring>

struct SurgeWaveShaper : virtual public SurgeModuleCommon {
    enum ParamIds {
        MODE_PARAM,
        DRIVE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        DRIVE_CV,
        SIGNAL_IN,
        NUM_INPUTS
    };
    enum OutputIds {
        SIGNAL_OUT,
        NUM_OUTPUTS
    };
    enum LightIds { NUM_LIGHTS };

#if RACK_V1
    SurgeWaveShaper() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MODE_PARAM, 0, n_ws_type-1, 0);
        configParam(DRIVE_PARAM,-24.0, 24.0, 0); // ct_decibel_narrow
        setupSurge();
    }
#else
    SurgeWaveShaper() : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    ParamCache pc;
    StringCache wsNameCache;
    std::vector<std::string> wsNames;
    
    virtual void setupSurge() {
        setupSurgeCommon();
        
        pc.resize(NUM_PARAMS);

        wsNames.push_back( "none" );
        wsNames.push_back( "tanh" );
        wsNames.push_back( "hard" );
        wsNames.push_back( "asym" );
        wsNames.push_back( "sinus" );
        wsNames.push_back( "digi" );
    }

    int processPosition = 0;
    float wsMul = 1.0;
    float inBuffer alignas(16)[4], outBuffer alignas(16)[4];

    void swapWS(int i) {
        if( i == 0 )
            wsPtr = nullptr;
        else
            wsPtr = GetQFPtrWaveshaper(i);

        wsMul = 10.0;
        if( i == 0 || i == wst_digi ) wsMul = 1.0;
        
        for( int i=0; i<4; ++i )
        {
            inBuffer[ i ] = 0;
            outBuffer[ i ] = 0;
            processPosition = 0;
        }
    }
    
#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if( (int)getParam(MODE_PARAM) != (int)pc.get(MODE_PARAM) )
        {
            swapWS((int)getParam(MODE_PARAM));
            wsNameCache.reset(wsNames[(int)getParam(MODE_PARAM)]);
        }
        pc.update(this);

        float drive = db_to_linear(getParam(DRIVE_PARAM));
        
        if( wsPtr == nullptr )
        {
            setOutput(SIGNAL_OUT, getInput(SIGNAL_IN));
        }
        else
        {
            if(processPosition == 4)
            {
                __m128 in, driveM, out;
                in = _mm_load_ps(inBuffer);
                driveM = _mm_set1_ps(drive);
                out = wsPtr(in, driveM);
                _mm_store_ps(outBuffer, out);
                processPosition = 0;
            }
            inBuffer[processPosition] = getInput(SIGNAL_IN);
            setOutput(SIGNAL_OUT, outBuffer[processPosition] * wsMul);
            processPosition ++;
        }
        
    }

    WaveshaperQFPtr wsPtr = nullptr;
    
};
