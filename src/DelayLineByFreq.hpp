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
struct DelayLineByFreq : modules::XTModule
{
    enum ParamIds
    {
        FREQUENCY,
        CORRECTION,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        INPUT_CLOCK,
        INPUT_VOCT,
        NUM_INPUTS
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

    DelayLineByFreq() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        lineL = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        lineR = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
    }
    std::string getName() override { return "DelayLineByFreq"; }

    static constexpr size_t delayLineLength = 1 << 14;
    std::unique_ptr<SSESincDelayLine<delayLineLength>> lineL, lineR;

    void process(const ProcessArgs &args) override
    {
        // FIXME make polyphonic
        auto il = inputs[INPUT_L].getVoltage();
        auto ir = inputs[INPUT_R].getVoltage();

        auto dl = lineL->read(100);
        auto dr = lineR->read(100);

        lineL->write(il);
        lineR->write(ir);

        outputs[INPUT_L].setVoltage(dl);
        outputs[INPUT_R].setVoltage(dr);
    }
};
} // namespace sst::surgext_rack::delay
#endif // RACK_HACK_mixer_HPP
