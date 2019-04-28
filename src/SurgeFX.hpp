#pragma once

#include "dsp/effect/Effect.h"

template <typename TBase>
struct SurgeFX : virtual TBase
{
    enum ParamIds {
        FX_TYPE,
        FX_PARAM_0,
        NUM_PARAMS = FX_PARAM_0 + 12
    };
    enum InputIds {
        INPUT_R_OR_MONO,
        INPUT_L,

        FX_PARAM_INPUT_0,
        
        NUM_INPUTS = FX_PARAM_INPUT_0 + 12
    };
    enum OutputIds {
        OUTPUT_R_OR_MONO,
        OUTPUT_L,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

#if RACK_V1
    SurgeFX() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        TBase::configParam(FX_TYPE, 0, 9, 1 );
        
        INFO( "Making FX" );
        // TODO: Have a mode where these paths come from res/
        storage.reset(new SurgeStorage());
        INFO( "Storage datapath is: %s", storage->datapath.c_str() );

        // FIX THIS of course
        float sr = 44100.0;
        samplerate = sr;
        dsamplerate = sr;
        samplerate_inv = 1.0 / sr;
        dsamplerate_inv = 1.0 / sr;
        dsamplerate_os = dsamplerate * OSC_OVERSAMPLING;
        dsamplerate_os_inv = 1.0 / dsamplerate_os;
        storage->init_tables();


        fxstorage = &(storage->getPatch().fx[0]);
        surge_effect.reset(spawn_effect(1, storage.get(), &(storage->getPatch().fx[0]), storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();

        // Because I know this
        fxstorage->p[0].set_value_f01(0.6);
        fxstorage->p[1].set_value_f01(0.54);
        fxstorage->p[2].set_value_f01(0.9);
        fxstorage->p[3].set_value_f01(0.5);
        fxstorage->p[4].set_value_f01(0);
        fxstorage->p[5].set_value_f01(1);
        fxstorage->p[6].set_value_f01(0.5);
        fxstorage->p[7].set_value_f01(0.0);
        fxstorage->p[8].set_value_f01(0.0);
        fxstorage->p[9].set_value_f01(0.0);
        fxstorage->p[10].set_value_f01(1.0);
        fxstorage->p[11].set_value_f01(0.0);

        for( int i=0; i<12; ++i )
        {
            TBase::configParam(FX_PARAM_0 + i, 0, 1, fxstorage->p[i].get_value_f01() );
        }

        INFO( "P10 = %lf %lf\n", fxstorage->p[10].get_value_f01(), fxstorage->p[10].val.f );

        INFO( "FX PTR is %x", (size_t)surge_effect.get());
        if( surge_effect.get())
        {
            INFO( "FX Type is %s", surge_effect->get_effectname() );
            for(auto i=0;i<n_fx_params;++i)
            {
                if(surge_effect->group_label(i))
                {
                    INFO( "GROUP: %d %s", surge_effect->group_label_ypos(i), surge_effect->group_label(i));
                }
            }

            std::vector<std::pair<int,int>> orderTrack;
            for(auto i=0; i<n_fx_params; ++i)
            {
                if( fxstorage->p[i].posy_offset )
                {
                    char txt[256];
                    fxstorage->p[i].get_display(txt, false, 0 );
                    INFO( "   Param[%d] -> ord=%d %s %lf %lf '%s'", i, i * 2 + fxstorage->p[i].posy_offset, fxstorage->p[i].get_name(), fxstorage->p[i].get_value_f01(), fxstorage->p[i].val.f, txt );
                    orderTrack.push_back(std::pair<int,int>(i,i * 2 + fxstorage->p[i].posy_offset));
                }
                else
                {
                    orderTrack.push_back(std::pair<int,int>(i,INT_MAX));
                }
            }
            std::sort(orderTrack.begin(), orderTrack.end(), [](const std::pair<int,int>& a, const std::pair<int,int>& b) { return a.second < b.second; } );
            for( auto a: orderTrack )
            {
                orderToParam.push_back(a.first);
                INFO( " %d -> %d", orderToParam.size() - 1, a.first  );
            }
        }

        for(auto i=0; i<BLOCK_SIZE; ++i)
        {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
            processedL[i] = 0.0f;
            processedR[i] = 0.0f;
        }

        for(auto i=0; i<BLOCK_SIZE; ++i)
        {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
            processedL[i] = 0.0f;
            processedR[i] = 0.0f;
        }
    }
#else
    SurgeFX() : TBase(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
    }
#endif


    float bufferL alignas(16)[BLOCK_SIZE], bufferR alignas(16)[BLOCK_SIZE];
    float processedL alignas(16)[BLOCK_SIZE], processedR alignas(16)[BLOCK_SIZE];
    int bufferPos = BLOCK_SIZE-1;
    
    void process(const typename TBase::ProcessArgs &args) override {
        for( int i=0; i<n_fx_params; ++i )
        { 
            fxstorage->p[orderToParam[i]].set_value_f01(params[FX_PARAM_0 + i].getValue());
        }
        
        bufferR[bufferPos] = inputs[INPUT_R_OR_MONO].getVoltage() / 5.0;
        bufferL[bufferPos] = inputs[INPUT_L].getVoltage() / 5.0; // Surge works on a +/- 1; rack works on +/- 5
        // FIXME - deal with MONO when L not hooked up

        bufferPos ++;
        if( bufferPos >= BLOCK_SIZE)
        {
            std::memcpy(processedL, bufferL, BLOCK_SIZE * sizeof(float));
            std::memcpy(processedR, bufferR, BLOCK_SIZE * sizeof(float));
            
            storage->getPatch().copy_globaldata(storage->getPatch().globaldata); 
            surge_effect->process_ringout(processedL, processedR, true);

            bufferPos = 0;
        }

        outputs[OUTPUT_R_OR_MONO].setVoltage(processedR[bufferPos] * 5.0);
        outputs[OUTPUT_L].setVoltage(processedL[bufferPos] * 5.0);
    }

    std::string getStringName(int gi) {
        int i = orderToParam[gi];
        char txt[256];
        fxstorage->p[i].get_display(txt, false, 0 );
        std::string res = fxstorage->p[i].get_name();
        res += "/ ";
        res += txt;
        return res;
    }
    bool getStringDirty(int gi) {
        return true; // fixme of course
    }
        
    
    std::unique_ptr<Effect> surge_effect;
    std::unique_ptr<SurgeStorage> storage;
    FxStorage *fxstorage;
    std::vector<int> orderToParam;
};
