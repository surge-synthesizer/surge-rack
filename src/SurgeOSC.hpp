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
        PITCH_0_IN_FREQ,
        
        OSC_CTRL_PARAM_0,

        NUM_PARAMS = OSC_CTRL_PARAM_0 + n_osc_params
    };
    enum InputIds {
        PITCH_CV,

        OSC_CTRL_CV_0,

        NUM_INPUTS = OSC_CTRL_CV_0 + n_osc_params
    };
    enum OutputIds { OUTPUT_L, OUTPUT_R, NUM_OUTPUTS };
    enum LightIds { NUM_LIGHTS };

    ParamCache pc;

#if RACK_V1
    SurgeOSC() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(OUTPUT_GAIN, 0, 1, 1);
        configParam(OSC_TYPE, 0, 4, 0);
        configParam(PITCH_0, 1, 127, 60);
        configParam(PITCH_0_IN_FREQ, 0, 1, 0);
        for (int i = 0; i < n_osc_params; ++i)
            configParam(OSC_CTRL_PARAM_0 + i, 0, 1, 0.5);
        setupSurge();
    }
#else
    SurgeOSC()
        : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    std::vector<std::pair<int, std::string>> oscConfigurations;
    StringCache oscNameCache;
    StringCache pitch0DisplayCache;
    
    StringCache paramNameCache[n_osc_params], paramValueCache[n_osc_params];

    virtual void setupSurge() {
        setupSurgeCommon();

        pc.resize(NUM_PARAMS);
        pc.update(this);

        oscConfigurations.push_back(std::pair<int, std::string>(0, "Classic"));
        oscConfigurations.push_back(std::pair<int, std::string>(1, "Sine"));
        oscConfigurations.push_back(
            std::pair<int, std::string>(6, "FM2 (fixed)"));
        oscConfigurations.push_back(
            std::pair<int, std::string>(5, "FM3 (free)"));
        oscConfigurations.push_back(std::pair<int, std::string>(3, "SH Noise"));

        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        auto config = oscConfigurations[0];
        respawn(config.first);
        oscNameCache.reset(config.second);


        /*
        ** I am making a somewhat dangerous assumption here which is a good one
        ** as long as noone changes the memory layout in SurgeStorage.h. Namely that
        ** in oscstorage all the parameters are "up front" and are in order with 
        ** nothing between them.
        */
        setupStorageRanges((Parameter *)oscstorage, &(oscstorage->retrigger));
    }

    int processPosition = BLOCK_SIZE_OS + 1;

    void updatePitchCache() {
        char txt[ 1024 ];
        if( getParam(PITCH_0_IN_FREQ) > 0.5)
        {
            float fpitch = getParam(PITCH_0);
            float freq = 440.f * pow(2.0, (fpitch-69.0)/12.0);
            snprintf(txt, 1024, "%7.2f hz", freq );
            char *x = txt;
            while( *x && *x == ' ' ) ++x;
            pitch0DisplayCache.reset(x);
        }
        else
        {
            int ipitch = (int)getParam(PITCH_0);
            get_notename( txt, ipitch + 12 );
            pitch0DisplayCache.reset(txt);
        }

    }
    
    void respawn(int i) {
        for (int i = 0; i < n_osc_params; ++i) {
            oscstorage->p[i].set_name("-");
            oscstorage->p[i].set_type(ct_none);
        }
        surge_osc.reset(spawn_osc(i, storage.get(), oscstorage,
                                  storage->getPatch().scenedata[0]));
        surge_osc->init(72.0);
        surge_osc->init_ctrltypes();
        surge_osc->init_default_values();
        processPosition = BLOCK_SIZE_OS + 1;
        oscstorage->type.val.i = i;
        for (auto i = 0; i < n_osc_params; ++i) {
            paramNameCache[i].reset(oscstorage->p[i].get_name());
            char txt[256];
            oscstorage->p[i].get_display(txt, false, 0);
            paramValueCache[i].reset(txt);

            setParam(OSC_CTRL_PARAM_0 + i, oscstorage->p[i].get_value_f01());
        }

        updatePitchCache();
        
        pc.update(this);
    }

    int lastUnison = -1;
#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if (processPosition >= BLOCK_SIZE_OS) {
            // As @Vortico says "think like a hardware engineer; only snap values when you need them".
            processPosition = 0;

            bool respawned = false;
            if ((int)getParam(OSC_TYPE) != (int)pc.get(OSC_TYPE)) {
                auto conf = oscConfigurations[(int)getParam(OSC_TYPE)];
                respawn(conf.first);
                oscNameCache.reset(conf.second);
                respawned = true;
            }

            if(pc.changed(PITCH_0, this) ||
               pc.changed(PITCH_0_IN_FREQ, this))
            {
                updatePitchCache();
            }
            
            for (int i = 0; i < n_osc_params; ++i) {
                if (getParam(OSC_CTRL_PARAM_0 + i) !=
                    pc.get(OSC_CTRL_PARAM_0 + i) || respawned) {
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                    char txt[256];
                    oscstorage->p[i].get_display(txt, false, 0);
                    paramValueCache[i].reset(txt);
                }
            }

            if( respawned )
                lastUnison = oscstorage->p[n_osc_params-1].val.i;

            /*
            ** Unison is special; in surge it is "per voice" and we have one
            ** voice here; so in classic mode if unison changes, then go ahead
            ** and re-init
            */
            if((int)getParam(OSC_TYPE) == 0 &&
               pc.changed(OSC_CTRL_PARAM_0 + n_osc_params - 1, this ) &&
               oscstorage->p[n_osc_params-1].val.i != lastUnison
                )
            {
                lastUnison = oscstorage->p[n_osc_params-1].val.i;
                surge_osc->init(72.0);
            }

            pc.update(this);
            if( outputConnected(OUTPUT_L) || outputConnected(OUTPUT_R) )
            {
                for (int i = 0; i < n_scene_params; ++i) {
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i) +
                                                   getInput(OSC_CTRL_CV_0 + i) / 10.0);
                }

                copyScenedataSubset(0, storage_id_start, storage_id_end);
                float pitch0 = (getParam(PITCH_0_IN_FREQ) > 0.5) ? getParam(PITCH_0) : (int)getParam(PITCH_0);
                surge_osc->process_block(
                    pitch0 + getInput(PITCH_CV) * 12.0, 0, true);
            }
        }

        float avgl = (surge_osc->output[processPosition] + surge_osc->output[processPosition+1]) * 0.5;
        float avgr = (surge_osc->outputR[processPosition] + surge_osc->output[processPosition+1]) * 0.5;
        if( outputConnected(OUTPUT_L) && !outputConnected(OUTPUT_R) )
        {
            // Special mono mode
            float output = (avgl + avgr) * 5.0 * getParam(OUTPUT_GAIN);
            setOutput(OUTPUT_L, output);
        }
        else
        {
            if( outputConnected(OUTPUT_L) )
                setOutput(OUTPUT_L, avgl * 10 *
                          getParam(OUTPUT_GAIN));
            
            if( outputConnected(OUTPUT_R) )
                setOutput(OUTPUT_R, avgr * 10 *
                          getParam(OUTPUT_GAIN));
        }

        processPosition += 2; // that's why the call it block_size _OS (oversampled)
    }

    std::unique_ptr<Oscillator> surge_osc;
    OscillatorStorage *oscstorage;
};
