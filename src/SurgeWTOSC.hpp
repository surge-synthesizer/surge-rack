#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/Oscillator.h"
#include "rack.hpp"
#include <cstring>

struct SurgeWTOSC : virtual public SurgeModuleCommon {
    enum ParamIds {
        OUTPUT_GAIN,

        PITCH_0,
        PITCH_0_IN_FREQ,
        
        OSC_CTRL_PARAM_0,

        CATEGORY_IDX = OSC_CTRL_PARAM_0 + n_osc_params,
        WT_IN_CATEGORY_IDX,

        LOAD_WT,

        WT_OR_WINDOW,
        
        NUM_PARAMS
    };
    enum InputIds {
        PITCH_CV,

        OSC_CTRL_CV_0,

        NUM_INPUTS = OSC_CTRL_CV_0 + n_osc_params
    };
    enum OutputIds { OUTPUT_L, OUTPUT_R, NUM_OUTPUTS };
    enum LightIds {
        NEEDS_LOAD,
        NUM_LIGHTS
    };

    ParamCache pc;
    
    SurgeWTOSC() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(OUTPUT_GAIN, 0, 1, 1);
        configParam(PITCH_0, 1, 127, 60);
        configParam(PITCH_0_IN_FREQ, 0, 1, 0);
        for (int i = 0; i < n_osc_params; ++i)
            configParam(OSC_CTRL_PARAM_0 + i, 0, 1, 0.5);

