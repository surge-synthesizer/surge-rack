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
#include "BiquadFilter.h"

#include "dsp/utilities/SSESincDelayLine.h"

namespace sst::surgext_rack::delay
{
struct Delay : modules::XTModule
{
    static constexpr int n_delay_params{10};
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        TIME_L,
        TIME_R,
        TIME_S,
        FEEDBACK,
        CROSSFEED,

        LOCUT,
        HICUT,

        MODRATE,
        MODDEPTH,
        MIX,

        DELAY_MOD_PARAM_0,

        NUM_PARAMS = DELAY_MOD_PARAM_0 + n_delay_params * n_mod_inputs,
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

        configParam(TIME_L, -3, 5, 0, "Left Delay");
        configParam(TIME_R, -3, 5, 0, "Left Delay");
        configParam(TIME_S, -2, 2, 0, "Time Tweak");
        configParam(FEEDBACK, 0, 1, .5, "Feedback");
        configParam(CROSSFEED, 0, 1, 0, "CrossFeed");
        configParam(LOCUT, -60, 70, -60, "LoCut");
        configParam(HICUT, -60, 70, 70, "HiCut");

        configParam(MODRATE, -7, 9, 0, "ModRate");
        configParam(MODDEPTH, 0, 1, 0, "ModDepth");
        configParam(MIX, 0, 1, 1, "Mix");

        for (int i = 0; i < n_delay_params * n_mod_inputs; ++i)
        {
            configParam(DELAY_MOD_PARAM_0 + i, -1, 1, 0);
        }

        lineL = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        lineR = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);

        lpPost = std::make_unique<BiquadFilter>(storage.get());
        lpPost->suspend();
        hpPost = std::make_unique<BiquadFilter>(storage.get());
        hpPost->suspend();

        modulationAssistant.initialize(this);
    }
    std::string getName() override { return "Delay"; }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - TIME_L;
        if (idx < 0 || idx >= n_delay_params)
            return 0;
        return modulationAssistant.modvalues[idx] * modulationAssistant.fInv[idx];
    }

    static constexpr size_t delayLineLength = 1 << 19;
    std::unique_ptr<SSESincDelayLine<delayLineLength>> lineL, lineR;
    std::unique_ptr<BiquadFilter> lpPost, hpPost;

    static constexpr int slowUpdate{8};
    int blockPos{0};
    void process(const ProcessArgs &args) override
    {
        if (blockPos == slowUpdate)
        {
            modulationAssistant.setupMatrix(this);
            blockPos = 0;

            lpPost->coeff_LP2B(lpPost->calc_omega(modulationAssistant.values[HICUT] / 12.0), 0.707);

            hpPost->coeff_HP(lpPost->calc_omega(modulationAssistant.values[LOCUT] / 12.0), 0.707);
        }
        modulationAssistant.updateValues(this);
        // TODO
        /*

    TIME_S
    MODRATE,
    MODDEPTH,

    MIX,
         */
        auto il = inputs[INPUT_L].getVoltage() * RACK_TO_SURGE_OSC_MUL;
        auto ir = inputs[INPUT_R].getVoltage() * RACK_TO_SURGE_OSC_MUL;

        auto wobble = 1.0 + 0.01 * modulationAssistant.values[TIME_S];
        // FIXME - temposync
        auto tl = storage->samplerate * storage->note_to_pitch_ignoring_tuning(
                                            12 * wobble * modulationAssistant.values[TIME_L]);
        auto tr = storage->samplerate * storage->note_to_pitch_ignoring_tuning(
                                            12 * wobble * modulationAssistant.values[TIME_R]);

        auto dl = std::clamp(lineL->read(tl), -1.5f, 1.5f);
        auto dr = std::clamp(lineR->read(tr), -1.5f, 1.5f);

        // softclip
        dl = dl - 4.0 / 27.0 * dl * dl * dl;
        dr = dr - 4.0 / 27.0 * dr * dr * dr;

        float fb = modulationAssistant.values[FEEDBACK];
        float cf = modulationAssistant.values[CROSSFEED];
        float wl = il + fb * dl + cf * dr;
        float wr = ir + fb * dr + cf * dl;

        lpPost->process_sample(wl, wr, wl, wr);
        hpPost->process_sample(wl, wr, wl, wr);
        lineL->write(wl);
        lineR->write(wr);

        auto mx = modulationAssistant.values[MIX];
        outputs[OUTPUT_L].setVoltage((mx * dl + (1 - mx) * il) * SURGE_TO_RACK_OSC_MUL);
        outputs[OUTPUT_R].setVoltage((mx * dr + (1 - mx) * ir) * SURGE_TO_RACK_OSC_MUL);
        blockPos++;
    }
};
} // namespace sst::surgext_rack::delay
#endif // RACK_HACK_mixer_HPP
