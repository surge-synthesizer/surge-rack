#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/QuadFilterUnit.h"
#include "dsp/FilterCoefficientMaker.h"
#include "rack.hpp"
#include <cstring>

struct SurgeVCF :  public SurgeModuleCommon {
    enum ParamIds {
        FILTER_TYPE,
        FILTER_SUBTYPE,
        NUM_PARAMS 
    };
    enum InputIds {
        NUM_INPUTS,
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds { NUM_LIGHTS };

    ParamCache pc;

#if RACK_V1
    SurgeVCF() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FILTER_TYPE, 0, 9, 0 );
        configParam(FILTER_SUBTYPE, 0, 3, 0 );
        setupSurge();
    }
#else
    SurgeVCF()
        : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    virtual std::string getName() override { return "VCF"; }

    virtual void setupSurge() {
        pc.resize(NUM_PARAMS);
        setupSurgeCommon();
    }

    inline void set1f(__m128& m, int i, float f) {
        *((float*)&m + i) = f;
    }


#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        if( pc.changed(FILTER_SUBTYPE,this) || pc.changed(FILTER_TYPE,this) )
        {
            filter = GetQFPtrFilterUnit((int)getParam(FILTER_TYPE) + 1, (int)getParam(FILTER_SUBTYPE));
            CM.Reset();
            INFO( "FILTER is %x", (size_t)filter);
        }
        pc.update(this);

        /*
          Strategy here is
          1. Have a coefficient maker
          2. have it make coefficients
          3. blat those onto the state
          4. Update DB and WP somehow (how??) with special case for moog
         */

        // 1/2: have coeffieicnt maker make values
        float freq = 440;
        float res = 20;
        CM.MakeCoeffs(freq, res, (int)getParam(FILTER_TYPE) + 1, (int)getParam(FILTER_SUBTYPE));

        // 3: blat those onto the state
        for( auto i=0; i<n_cm_coeffs; ++i )
        {
            // This '0' is obviously wrong - look at SurgeVoice::process_block for Qe
            set1f( Q.C[i], 0, CM.C[i] );
            set1f( Q.dC[i], 0, CM.dC[i] );
        }

        // 4. Update DB and WP somehow

        // 5. Set active

        // 6. Filter
        //out = filter( *Q, in );
    }

    QuadFilterUnitState Q;
    FilterUnitQFPtr filter;
    FilterCoefficientMaker CM;
};
