//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_DELAY_HPP
#define XTRACK_DELAY_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

#include "dsp/utilities/SSESincDelayLine.h"

namespace sst::surgext_rack::delay
{
struct Delay : modules::XTModule
{
    static constexpr int n_delay_params{5};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        TIME_L,
        TIME_R,
        FEEDBACK,
        MIX,

        DELAY_MOD_PARAM_0,

        DUMMY = DELAY_MOD_PARAM_0 + n_delay_params * n_mod_inputs,
        VCF_SUBTYPE,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        INPUT_CLOCK,

        DELAY_MOD_INPUT,
        NUM_INPUTS = DELAY_MOD_INPUT + n_mod_inputs,

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
        int offset = baseParam - TIME_L;
        return DELAY_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    modules::MonophonicModulationAssistant<Delay, n_delay_params, TIME_L, n_mod_inputs,
                                           DELAY_MOD_INPUT>
        modulationAssistant;

    Delay() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        lineL = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        lineR = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
    }
    std::string getName() override { return "Delay"; }

    static constexpr size_t delayLineLength = 1 << 19;
    std::unique_ptr<SSESincDelayLine<delayLineLength>> lineL, lineR;

    void process(const ProcessArgs &args) override
    {
        auto il = inputs[INPUT_L].getVoltage();
        auto ir = inputs[INPUT_R].getVoltage();

        auto dl = lineL->read(12000);
        auto dr = lineR->read(12000);

        auto fb = 0.4;
        auto wl = il + fb * dl;
        auto wr = ir + fb * dr;
        lineL->write(wl);
        lineR->write(wr);

        outputs[INPUT_L].setVoltage(dl);
        outputs[INPUT_R].setVoltage(dr);
    }
};
} // namespace sst::surgext_rack::delay
#endif // RACK_HACK_mixer_HPP