        configParam(CATEGORY_IDX, 0,1, 0);
        configParam(WT_IN_CATEGORY_IDX, 0,1, 0);
        configParam(WT_OR_WINDOW,0,1,0);
        setupSurge();
    }

    virtual std::string getName() override { return "WTOSC"; }
    
    StringCache pitch0DisplayCache;
    
    StringCache paramNameCache[n_osc_params], paramValueCache[n_osc_params], wtInfoCache[3];
    ParamValueStateSaver knobSaver;

    std::vector<int> catOrderSkipEmpty;
    
    virtual void setupSurge() {
        pc.resize(NUM_PARAMS);
        setupSurgeCommon();

        for( auto ci : storage->wtCategoryOrdering )
        {
            PatchCategory pc = storage->wt_category[ci];
            if( pc.numberOfPatchesInCatgory != 0 )
            {
                catOrderSkipEmpty.push_back(ci);
            }
        }

        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        for (int i = 0; i < n_osc_params; ++i) {
            oscstorage->p[i].set_name("-");
            oscstorage->p[i].set_type(ct_none);
        }
        surge_osc.reset(spawn_osc(ot_wavetable, storage.get(), oscstorage,
                                  storage->getPatch().scenedata[0]));

        /*
        ** FIXME: This is a foul hack we should work around. Something about the surge synth
        ** initialization path sets up wavetables in a way which means you can load any wt
        ** you want, but in surge-rack if you haven't loadeed a 512 wide wavetable once 
        ** you get odd behavior. See issue #124.
        **
        ** I should really fix that in surge proper but to get shipping for now, I'm just
        ** going to do the gross thing of finding "sawtooth reso" in our list and loading that at
        ** init time befire clobbering
        */
        oscstorage->wt.queue_id = 0;
        for( int i=0; i<storage->wt_list.size(); ++i )
        {
            if( storage->wt_list[i].name == "sawtooth reso" )
            {
                oscstorage->wt.queue_id = i;
                break;
            }
        }
        storage->perform_queued_wtloads();
        surge_osc->init(72.0);

        surge_osc->init_ctrltypes();
        surge_osc->init_default_values();

        updateWtIdx();
        oscstorage->wt.queue_id = wtIdx;
        storage->perform_queued_wtloads();
        surge_osc->init(72.0);
        updateWtLabels();

        processPosition = BLOCK_SIZE_OS + 1;
        oscstorage->type.val.i = 2;

        for (auto i = 0; i < n_osc_params; ++i) {
            setParam(OSC_CTRL_PARAM_0 + i, oscstorage->p[i].get_value_f01());
            paramNameCache[i].reset(oscstorage->p[i].get_name());
            char txt[256];
            oscstorage->p[i].get_display(txt, false, 0);
            paramValueCache[i].reset(txt);
        }

        updatePitchCache();

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

    void updateWtLabels() {
        Patch p = storage->wt_list[wtIdx];
        PatchCategory pc = storage->wt_category[p.category];
        wtInfoCache[0].reset(pc.name);
        wtInfoCache[1].reset(p.name);

        char txt[256];
        snprintf(txt, 256, "%d tbl of %d samples",
                 oscstorage->wt.n_tables, oscstorage->wt.size );
        wtInfoCache[2].reset(txt);
    }
    
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

    int wtIdx = 0;
    StringCache wtCategoryName;

    StringCache wtItemName[7]; // 3 on each side

    void updateWtIdx() {
        int priorWtIdx = wtIdx;
        
        /*
        ** FIXME: So many ways to make this calculate less (like stash int of patchIdx and stuf
        */
        int catIdx = getParam(CATEGORY_IDX) * catOrderSkipEmpty.size();
        if( catIdx >= catOrderSkipEmpty.size() ) catIdx --;

        PatchCategory cat = storage->wt_category[catOrderSkipEmpty[catIdx]];
        
        wtCategoryName.reset( cat.name );

        int patchIdx = getParam(WT_IN_CATEGORY_IDX) * cat.numberOfPatchesInCatgory;
        if( patchIdx == cat.numberOfPatchesInCatgory ) patchIdx--;

        int catO = catOrderSkipEmpty[catIdx];
        int counted = 0;
        Patch p;
        bool found = false;
        int firstCount = -1, lastCount = 0;;
        for( auto pci : storage->wtOrdering )
        {
            if( storage->wt_list[pci].category == catO )
            {

                int countDiff = counted++ - patchIdx;
                if( countDiff == 0 )
                {
                    p = storage->wt_list[pci];
                    found = true;
                    wtIdx = pci;
                }
                countDiff += 3;

                if( countDiff >= 7 )
                    break;
                if( countDiff >= 0 )
                {
                    if( firstCount < 0 ) firstCount = countDiff;
                    lastCount = countDiff;
                    wtItemName[countDiff].resetCheck(storage->wt_list[pci].name);
                }
            }
        }

        for( int i=0; i<firstCount; ++i )
            wtItemName[i].resetCheck( "-" );
        for( int i=lastCount+1; i<7; ++i )
            wtItemName[i].resetCheck( "-" );
        
        if (!found)
            for( int i=0; i<7; ++i )
                wtItemName[i].resetCheck( "ERROR" );
    }

    int lastUnison = -1;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if (processPosition >= BLOCK_SIZE_OS) {
            // As @Vortico says "think like a hardware engineer; only snap values when you need them".
            processPosition = 0;

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
            
            if( pc.changed(WT_OR_WINDOW, this) )
            {
                knobSaver.storeParams( (int)pc.get(WT_OR_WINDOW), OSC_CTRL_PARAM_0, OSC_CTRL_PARAM_0 + n_osc_params, this );
                
                int toWhat = ot_wavetable;
                if( getParam(WT_OR_WINDOW) > 0.5)
                    toWhat = ot_WT2;

                for (int i = 0; i < n_osc_params; ++i) {
                    oscstorage->p[i].set_name("-");
                    oscstorage->p[i].set_type(ct_none);
                }

                surge_osc.reset(spawn_osc(toWhat, storage.get(), oscstorage,
                                          storage->getPatch().scenedata[0]));
                surge_osc->init(72.0);
                surge_osc->init_ctrltypes();
                surge_osc->init_default_values();
                
                oscstorage->wt.queue_id = wtIdx;
                storage->perform_queued_wtloads();
                surge_osc->init(72);
                updateWtLabels();

                
                for (auto i = 0; i < n_osc_params; ++i) {
                    setParam(OSC_CTRL_PARAM_0 + i, oscstorage->p[i].get_value_f01());
                }
                knobSaver.applyFromIndex((int)getParam(WT_OR_WINDOW), this );

                for(auto i=0; i<n_osc_params; ++i )
                {
                    paramNameCache[i].reset(oscstorage->p[i].get_name());
                    char txt[256];
                    oscstorage->p[i].get_display(txt, false, 0);
                    paramValueCache[i].reset(txt);
                }
            }

            for (int i = 0; i < n_osc_params; ++i) {
                if (getParam(OSC_CTRL_PARAM_0 + i) !=
                    pc.get(OSC_CTRL_PARAM_0 + i) ) {
                    oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                    char txt[256];
                    oscstorage->p[i].get_display(txt, false, 0);
                    paramValueCache[i].reset(txt);
                }
            }


            if( pc.changed(CATEGORY_IDX, this) || pc.changed(WT_IN_CATEGORY_IDX, this) )
            {
                updateWtIdx();
            }

            if( pc.changedAndIsNonZero(LOAD_WT, this) || firstRespawnIsFromJSON )
            {
                oscstorage->wt.queue_id = wtIdx;
                storage->perform_queued_wtloads();
                surge_osc->init(72);
                updateWtLabels();
            }

            if( wtIdx != oscstorage->wt.current_id )
            {
                setLight(NEEDS_LOAD, 1.0);
            }
            else
            {
                setLight(NEEDS_LOAD, 0.0);
            }
        

            if(oscstorage->p[n_osc_params-1].val.i != lastUnison)
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
        float avgr = (surge_osc->outputR[processPosition] + surge_osc->outputR[processPosition+1]) * 0.5;
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
        firstRespawnIsFromJSON = false;
    }

    std::unique_ptr<Oscillator> surge_osc;
    OscillatorStorage *oscstorage;
};
