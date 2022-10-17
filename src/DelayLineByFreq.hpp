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

#include <memory>
#include <array>

#include "dsp/utilities/SSESincDelayLine.h"

namespace sst::surgext_rack::delay
{
struct DelayLineByFreq : modules::XTModule
{
    enum ParamIds
    {
        VOCT,
        CORRECTION,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,
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
        configParam(VOCT, -5, 5, 0, "V/Oct Center");
        auto pq = configParam(CORRECTION, 0, 20, 0, "Sample Correction");
        pq->snapEnabled = true;

        for (int i = 0; i < MAX_POLY; ++i)
        {
            lineL[i] = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
            lineR[i] = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        }
    }
    std::string getName() override { return "DelayLineByFreq"; }

    static constexpr size_t delayLineLength = 1 << 14;
    std::array<std::unique_ptr<SSESincDelayLine<delayLineLength>>, MAX_POLY> lineL, lineR;

    bool isBipolar(int paramId) override
    {
        if (paramId == VOCT)
            return true;
        return false;
    }

    void process(const ProcessArgs &args) override
    {
        auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        int cc = std::max(inputs[INPUT_L].getChannels(), 1);

        outputs[OUTPUT_L].setChannels(cc);
        outputs[OUTPUT_R].setChannels(cc);

        for (int i = 0; i < cc; ++i)
        {
            auto il = inputs[INPUT_L].getVoltage(i);
            auto ir = inputs[INPUT_R].getVoltage(i);

            float pitch0 =
                (params[VOCT].getValue() + 5) * 12 + inputs[INPUT_VOCT].getVoltage(i) * 12;

            auto n2p = storage->note_to_pitch_ignoring_tuning(pitch0) * Tunings::MIDI_0_FREQ;
            auto tm = storage->samplerate / n2p - params[CORRECTION].getValue();

            auto dl = lineL[i]->read(tm);
            auto dr = lineR[i]->read(tm);

            lineL[i]->write(il);
            lineR[i]->write(ir);

            outputs[INPUT_L].setVoltage(dl, i);
            outputs[INPUT_R].setVoltage(dr, i);
        }
    }
};
} // namespace sst::surgext_rack::delay
#endif // RACK_HACK_mixer_HPP
