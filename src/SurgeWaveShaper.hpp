#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <cstring>

struct SurgeWaveShaper : virtual public SurgeModuleCommon
{
    enum ParamIds
    {
        MODE_PARAM,
        DRIVE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        DRIVE_CV,
        SIGNAL_IN,
        NUM_INPUTS
    };
    enum OutputIds
    {
        SIGNAL_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    static constexpr int n_ws_types = 1; // FIXME

    SurgeWaveShaper() : SurgeModuleCommon()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MODE_PARAM, 0, n_ws_types - 1, 0, "Mode");
        configParam(DRIVE_PARAM, -24.0, 24.0, 0, "Drive",
                    "dB"); // ct_decibel_narrow
        setupSurge();
    }

    virtual std::string getName() override { return "WS"; }

    StringCache dbGainCache;

    virtual void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS);
        for (int i = 0; i < MAX_POLY; ++i)
            processPosition[i] = 0;
    }

    int processPosition[MAX_POLY];
    float inBuffer alignas(16)[MAX_POLY][4], outBuffer alignas(16)[MAX_POLY][4];

    void swapWS(int i)
    {
#if 0
        if (i == 0)
            wsPtr = nullptr;
        else
            wsPtr = GetQFPtrWaveshaper(i);

        for(int c=0; c<MAX_POLY; ++c )
        {
            for (int i = 0; i < 4; ++i) {
                inBuffer[c][i] = 0;
                outBuffer[c][i] = 0;
            }
            processPosition[c] = 0;
        }
#endif
    }

    void process(const typename rack::Module::ProcessArgs &args) override
    {
#if 0
        if ((int)getParam(MODE_PARAM) != (int)pc.get(MODE_PARAM)) {
            swapWS((int)getParam(MODE_PARAM));
        }
        if (getParam(DRIVE_PARAM) != pc.get(DRIVE_PARAM)) {
            char txt[256];
            snprintf(txt, 256, "%.2f dB", getParam(DRIVE_PARAM));
            dbGainCache.reset(txt);
        }
        pc.update(this);

        int nChan = std::max(1, inputs[SIGNAL_IN].getChannels());
        outputs[SIGNAL_OUT].setChannels(nChan);
        for( int i=0; i<nChan; ++i )
        {
            float drive = db_to_linear(getParam(DRIVE_PARAM) + inputs[DRIVE_CV].getPolyVoltage(i));

            if (wsPtr == nullptr) {
                outputs[SIGNAL_OUT].setVoltage(inputs[SIGNAL_IN].getVoltage(i),i);
            } else {
                if (processPosition[i] == 4) {
                    __m128 in, driveM, out;
                    in = _mm_load_ps(inBuffer[i]);
                    driveM = _mm_set1_ps(drive);
                    // FIXME
                    // out = wsPtr(in, driveM);
                    _mm_store_ps(outBuffer[i], out);
                    processPosition[i] = 0;
                }
                inBuffer[i][processPosition[i]] = inputs[SIGNAL_IN].getVoltage(i) * RACK_TO_SURGE_OSC_MUL;
                outputs[SIGNAL_OUT].setVoltage( outBuffer[i][processPosition[i]] * SURGE_TO_RACK_OSC_MUL, i);
                processPosition[i]++;
            }
        }
#endif
    }

    // WaveshaperQFPtr wsPtr = nullptr;
};
