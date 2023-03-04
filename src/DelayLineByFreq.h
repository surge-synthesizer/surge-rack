/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef XTRACK_DELAY_HPP
#define XTRACK_DELAY_HPP

#include "SurgeXT.h"
#include "XTModule.h"
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
        setupSurgeCommon(NUM_PARAMS, false, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(VOCT, -5, 5, 0, "V/Oct Center");
        auto pq = configParam(CORRECTION, 0, 20, 0, "Sample Correction");
        pq->snapEnabled = true;

        for (int i = 0; i < MAX_POLY; ++i)
        {
            lineL[i] = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
            lineR[i] = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        }

        configInput(INPUT_L, "In Left");
        configInput(INPUT_R, "In Right");
        configInput(INPUT_VOCT, "Delay Time as Frequency in v/oct");
        configOutput(OUTPUT_L, "Out Left");
        configOutput(OUTPUT_R, "Out Right");

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
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
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        int lc = inputs[INPUT_L].getChannels();
        int rc = inputs[INPUT_R].getChannels();
        int cc = std::max({lc, rc, inputs[INPUT_VOCT].getChannels(), 1});

        // If LC or RC are 1 we want to braodcast that input to all poly channels
        // so set up a multiplier for channel in the get below
        auto lm = (lc == 1 ? 0 : 1);
        auto rm = (rc == 1 ? 0 : 1);

        outputs[OUTPUT_L].setChannels(cc);
        outputs[OUTPUT_R].setChannels(cc);

        auto rInp = inputs[INPUT_R].isConnected() ? INPUT_R : INPUT_L;

        for (int i = 0; i < cc; ++i)
        {
            auto il = inputs[INPUT_L].getVoltage(lm * i);
            auto ir = inputs[rInp].getVoltage(rm * i);

            float pitch0 =
                (params[VOCT].getValue() + 5) * 12 + inputs[INPUT_VOCT].getVoltage(i) * 12;

            auto n2p = storage->note_to_pitch_ignoring_tuning(pitch0) * Tunings::MIDI_0_FREQ;
            float tm = storage->samplerate / n2p - params[CORRECTION].getValue();

            tm = std::clamp(tm, FIRipol_N * 1.f, delayLineLength * 1.f);

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
#endif
