//
// Created by Paul Walker on 8/29/22.
//

#ifndef SURGE_RACK_SURGEVCF_HPP
#define SURGE_RACK_SURGEVCF_HPP

#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <cstring>


struct SurgeVCF : public SurgeModuleCommon
{
    enum ParamIds
    {
        A_PARAM,
        A_MOD1_DEPTH,
        A_MOD2_DEPTH,
        NUM_PARAMS
    };
    enum InputIds
    {
        MOD_1,
        MOD_2,
        NUM_INPUTS
    };
    enum OutputIds
    {
        SIGNAL_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    SurgeVCF() : SurgeModuleCommon()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(A_PARAM, -1, 1, 0.0, "A");
        configParam(A_MOD1_DEPTH, -1, 1, 0, "AMod1");
        configParam(A_MOD2_DEPTH, -1, 1, 0, "AMod2");
        setupSurge();
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
        outputs[SIGNAL_OUT].setChannels(1);
        auto am1 = inputs[MOD_1].getPolyVoltage(0) / 5.0f;
        auto am2 = inputs[MOD_2].getPolyVoltage(0) / 5.0f;
        auto md1 = params[A_MOD1_DEPTH].getValue();
        auto md2 = params[A_MOD2_DEPTH].getValue();

        auto uv = params[A_PARAM].getValue();

        auto ov = std::clamp(uv + md1 * am1 + md2 * am2, -1.f, 1.f) * 5;
        outputs[SIGNAL_OUT].setVoltage(ov, 0);
    }
};

#endif // SURGE_RACK_SURGEVCF_HPP
