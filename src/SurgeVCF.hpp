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

        FREQ_KNOB,
        RESO_KNOB,

        INPUT_GAIN,
        OUTPUT_GAIN,
        
        NUM_PARAMS 
    };
    enum InputIds {
        INPUT_L_OR_MONO,
        INPUT_R,
        
        FREQ_CV,
        RESO_CV,
        
        NUM_INPUTS,
    };
    enum OutputIds {
        OUTPUT_L_OR_MONO,
        OUTPUT_R,
        
        NUM_OUTPUTS
    };
    enum LightIds { NUM_LIGHTS };

    SurgeVCF() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FILTER_TYPE, 0, 9, 0 );
        configParam(FILTER_SUBTYPE, 0, 3, 0 );
        configParam(FREQ_KNOB, 0, 1, 0.5 );
        configParam(RESO_KNOB, 0, 1, 0.1 );

        configParam(INPUT_GAIN, 0, 1, 1 );
        configParam(OUTPUT_GAIN, 0, 1, 1 );
        
        setupSurge();
    }

    virtual std::string getName() override { return "VCF"; }

    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);
        resetBuffers();
    }

    void resetBuffers() {
        memset(Delay[0], 0, 4 * (MAX_FB_COMB + FIRipol_N ) * sizeof(float));
        memset(inBufferL, 0, 4 * sizeof(float));
        memset(inBufferR, 0, 4 * sizeof(float));
        memset(outBufferL, 0, 4 * sizeof(float));
        memset(outBufferR, 0, 4 * sizeof(float));
    }
    inline void set1f(__m128& m, int i, float f) {
        *((float*)&m + i) = f;
    }


    int Qe = 0;

    float inBufferL alignas(16)[4], outBufferL alignas(16)[4];
    float inBufferR alignas(16)[4], outBufferR alignas(16)[4];
    int processPos = 0;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if( pc.changed(FILTER_SUBTYPE,this) || pc.changed(FILTER_TYPE,this) )
        {
            filter = GetQFPtrFilterUnit((int)getParam(FILTER_TYPE) + 1, (int)getParam(FILTER_SUBTYPE));
            CM.Reset();
            resetBuffers();
        }
        pc.update(this);

        if( processPos == 4 )
        {
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
                set1f( Q.C[i], Qe, CM.C[i] );
                set1f( Q.dC[i], Qe, CM.dC[i] );
            }
            
            // 4. Update DB and WP somehow
            Q.DB[Qe] = Delay[Qe];
            Q.WP[Qe] = 0; // for now - deal with comb filter in a bit
            
            // 5. Set active
            Q.active[Qe] = 0xffffffff;
            
            // Increment QE
            Qe = ( Qe + 1 ) & 3;
            
            // 6. Filter - CRIB from waveshaper
            __m128 in,  out;

            in = _mm_load_ps(inBufferL);
            out = filter( &Q, in );
            _mm_store_ps(outBufferL,out);
            processPos = 0;
        }

        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);

        float inl = inpG * getInput(INPUT_L_OR_MONO) * RACK_TO_SURGE_OSC_MUL;
        float inr = inpG * getInput(INPUT_R) * RACK_TO_SURGE_OSC_MUL;

        if( inputConnected(INPUT_L_OR_MONO) && ! inputConnected(INPUT_R) )
        {
            inBufferL[processPos] = inl;
            inBufferR[processPos] = inl;
        }
        else
        {
            inBufferL[processPos] = inl;
            inBufferR[processPos] = inr;
        }

#if 0
        if( ! outputConnected(OUTPUT_R) )
        {
            setOutput(OUTPUT_L_OR_MONO, outG * (outBufferR[processPos] + outBufferL[processPos]) * 5 );
        }
        else
        {
            setOutput(OUTPUT_L_OR_MONO, outG * outBufferL[processPos]);
            setOutput(OUTPUT_R, outG * outBufferR[processPos]);
        }
#endif        
        processPos ++;
    }

    QuadFilterUnitState Q;
    float Delay[4][MAX_FB_COMB + FIRipol_N];
    FilterUnitQFPtr filter;
    FilterCoefficientMaker CM;
};
