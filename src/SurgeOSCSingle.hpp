#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/Oscillator.h"
#include "rack.hpp"
#include <cstring>

template<int type>
struct SingleConfig
{
    static constexpr bool needsRetrigger() { return false; }
};

template <int oscType>
struct SurgeOSCSingle : virtual public SurgeModuleCommon {
    enum ParamIds {
        OUTPUT_GAIN,

        PITCH_0,

        OSC_CTRL_PARAM_0,
        OSC_DEACTIVATE_INVERSE_PARAM_0 = OSC_CTRL_PARAM_0 + n_osc_params, // state -1, 0, 1 for n/a, off, on
        OSC_EXTEND_PARAM_0 = OSC_DEACTIVATE_INVERSE_PARAM_0 + n_osc_params, // state -1, 0, 1 for n/a, off, on

        NUM_PARAMS = OSC_EXTEND_PARAM_0 + n_osc_params
    };
    enum InputIds {
        PITCH_CV,

        RETRIGGER,

        OSC_CTRL_CV_0,

        NUM_INPUTS = OSC_CTRL_CV_0 + n_osc_params
    };
    enum OutputIds { OUTPUT_L, OUTPUT_R, NUM_OUTPUTS };
    enum LightIds { NUM_LIGHTS };

    ParamValueStateSaver knobSaver;
    
    SurgeOSCSingle() : SurgeModuleCommon() {
        for (int i=0; i<MAX_POLY; ++i)
            surge_osc[i] = nullptr;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(OUTPUT_GAIN, 0, 1, 1, "Output Gain");
        configParam(PITCH_0, 1, 127, 60, "Pitch in Midi Note");

        for (int i = 0; i < n_osc_params; ++i)
        {
            configParam<SurgeRackOSCParamQuantity<SurgeOSCSingle>>(OSC_CTRL_PARAM_0 + i, 0, 1, 0.5);
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

    ~SurgeOSCSingle()
    {
        for (int i=0; i<MAX_POLY; ++i)
        {
            if (surge_osc[i])
                 surge_osc[i]->~Oscillator();
            surge_osc[i] = nullptr;
        }
    }

    virtual std::string getName() override { return "OSC"; }
    
    StringCache oscNameCache;
    StringCache pitch0DisplayCache;
    
    StringCache paramNameCache[n_osc_params], paramValueCache[n_osc_params];

    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);

        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        respawn(0);
        oscNameCache.reset(osc_type_names[oscType]);


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
        int ipitch = (int)getParam(PITCH_0);
        get_notename( txt, ipitch + 12 );
        pitch0DisplayCache.reset(txt);
    }

    virtual void moduleSpecificSampleRateChange() override {
        forceRespawnDueToSampleRate = true;
    }
    
    void respawn(int idx) {
        if( idx == 0 )
        {
            for (int i = 0; i < n_osc_params; ++i) {
                oscstorage->p[i].set_name("-");
                oscstorage->p[i].set_type(ct_none);
            }
        }
        
        if (surge_osc[idx])
        {
            surge_osc[idx]->~Oscillator();
            surge_osc[idx] = nullptr;
        }
        surge_osc[idx] = spawn_osc(oscType, storage.get(), oscstorage,
                                   storage->getPatch().scenedata[0], oscbuffer[idx]);
        copyScenedataSubset(0, storage_id_start, storage_id_end);
        surge_osc[idx]->init(72.0);
        surge_osc[idx]->init_ctrltypes();

        if( idx == 0 )
            surge_osc[idx]->init_default_values();
        
        processPosition = BLOCK_SIZE + 1;
        oscstorage->type.val.i = oscType;
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
                respawn(i);
            }
            
            for( int i=nChan; i < MAX_POLY; ++i )
            {
                if( surge_osc[i] != nullptr )
                {
                    surge_osc[i]->~Oscillator();
                    surge_osc[i] = nullptr;
                }
            }
            lastNChan = nChan;
        }
            
        
        if (processPosition >= BLOCK_SIZE) {
            // As @Vortico says "think like a hardware engineer; only snap values when you need them".
            processPosition = 0;

            bool respawned = false;
            if (forceRespawnDueToSampleRate) {
                for( int c=0; c<nChan; ++c )
                {
                    respawn(c);
                }
                respawned = true;
            }
            forceRespawnDueToSampleRate = false;

            if(pc.changed(PITCH_0, this))
            {
                setParam(PITCH_0, (int)getParam(PITCH_0));
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

            if( ( oscType == ot_classic ||
                 oscType == ot_sine ||
                 oscType == ot_shnoise ||
                 oscType == ot_alias ||
                 oscType == ot_modern
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
                bool retrig = false;
                if (inputConnected(RETRIGGER) && reTrigger[c].process(inputs[RETRIGGER].getVoltage(c))) {
                    retrig = true;
                }
                /*
                ** Unison is special; in surge it is "per voice" and we have one
                ** voice here; so in classic mode if unison changes, then go ahead
                ** and re-init
                */
                if( newUnison || retrig ) {
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    surge_osc[c]->init(72.0);
                }

                if( outputConnected(OUTPUT_L) || outputConnected(OUTPUT_R) )
                {
                    for (int i = 0; i < n_osc_params; ++i) {
                        if( true || pc.changed(OSC_CTRL_PARAM_0 + i, this) ||
                            inputs[OSC_CTRL_CV_0 + i].isConnected() )
                        {
                            oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i) +
                                                           inputs[OSC_CTRL_CV_0 + i].getPolyVoltage(c) * RACK_TO_SURGE_CV_MUL );
                        }
                    }
                    
                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    float pitch0 = getParam(PITCH_0);
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

    // With surge-xt the oscillator memory is owned by the synth after spawn
    std::array<Oscillator *, MAX_POLY> surge_osc;
    unsigned char oscbuffer alignas(16)[MAX_POLY][oscillator_buffer_size];

    OscillatorStorage *oscstorage;
    float  osc_downsample alignas(16)[2][MAX_POLY][BLOCK_SIZE_OS];
    std::vector<HalfRateFilter> halfbandOUT;

    rack::dsp::SchmittTrigger reTrigger[MAX_POLY];

};
