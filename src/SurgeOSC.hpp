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

    ParamValueStateSaver knobSaver;
    
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

    virtual std::string getName() override { return "OSC"; }
    
    std::vector<std::pair<int, std::string>> oscConfigurations;
    StringCache oscNameCache;
    StringCache pitch0DisplayCache;
    
    StringCache paramNameCache[n_osc_params], paramValueCache[n_osc_params];

    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);

        oscConfigurations.push_back(std::pair<int, std::string>(ot_classic, "Classic"));
        oscConfigurations.push_back(std::pair<int, std::string>(ot_sinus, "Sine"));
        oscConfigurations.push_back(
            std::pair<int, std::string>(ot_FM2, "FM2 (fixed)"));
        oscConfigurations.push_back(
            std::pair<int, std::string>(ot_FM, "FM3 (free)"));
        oscConfigurations.push_back(std::pair<int, std::string>(ot_shnoise, "SH Noise"));

        
        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        int configNum = 0;
        if( firstRespawnIsFromJSON )
        {
            configNum = (int)getParam(OSC_TYPE);
        }

        auto config = oscConfigurations[configNum];
        respawn(config.first, 0);
        oscNameCache.reset(config.second);


        /*
        ** I am making a somewhat dangerous assumption here which is a good one
        ** as long as noone changes the memory layout in SurgeStorage.h. Namely that
        ** in oscstorage all the parameters are "up front" and are in order with 
        ** nothing between them.
        */
        setupStorageRanges((Parameter *)oscstorage, &(oscstorage->retrigger));

        pc.update(this);
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
    
    void respawn(int i, int idx) {
        if( idx == 0 )
        {
            for (int i = 0; i < n_osc_params; ++i) {
                oscstorage->p[i].set_name("-");
                oscstorage->p[i].set_type(ct_none);
            }
            surge_osc.resize(MAX_POLY);
        }
        
        surge_osc[idx].reset(spawn_osc(i, storage.get(), oscstorage,
                                       storage->getPatch().scenedata[0]));
        surge_osc[idx]->init(72.0);
        surge_osc[idx]->init_ctrltypes();

        if( idx == 0 )
            surge_osc[idx]->init_default_values();
        
        processPosition = BLOCK_SIZE_OS + 1;
        oscstorage->type.val.i = i;
        if( idx == 0 )
        {
            for (auto i = 0; i < n_osc_params; ++i) {
                if( ! firstRespawnIsFromJSON )
                {
                    /*
                    ** I"m not coming from JSON - so use the params I see
                    */
                    if( oscstorage->p[i].ctrltype == ct_none )
                    {
                        setParam(OSC_CTRL_PARAM_0 + 0, 0.0 );
                    }
                    else
                    {
                        setParam(OSC_CTRL_PARAM_0 + i, oscstorage->p[i].get_value_f01());
                    }
                }
                else
                {
                    /*
                    ** I am coming from JSON so use the params I have
                    */
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                }
                paramNameCache[i].reset(oscstorage->p[i].get_name());
                char txt[256];
                oscstorage->p[i].get_display(txt, false, 0);
                paramValueCache[i].reset(txt);
            }
        }
        else
        {
            for (auto i = 0; i < n_osc_params; ++i) {
                oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
            }
        }
        firstRespawnIsFromJSON = false;

        updatePitchCache();
        
        pc.update(this);
    }

    int lastUnison = -1;
    int lastNChan = -1;
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[PITCH_CV].getChannels());
        outputs[OUTPUT_L].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);
        if( nChan != lastNChan )
        {
            for( int i=0; i<nChan; i++ )
            {
                if( surge_osc[i] == nullptr )
                {
                    auto conf = oscConfigurations[(int)getParam(OSC_TYPE)];

                    respawn(conf.first, i);
                }
            }
            
            for( int i=nChan; i < MAX_POLY; ++i )
            {
                if( surge_osc[i] != nullptr )
                {
                    surge_osc[i].reset(nullptr);
                }
            }
            lastNChan = nChan;
        }
            
        
        if (processPosition >= BLOCK_SIZE_OS) {
            // As @Vortico says "think like a hardware engineer; only snap values when you need them".
            processPosition = 0;

            bool respawned = false;
            if ((int)getParam(OSC_TYPE) != (int)pc.get(OSC_TYPE)) {
                knobSaver.storeParams( (int)pc.get(OSC_TYPE), OSC_CTRL_PARAM_0, OSC_CTRL_PARAM_0 + n_osc_params, this );
                
                auto conf = oscConfigurations[(int)getParam(OSC_TYPE)];
                for( int c=0; c<nChan; ++c )
                {
                    respawn(conf.first, c);
                }
                oscNameCache.reset(conf.second);
                respawned = true;
                
                knobSaver.applyFromIndex((int)getParam(OSC_TYPE), this );
            }

            if(pc.changed(PITCH_0, this) ||
               pc.changed(PITCH_0_IN_FREQ, this))
            {
                if( pc.changed(PITCH_0_IN_FREQ, this ) && getParam(PITCH_0_IN_FREQ) > 0.5 )
                {
                    // We have switched from note to frequency. We really want to be at the note exactly
                    setParam(PITCH_0, (int)getParam(PITCH_0));
                }
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

            bool newUnison = false;
            if((int)getParam(OSC_TYPE) == 0 &&
               pc.changed(OSC_CTRL_PARAM_0 + n_osc_params - 1, this ) &&
               oscstorage->p[n_osc_params-1].val.i != lastUnison
                )
            {
                lastUnison = oscstorage->p[n_osc_params-1].val.i;
                newUnison = true;
            }
            
            for( int c=0; c<nChan; ++c)
            {
                /*
                ** Unison is special; in surge it is "per voice" and we have one
                ** voice here; so in classic mode if unison changes, then go ahead
                ** and re-init
                */
                if( newUnison ) {
                    surge_osc[c]->init(72.0);
                }

                if( outputConnected(OUTPUT_L) || outputConnected(OUTPUT_R) )
                {
                    for (int i = 0; i < n_osc_params; ++i) {
                        if( pc.changed(OSC_CTRL_PARAM_0 + i, this) ||
                            inputs[OSC_CTRL_CV_0 + i].isConnected() )
                        {
                            oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i) +
                                                           inputs[OSC_CTRL_CV_0 + i].getPolyVoltage(c) * RACK_TO_SURGE_CV_MUL );
                        }
                    }
                    
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    float pitch0 = (getParam(PITCH_0_IN_FREQ) > 0.5) ? getParam(PITCH_0) : (int)getParam(PITCH_0);
                    surge_osc[c]->process_block(
                        pitch0 + inputs[PITCH_CV].getVoltage(c) * 12.0, 0, true);
                }
            }
            pc.update(this);
        }

        for( int c=0; c<nChan; ++c )
        {
            float avgl = (surge_osc[c]->output[processPosition] + surge_osc[c]->output[processPosition+1]) * 0.5;
            float avgr = (surge_osc[c]->outputR[processPosition] + surge_osc[c]->outputR[processPosition+1]) * 0.5;
            if( outputConnected(OUTPUT_L) && !outputConnected(OUTPUT_R) )
            {
                // Special mono mode
                float output = (avgl + avgr) * 0.5 * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN);
                outputs[OUTPUT_L].setVoltage(output,c);
            }
            else
            {
                if( outputConnected(OUTPUT_L) )
                    outputs[OUTPUT_L].setVoltage(avgl * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN), c);
                
                if( outputConnected(OUTPUT_R) )
                    outputs[OUTPUT_R].setVoltage(avgr * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN), c);
            }
        }
            
        processPosition += 2; // that's why the call it block_size _OS (oversampled)
    }

    std::vector<std::unique_ptr<Oscillator>> surge_osc;
    OscillatorStorage *oscstorage;
};
