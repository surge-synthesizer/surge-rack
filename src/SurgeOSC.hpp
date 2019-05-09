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
        configParam(PITCH_0, 1, 127, 72);
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

    void respawn(int i) {
        for (int i = 0; i < n_osc_params; ++i) {
            oscstorage->p[i].set_name("-");
            oscstorage->p[i].set_type(ct_none);
        }
        surge_osc.reset(spawn_osc(i, storage.get(), oscstorage,
                                  storage->getPatch().scenedata[0]));
        surge_osc->init(72.0);
        surge_osc->init_ctrltypes();
        processPosition = BLOCK_SIZE_OS + 1;
        oscstorage->type.val.i = i;
        for (auto i = 0; i < n_osc_params; ++i) {
            paramNameCache[i].reset(oscstorage->p[i].get_name());
            char txt[256];
            oscstorage->p[i].get_display(txt, false, 0);
            paramValueCache[i].reset(txt);
        }
    }

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
            
            for (int i = 0; i < n_osc_params; ++i) {
                if (getParam(OSC_CTRL_PARAM_0 + i) !=
                    pc.get(OSC_CTRL_PARAM_0 + i) || respawned) {
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                    char txt[256];
                    oscstorage->p[i].get_display(txt, false, 0);
                    paramValueCache[i].reset(txt);
                }
            }
            
            pc.update(this);
            if( outputConnected(OUTPUT_L) || outputConnected(OUTPUT_R) )
            {
                for (int i = 0; i < n_scene_params; ++i) {
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i) +
                                                   getInput(OSC_CTRL_CV_0 + i) / 10.0);
                }

                copyScenedataSubset(0, storage_id_start, storage_id_end);
                surge_osc->process_block(
                    getParam(PITCH_0) + getInput(PITCH_CV) * 12.0, 0, true);
            }
        }

        if( outputConnected(OUTPUT_L) )
            setOutput(OUTPUT_L, surge_osc->output[processPosition] * 10 *
                      getParam(OUTPUT_GAIN));

        if( outputConnected(OUTPUT_R) )
            setOutput(OUTPUT_R, surge_osc->outputR[processPosition] * 10 *
                      getParam(OUTPUT_GAIN));

        processPosition++;
    }

    std::unique_ptr<Oscillator> surge_osc;
    OscillatorStorage *oscstorage;
};
