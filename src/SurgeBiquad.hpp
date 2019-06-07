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
        configParam(FREQ_KNOB, -60, 70, 3, "Frequency", "Hz", rack::dsp::FREQ_SEMITONE, rack::dsp::FREQ_A4 );
        configParam(RESO_KNOB, 0, 1, 0.707, "Resonance" );
        configParam(THIRD_KNOB, 0, 1, 0.5 );

        configParam(INPUT_GAIN, 0, 1, 1, "Input Gain" );
        configParam(OUTPUT_GAIN, 0, 1, 1, "Output Gain" );
        
        setupSurge();
    }

    virtual std::string getName() override { return "Biquad"; }

    virtual void setupSurge() {
        setupSurgeCommon(NUM_PARAMS);
        biquad.resize(MAX_POLY);
        for( int i=0; i<MAX_POLY; ++i )
        {
            biquad[i].reset(new BiquadFilter(storage.get()));
            biquad[i]->coeff_instantize();
        }
    }

    float lastfr = -1, lastq = -1;

    StringCache pStrings[3];
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = inputs[INPUT_L_OR_MONO].getChannels();
        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);
        outputs[OUTPUT_L_OR_MONO].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);

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

        for( int i=0; i<nChan; ++i )
        {
            float inl = inpG * inputs[INPUT_L_OR_MONO].getVoltage(i) * RACK_TO_SURGE_OSC_MUL;
            float inr = inpG * inputs[INPUT_R].getPolyVoltage(i) * RACK_TO_SURGE_OSC_MUL;
            
            float fr = getParam(FREQ_KNOB) + inputs[FREQ_CV].getPolyVoltage(i) * 12.0 ; // +/- 5 -> +/- 60
            float res = getParam(RESO_KNOB) + inputs[RESO_CV].getPolyVoltage(i) / 10.0; // +/- 5 -> +/- 0.5
            float xtra = getParam(THIRD_KNOB) + inputs[THIRD_CV].getPolyVoltage(i) / 10.0;
            
            float lightValue = 0;
            
            if( inputConnected(INPUT_L_OR_MONO) && ! inputConnected(INPUT_R) )
            {
                inr = inl;
            }
            
            switch(type)
            {
            case LP:
                biquad[i]->coeff_LP(biquad[i]->calc_omega(fr/12), res);
                break;
            case LP2B:
                biquad[i]->coeff_LP2B(biquad[i]->calc_omega(fr/12), res);
                break;
            case HP:
                biquad[i]->coeff_HP(biquad[i]->calc_omega(fr/12), res);
                break;
            case BP:
                biquad[i]->coeff_BP(biquad[i]->calc_omega(fr/12), res);
                break;
            case BP2A:
                biquad[i]->coeff_BP2A(biquad[i]->calc_omega(fr/12), res);
                break;
            case PKA:
                biquad[i]->coeff_PKA(biquad[i]->calc_omega(fr/12), res);
                break;
            case NOTCH:
                biquad[i]->coeff_NOTCH(biquad[i]->calc_omega(fr/12), res);
                break;
            case APF:
                biquad[i]->coeff_APF(biquad[i]->calc_omega(fr/12), res);
                break;
            case peakEQ:
            {
                float gain = 48 * ( xtra - 0.5 );
                lightValue = 10;
                biquad[i]->coeff_peakEQ(biquad[i]->calc_omega(fr/12), res, gain);
                break;
            }
            
            default:
                biquad[i]->coeff_HP(biquad[i]->calc_omega(fr/12),res);
            }
            
            
            
            
            float outl, outr;
            biquad[i]->process_sample( inl, inr, outl, outr );
            
            if( ! outputConnected(OUTPUT_R) )
            {
                outputs[OUTPUT_L_OR_MONO].setVoltage( outG * (outl + outr) * 5 / 2, i );
            }
            else
            {
                outputs[OUTPUT_L_OR_MONO].setVoltage( outG * outl * 5, i );
                outputs[OUTPUT_R].setVoltage( outG * outr * 5, i );
            }
        }
    }

    std::vector<std::unique_ptr<BiquadFilter>> biquad;
};
