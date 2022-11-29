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
    typedef modules::ClockProcessor<Delay> clockProcessor_t;
    clockProcessor_t clockProc;

    float tsV(float f)
    {
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

    struct DelayTimeParamQuantity : public rack::engine::ParamQuantity
    {
        inline Delay *dm() { return static_cast<Delay *>(module); }
        virtual std::string getDisplayValueString() override
        {
            auto m = dm();
            if (!m)
                return "ERROR";

            auto v = getValue();
            if (m->tempoSync)
            {
                auto ts = m->tsV(v);
                Parameter p;
                return p.tempoSyncNotationValue(ts);
            }
            else
            {
                auto tl = m->storage->note_to_pitch_ignoring_tuning(12 * v);
                tl = std::clamp(m->storage->samplerate * tl, 0.f, delayLineLength * 1.f) *
                     m->storage->samplerate_inv;
                return fmt::format("{:7.3f} s", tl);
            }
            return "ERROR";
        }

        void setDisplayValue(float displayValue) override
        {
            auto dv = std::max(displayValue, 0.00001f);
            setValue(log2(dv));
        }
    };

    struct QuadRateParamQuantity : public rack::engine::ParamQuantity
    {
        virtual std::string getDisplayValueString() override
        {
            auto v = getValue();
            return fmt::format("{:6.2f} Hz", v * v);
        }

        void setDisplayValue(float displayValue) override
        {
            auto dv = std::max(displayValue, 0.f);
            setValue(sqrt(dv));
        }
    };
    Delay() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<DelayTimeParamQuantity>(TIME_L, log2(0.01), log2(10.0), 0, "Left Delay");
        configParam<DelayTimeParamQuantity>(TIME_R, log2(0.01), log2(10.0), 0, "Right Delay");
        configParam(TIME_S, -1, 1, 0, "Time Tweak", "%", 0, 2);
        configParam(FEEDBACK, 0, 1, .5, "Feedback", "%", 0, 100);
        configParam(CROSSFEED, 0, 1, 0, "CrossFeed", "%", 0, 100);
        configParam<modules::MidiNoteParamQuantity<69>>(LOCUT, -60, 70, -60, "LoCut");
        configParam<modules::MidiNoteParamQuantity<69>>(HICUT, -60, 70, 70, "HiCut");

        configParam<QuadRateParamQuantity>(MODRATE, 0, 4, 2, "ModRate"); // 0 - 16 hz quadratic
        configParam(MODDEPTH, 0, 1, 0, "ModDepth", "%", 0, 100);
        configParam(MIX, 0, 1, 1, "Mix", "%", 0, 100);

        for (int i = 0; i < n_delay_params * n_mod_inputs; ++i)
        {
            int tp = paramModulatedBy(i + DELAY_MOD_PARAM_0);
            auto lb = paramQuantities[tp]->getLabel();
            std::string name = std::string("Mod ") + std::to_string(i % 4 + 1) + " to " + lb;

            configParamNoRand(DELAY_MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
        }

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        configInput(INPUT_CLOCK, "Clock/BPM Input");
        for (int i = 0; i < n_mod_inputs; ++i)
            configInput(DELAY_MOD_INPUT + i, std::string("Mod ") + std::to_string(i + 1));
        configOutput(OUTPUT_L, "Left");
        configOutput(OUTPUT_R, "Right");

        lineL = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        lineR = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);

        lpPost = std::make_unique<BiquadFilter>(storage.get());
        lpPost->suspend();
        hpPost = std::make_unique<BiquadFilter>(storage.get());
        hpPost->suspend();

        modulationAssistant.initialize(this);

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
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

    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - DELAY_MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_delay_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs;
    }

    bool tempoSync{false};
    void activateTempoSync() { tempoSync = true; }
    void deactivateTempoSync() { tempoSync = false; }

    static constexpr int slowUpdate{8};
    int blockPos{0};
    float tsL{0}, tsR{0};
    float modVal{0}, dMod{0}, modPhase{0};
    void process(const ProcessArgs &args) override
    {
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

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
                tsL = tsV(params[TIME_L].getValue());
                /*Parameter p;
                std::cout << tsL << " " << p.tempoSyncNotationValue(tsL) << std::endl;
                */
                tsR = tsV(params[TIME_R].getValue());
            }
        }
        modulationAssistant.updateValues(this);
        auto il = inputs[INPUT_L].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
        auto ir = inputs[INPUT_R].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;

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
        tl = std::clamp(tl, 0.f, delayLineLength * 1.f);
        tr = std::clamp(tr, 0.f, delayLineLength * 1.f);
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
        clockProc.toJson(fx);
        return fx;
    }

    void readModuleSpecificJson(json_t *modJ) override { clockProc.fromJson(modJ); }
};
} // namespace sst::surgext_rack::delay
#endif
