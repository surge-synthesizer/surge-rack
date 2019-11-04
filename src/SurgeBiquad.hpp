#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/BiquadFilter.h"
#include "rack.hpp"
#include <cstring>
#include <cmath>

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
        configParam(FREQ_KNOB, -60, 65, 3, "Frequency", "Hz", rack::dsp::FREQ_SEMITONE, rack::dsp::FREQ_A4 );
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

        char tmp[256];

        snprintf(tmp, 256, "%8.2f Hz", 440.0 * pow(2.0, getParam(FREQ_KNOB) / 12.0 ) );
        pStrings[0].reset(tmp);

        snprintf(tmp, 256, "%7.5f", getParam(RESO_KNOB) );
        pStrings[1].reset( tmp );
        
        pStrings[2].reset("-");
    }

    float lastfr = -1, lastq = -1;

    StringCache pStrings[3];
    int updateCoeffEvery = BLOCK_SIZE;
    int lastCoeffUpdate = BLOCK_SIZE;
    int lastChans = -1;
    
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1,inputs[INPUT_L_OR_MONO].getChannels());

        if( ! ( outputs[OUTPUT_L_OR_MONO].isConnected() || outputs[OUTPUT_R].isConnected()) )
        {
            nChan = 1;
        }

        outputs[OUTPUT_L_OR_MONO].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);

        bool forceUpdate = false;
        if( nChan != lastChans )
        {
            lastChans = nChan;
            lastCoeffUpdate = BLOCK_SIZE;
            forceUpdate = true;
        }
        
        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);

        if( pc.changed( FREQ_KNOB, this ) || forceUpdate )
        {
            char tmp[256];
            snprintf(tmp, 256, "%8.2f Hz", 440.0 * pow(2.0, getParam(FREQ_KNOB) / 12.0 ) );
            pStrings[0].reset(tmp);
        }
        
        if( pc.changed( RESO_KNOB, this ) || forceUpdate )
        {
            char tmp[256];
            snprintf(tmp, 256, "%7.5f", getParam(RESO_KNOB) );
            pStrings[1].reset( tmp );
        }
        
        if( pc.changed( FILTER_TYPE, this ) )
        {
            lastCoeffUpdate = BLOCK_SIZE;
            forceUpdate = true;
        }
        
        if( pc.changed( THIRD_KNOB, this ) || forceUpdate )
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

        FilterTypesFromBiquad type = (FilterTypesFromBiquad)getParam(FILTER_TYPE);

        bool needCoeffUpdate =
            pc.changedInt(FILTER_TYPE,this) ||
            (pc.changed(FREQ_KNOB,this) || inputs[FREQ_CV].isConnected() ) ||
            (pc.changed(RESO_KNOB,this) || inputs[RESO_CV].isConnected() ) ||
            ((pc.changed(THIRD_KNOB,this) || inputs[THIRD_CV].isConnected() ) && type == peakEQ) ||
            forceUpdate;

        pc.update( this );

        for( int i=0; i<nChan; ++i )
        {
            float inl = inpG * inputs[INPUT_L_OR_MONO].getVoltage(i) * RACK_TO_SURGE_OSC_MUL;
            float inr = inpG * inputs[INPUT_R].getPolyVoltage(i) * RACK_TO_SURGE_OSC_MUL;
            if( inputConnected(INPUT_L_OR_MONO) && ! inputConnected(INPUT_R) )
            {
                inr = inl;
            }
            
            if( lastCoeffUpdate == BLOCK_SIZE && needCoeffUpdate )
            {
                float fr = getParam(FREQ_KNOB) + inputs[FREQ_CV].getPolyVoltage(i) * 12.0 ; // +/- 5 -> +/- 60
                fr = rack::clamp(fr, -60.0, 65.0); // don't allow overflows or underflows from CV
                float res = getParam(RESO_KNOB) + inputs[RESO_CV].getPolyVoltage(i) / 10.0; // +/- 5 -> +/- 0.5
                float xtra = getParam(THIRD_KNOB) + inputs[THIRD_CV].getPolyVoltage(i) / 10.0;
                
                float lightValue = 0;
                
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
            }
            
            
            float outl, outr;
            biquad[i]->process_sample( inl, inr, outl, outr );

            outl = std::isfinite(outl)? outl : 0;
            outr = std::isfinite(outr)? outr : 0;
            
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
        if( lastCoeffUpdate == BLOCK_SIZE )
            lastCoeffUpdate = 0;
        lastCoeffUpdate ++;
    }

    std::vector<std::unique_ptr<BiquadFilter>> biquad;
};
