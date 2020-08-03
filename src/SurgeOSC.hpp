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
        OSC_DEACTIVATE_INVERSE_PARAM_0 = OSC_CTRL_PARAM_0 + n_osc_params, // state -1, 0, 1 for n/a, off, on
        OSC_EXTEND_PARAM_0 = OSC_DEACTIVATE_INVERSE_PARAM_0 + n_osc_params, // state -1, 0, 1 for n/a, off, on

        NUM_PARAMS = OSC_EXTEND_PARAM_0 + n_osc_params
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

        configParam(OUTPUT_GAIN, 0, 1, 1, "Output Gain");
        configParam(OSC_TYPE, 0, 4, 0, "Oscillator Type");
        configParam(PITCH_0, 1, 127, 60, "Pitch in Midi Note");
        configParam(PITCH_0_IN_FREQ, 0, 1, 0, "Pitch in Hz");
        
        for (int i = 0; i < n_osc_params; ++i)
        {
            configParam<SurgeRackOSCParamQuantity<SurgeOSC>>(OSC_CTRL_PARAM_0 + i, 0, 1, 0.5);
            configParam(OSC_DEACTIVATE_INVERSE_PARAM_0 + i, -1, 1, -1, "Activate (if applicable)" );
            configParam(OSC_EXTEND_PARAM_0 + i, -1, 1, -1, "Extend (if applicable)" );
        }

        setupSurge();

        for( int i=0; i<MAX_POLY; ++i )
        {
            halfbandOUT.emplace_back( 6, true );
            halfbandOUT[i].reset();
        }
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
        ** nothing between them. With 1.6.3 I broke that assumptino and just assumeh tye
        ** are contiguous
        */
        setupStorageRanges(&(oscstorage->type), &(oscstorage->retrigger));

        pc.update(this);
    }

    int processPosition = BLOCK_SIZE + 1;
    
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

    virtual void moduleSpecificSampleRateChange() override {
        forceRespawnDueToSampleRate = true;
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
        
        processPosition = BLOCK_SIZE + 1;
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

                if( oscstorage->p[i].can_deactivate() )
                {
                    setParam(OSC_DEACTIVATE_INVERSE_PARAM_0 + i, 0 );
                    oscstorage->p[i].deactivated = true;
                } else {
                    setParam(OSC_DEACTIVATE_INVERSE_PARAM_0 + i, -1 );
                    oscstorage->p[i].deactivated = true;
                }

                if( oscstorage->p[i].can_extend_range() )
                {
                    setParam(OSC_EXTEND_PARAM_0 + i, 0 );
                    oscstorage->p[i].extend_range = false;
                } else {
                    setParam(OSC_EXTEND_PARAM_0 + i, -1 );
                    oscstorage->p[i].extend_range = false;
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
    bool forceRespawnDueToSampleRate = false;
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
            
        
        if (processPosition >= BLOCK_SIZE) {
            // As @Vortico says "think like a hardware engineer; only snap values when you need them".
            processPosition = 0;

            bool respawned = false;
            if (forceRespawnDueToSampleRate || (int)getParam(OSC_TYPE) != (int)pc.get(OSC_TYPE)) {
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
            forceRespawnDueToSampleRate = false;

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
                bool resetCache = false;
                if (getParam(OSC_CTRL_PARAM_0 + i) !=
                    pc.get(OSC_CTRL_PARAM_0 + i) || respawned) {
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                    resetCache = true;
                }

                if( oscstorage->p[i].can_deactivate() )
                {
                    auto pr = oscstorage->p[i].deactivated;
            
                    if( getParam(OSC_DEACTIVATE_INVERSE_PARAM_0 + i ) > 0.1 )
                        oscstorage->p[i].deactivated = false;
                    else
                        oscstorage->p[i].deactivated = true;

                    resetCache = resetCache || ( oscstorage->p[i].deactivated != pr );
                }
                
                if( oscstorage->p[i].can_extend_range() )
                {
                    auto pr = oscstorage->p[i].extend_range;
                    if( getParam(OSC_EXTEND_PARAM_0 + i ) > 0.1 )
                        oscstorage->p[i].extend_range = true;
                    else
                        oscstorage->p[i].extend_range = false;
                    
                    resetCache = resetCache || ( oscstorage->p[i].extend_range != pr );
                }
                if( resetCache )
                {
                    char txt[256];
                    oscstorage->p[i].get_display(txt, false, 0);
                    paramValueCache[i].reset(txt);
                }

            }

            bool newUnison = false;
            if( respawned )
            {
                lastUnison = oscstorage->p[n_osc_params-1].val.i;
                newUnison = true;
            }

            if( ( (int)getParam(OSC_TYPE) == 0 || // classic
                  (int)getParam(OSC_TYPE) == 1 || // sine
                  (int)getParam(OSC_TYPE) == 4    // shnoise
                    ) &&
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
                    copy_block( surge_osc[c]->output, osc_downsample[0][c], BLOCK_SIZE_OS_QUAD );
                    copy_block( surge_osc[c]->outputR,osc_downsample[1][c], BLOCK_SIZE_OS_QUAD );
                    halfbandOUT[c].process_block_D2( osc_downsample[0][c], osc_downsample[1][c] );
                }
            }
            pc.update(this);
        }

        for( int c=0; c<nChan; ++c )
        {
            if( outputConnected(OUTPUT_L) && !outputConnected(OUTPUT_R) )
            {
                // Special mono mode
                float output = (osc_downsample[0][c][processPosition] +
                                osc_downsample[1][c][processPosition]) * 0.5 * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN);
                outputs[OUTPUT_L].setVoltage(output,c);
            }
            else
            {
                if( outputConnected(OUTPUT_L) )
                    outputs[OUTPUT_L].setVoltage(osc_downsample[0][c][processPosition] * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN), c);
                
                if( outputConnected(OUTPUT_R) )
                    outputs[OUTPUT_R].setVoltage(osc_downsample[1][c][processPosition] * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN), c);
            }
        }
            
        processPosition ++;
    }

    std::vector<std::unique_ptr<Oscillator>> surge_osc;
    OscillatorStorage *oscstorage;
    float  osc_downsample alignas(16)[2][MAX_POLY][BLOCK_SIZE_OS];
    std::vector<HalfRateFilter> halfbandOUT;
};
