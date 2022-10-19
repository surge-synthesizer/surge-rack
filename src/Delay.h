//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_DELAY_HPP
#define XTRACK_DELAY_HPP

#include "SurgeXT.h"
#include "XTModule.h"
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
    modules::ClockProcessor<Delay> clockProc;

    Delay() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(TIME_L, -3, 5, 0, "Left Delay");
        configParam(TIME_R, -3, 5, 0, "Right Delay");
        configParam(TIME_S, -1, 1, 0, "Time Tweak");
        configParam(FEEDBACK, 0, 1, .5, "Feedback");
        configParam(CROSSFEED, 0, 1, 0, "CrossFeed");
        configParam(LOCUT, -60, 70, -60, "LoCut");
        configParam(HICUT, -60, 70, 70, "HiCut");

        configParam(MODRATE, 0, 4, 2, "ModRate");
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

    bool tempoSync{false};
    void activateTempoSync() { tempoSync = true; }
    void deactivateTempoSync() { tempoSync = false; }

    static constexpr int slowUpdate{8};
    int blockPos{0};
    float tsL{0}, tsR{0};
    float modVal{0}, dMod{0}, modPhase{0};
    void process(const ProcessArgs &args) override
    {
        auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        if (inputs[INPUT_CLOCK].isConnected())
            clockProc.process(this, INPUT_CLOCK);
        else
            clockProc.disconnect(this);

        if (blockPos == slowUpdate)
        {
            modulationAssistant.setupMatrix(this);
            blockPos = 0;

            lpPost->coeff_LP2B(lpPost->calc_omega(modulationAssistant.values[HICUT] / 12.0), 0.707);
            hpPost->coeff_HP(lpPost->calc_omega(modulationAssistant.values[LOCUT] / 12.0), 0.707);

            auto modFreq = std::clamp(modulationAssistant.values[MODRATE], 0.f, 4.f);
            modFreq = modFreq * modFreq;
            // 0 -> 16 hz
            auto dPhase = slowUpdate * storage->samplerate_inv * modFreq;
            modPhase += dPhase;
            if (modPhase > 1)
                modPhase -= 1;
            float modTarget = std::sin(modPhase * 2.0 * M_PI);
            dMod = (modTarget - modVal) / slowUpdate;

            if (tempoSync)
            {
                auto tsV = [](float f) {
                    float a, b = modff(f, &a);
                    if (b < 0)
                    {
                        b += 1.f;
                        a -= 1.f;
                    }
                    b = powf(2.0f, b);

                    if (b > 1.41f)
                    {
                        b = log2(1.5f);
                    }
                    else if (b > 1.167f)
                    {
                        b = log2(1.3333333333f);
                    }
                    else
                    {
                        b = 0.f;
                    }
                    return a + b;
                };
                tsL = tsV(params[TIME_L].getValue());
                /*Parameter p;
                std::cout << tsL << " " << p.tempoSyncNotationValue(tsL) << std::endl;
                */
                tsR = tsV(params[TIME_R].getValue());
            }
        }
        modulationAssistant.updateValues(this);
        auto il = inputs[INPUT_L].getVoltage() * RACK_TO_SURGE_OSC_MUL;
        auto ir = inputs[INPUT_R].getVoltage() * RACK_TO_SURGE_OSC_MUL;

        modVal += dMod;

        auto wobble = 1.0 + 0.02 * modulationAssistant.values[TIME_S] +
                      0.005 * modulationAssistant.values[MODDEPTH] * modVal;
        // FIXME - temposync
        float tl{0.f}, tr{0.f};
        if (tempoSync)
        {
            auto tvl = 12 * wobble * (tsL + modulationAssistant.modvalues[TIME_L]);
            tl = storage->samplerate * storage->temposyncratio_inv *
                 storage->note_to_pitch_ignoring_tuning(tvl);
            auto tvr = 12 * wobble * (tsR + modulationAssistant.modvalues[TIME_R]);
            tr = storage->samplerate * storage->temposyncratio_inv *
                 storage->note_to_pitch_ignoring_tuning(tvr);
        }
        else
        {
            tl = storage->samplerate * storage->note_to_pitch_ignoring_tuning(
                                           12 * wobble * modulationAssistant.values[TIME_L]);
            tr = storage->samplerate * storage->note_to_pitch_ignoring_tuning(
                                           12 * wobble * modulationAssistant.values[TIME_R]);
        }
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

    json_t *makeModuleSpecificJson() override
    {
        auto fx = json_object();
        json_object_set(fx, "clockStyle", json_integer((int)clockProc.clockStyle));
        return fx;
    }

    void readModuleSpecificJson(json_t *modJ) override
    {
        auto cs = json_object_get(modJ, "clockStyle");
        if (cs)
        {
            auto csv = json_integer_value(cs);
            clockProc.clockStyle =
                static_cast<typename modules::ClockProcessor<Delay>::ClockStyle>(csv);
        }
    }
};
} // namespace sst::surgext_rack::delay
#endif
