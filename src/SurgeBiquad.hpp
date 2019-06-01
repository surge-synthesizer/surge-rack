#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/BiquadFilter.h"
#include "rack.hpp"
#include <cstring>

struct SurgeBiquad :  public SurgeModuleCommon {
    enum ParamIds {
        FILTER_TYPE,

        FREQ_KNOB,
        RESO_KNOB,
        THIRD_KNOB,

        INPUT_GAIN,
        OUTPUT_GAIN,
        
        NUM_PARAMS 
    };
    enum InputIds {
        INPUT_L_OR_MONO,
        INPUT_R,
        
        FREQ_CV,
        RESO_CV,
        THIRD_CV,
        
        NUM_INPUTS,
    };
    enum OutputIds {
        OUTPUT_L_OR_MONO,
        OUTPUT_R,
        
        NUM_OUTPUTS
    };
    enum LightIds { NUM_LIGHTS };

    enum FilterTypesFromBiquad {
        LP,
        LP2B,
        HP,
        BP,
        BP2A,
        PKA,
        NOTCH,
        peakEQ,
        APF
    };
    
    SurgeBiquad() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FILTER_TYPE, 0, APF, 1 );
        configParam(FREQ_KNOB, -60, 70, 3 );
        configParam(RESO_KNOB, 0, 1, 0.707 );
        configParam(THIRD_KNOB, 0, 1, 0.5 );

        configParam(INPUT_GAIN, 0, 1, 1 );
        configParam(OUTPUT_GAIN, 0, 1, 1 );
        
        setupSurge();
    }

    virtual std::string getName() override { return "Biquad"; }

    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);
        biquad.reset(new BiquadFilter(storage.get()));
        biquad->coeff_instantize();
    }

    float lastfr = -1, lastq = -1;

    StringCache pStrings[3];
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);

        float inl = inpG * getInput(INPUT_L_OR_MONO) * RACK_TO_SURGE_OSC_MUL;
        float inr = inpG * getInput(INPUT_R) * RACK_TO_SURGE_OSC_MUL;
        if( inputConnected(INPUT_L_OR_MONO) && ! inputConnected(INPUT_R) )
        {
            inr = inl;
        }

        float fr = getParam(FREQ_KNOB) + getInput(FREQ_CV) * 12.0 ; // +/- 5 -> +/- 60
        float res = getParam(RESO_KNOB) + getInput(RESO_CV) / 10.0; // +/- 5 -> +/- 0.5
        float xtra = getParam(THIRD_KNOB) + getInput(THIRD_CV ) / 10.0;

        if( pc.changed( FREQ_KNOB, this ) )
        {
            char tmp[256];
            snprintf(tmp, 256, "%8.2f Hz", 440.0 * pow(2.0, getParam(FREQ_KNOB) / 12.0 ) );
            pStrings[0].reset(tmp);
            
        }

        if( pc.changed( RESO_KNOB, this ) )
        {
            char tmp[256];
            snprintf(tmp, 256, "%7.5f", getParam(RESO_KNOB) );
            pStrings[1].reset( tmp );
        }

        if( pc.changed( FILTER_TYPE, this ) || pc.changed( THIRD_KNOB, this ) )
        {
            if( (int)getParam(FILTER_TYPE) == peakEQ )
            {
                char tmp[256];
                snprintf(tmp, 256, "%5.2f dB", 48 * ( getParam(THIRD_KNOB) - 0.5 ) );
                pStrings[2].reset(tmp);
            }
            else
            {
                pStrings[2].resetCheck( "-" );
            }
        }

        pc.update( this );
        
        FilterTypesFromBiquad type = (FilterTypesFromBiquad)getParam(FILTER_TYPE);
        float lightValue = 0;
        switch(type)
        {
        case LP:
            biquad->coeff_LP(biquad->calc_omega(fr/12), res);
            break;
        case LP2B:
            biquad->coeff_LP2B(biquad->calc_omega(fr/12), res);
            break;
        case HP:
            biquad->coeff_HP(biquad->calc_omega(fr/12), res);
            break;
        case BP:
            biquad->coeff_BP(biquad->calc_omega(fr/12), res);
            break;
        case BP2A:
            biquad->coeff_BP2A(biquad->calc_omega(fr/12), res);
            break;
        case PKA:
            biquad->coeff_PKA(biquad->calc_omega(fr/12), res);
            break;
        case NOTCH:
            biquad->coeff_NOTCH(biquad->calc_omega(fr/12), res);
            break;
        case APF:
            biquad->coeff_APF(biquad->calc_omega(fr/12), res);
            break;
        case peakEQ:
        {
            float gain = 48 * ( xtra - 0.5 );
            lightValue = 10;
            biquad->coeff_peakEQ(biquad->calc_omega(fr/12), res, gain);
            break;
        }

        default:
            biquad->coeff_HP(biquad->calc_omega(fr/12),res);
        }



        
        float outl, outr;
        biquad->process_sample( inl, inr, outl, outr );

        if( ! outputConnected(OUTPUT_R) )
        {
            setOutput(OUTPUT_L_OR_MONO, outG * (outl + outr) * 5 / 2 );
        }
        else
        {
            setOutput(OUTPUT_L_OR_MONO, outG * outl * 5 );
            setOutput(OUTPUT_R, outG * outr * 5 );
        }
    }

    std::unique_ptr<BiquadFilter> biquad;
};
