#pragma once

#include "dsp/effect/Effect.h"

template <typename TBase>
struct SurgeFX : virtual TBase
{
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_R_OR_MONO,
        INPUT_L,
        NUM_INPUTS
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
        INFO( "Making FX" );
        // TODO: Have a mode where these paths come from res/
        storage.reset(new SurgeStorage());
        fxstorage.reset(new FxStorage());
        fxstorage->type.val.i = 1;
        INFO( "Storage datapath is: %s", storage->datapath.c_str() );
        
        surge_effect.reset(spawn_effect(1, storage.get(), fxstorage.get(), 0));
        surge_effect->init_ctrltypes();
        INFO( "FX PTR is %x", (size_t)surge_effect.get());
        if( surge_effect.get())
        {
            INFO( "FX Type is %s", surge_effect->get_effectname() );
            for(auto i=0; i<n_fx_params; ++i)
                INFO( "   Param[%d] -> %s %lf", i, fxstorage->p[i].get_name(), fxstorage->p[i].get_value_f01() );
        }
    }
#else
    SurgeFX() : TBase(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
    }
#endif

    std::unique_ptr<Effect> surge_effect;
    std::unique_ptr<SurgeStorage> storage;
    std::unique_ptr<FxStorage> fxstorage;
};
