//
// Created by Paul Walker on 8/29/22.
//

#ifndef SURGE_RACK_SURGEVCF_HPP
#define SURGE_RACK_SURGEVCF_HPP

#include "SurgeXT.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <cstring>


struct VCF : public SurgeModuleCommon
{
    static constexpr int n_vcf_params{5};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        FREQUENCY,
        RESONANCE,
        IN_GAIN,
        MIX,
        OUT_GAIN,

        VCF_MOD_PARAM_0,

        NUM_PARAMS =  VCF_MOD_PARAM_0 + n_vcf_params * n_mod_inputs
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,

        VCF_MOD_INPUT,
        NUM_INPUTS = VCF_MOD_INPUT + n_mod_inputs,

    };
    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - FREQUENCY;
        return VCF_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }


    VCF() : SurgeModuleCommon()
    {
        setupSurgeCommon(NUM_PARAMS);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(FREQUENCY,0,1,0);
        configParam(RESONANCE,0,1,0);
        configParam(IN_GAIN,0,1,0);
        configParam(MIX,0,1,0);
        configParam(OUT_GAIN,0,1,0);

        for (int i=0; i<n_vcf_params * n_mod_inputs; ++i)
        {
            configParam(VCF_MOD_PARAM_0 + i, -1, 1, 0);
        }
    }

    virtual std::string getName() override { return "VCF"; }

    virtual void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS);
        for (int i = 0; i < MAX_POLY; ++i)
            processPosition[i] = 0;
    }

    int processPosition[MAX_POLY];
    float inBuffer alignas(16)[MAX_POLY][4], outBuffer alignas(16)[MAX_POLY][4];

    void process(const typename rack::Module::ProcessArgs &args) override
    {
    }
};

#endif // SURGE_RACK_SURGEVCF_HPP
