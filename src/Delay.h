/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_SRC_DELAY_H
#define SURGE_XT_RACK_SRC_DELAY_H

#include "SurgeXT.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"
#include "BiquadFilter.h"

#include "dsp/utilities/SSESincDelayLine.h"
#include "TemposyncSupport.h"
#include "sst/rackhelpers/neighbor_connectable.h"

namespace sst::surgext_rack::delay
{

struct Delay : modules::XTModule, sst::rackhelpers::module_connector::NeighborConnectable_V1
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

        CLIP_MODE_PARAM = DELAY_MOD_PARAM_0 + n_delay_params * n_mod_inputs,
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

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        return {{std::make_pair("Input", std::make_pair(INPUT_L, INPUT_R))}};
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{std::make_pair("Output", std::make_pair(OUTPUT_L, OUTPUT_R))}};
    }

    modules::MonophonicModulationAssistant<Delay, n_delay_params, TIME_L, n_mod_inputs,
                                           DELAY_MOD_INPUT>
        modulationAssistant;
    typedef modules::ClockProcessor<Delay> clockProcessor_t;
    clockProcessor_t clockProc;

    enum ClipMode
    {
        TRANSPARENT,
        SOFTCLIP_DELAYLINE_5V,
        SOFTCLIP_DELAYLINE_10V,
        HARDCLIP_DELAYLINE_10V
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
                return temposync_support::temposyncLabel(v);
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
        setupSurgeCommon(NUM_PARAMS, false, false);
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

        configParamNoRand(CLIP_MODE_PARAM, TRANSPARENT, HARDCLIP_DELAYLINE_10V,
                          HARDCLIP_DELAYLINE_10V, "Clip Mode");

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        configInput(INPUT_CLOCK, "Clock/BPM");
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
    ClipMode currentClipMode{HARDCLIP_DELAYLINE_10V};

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

            currentClipMode = (ClipMode)std::round(params[CLIP_MODE_PARAM].getValue());

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
                tsL = temposync_support::roundTemposync(params[TIME_L].getValue());
                /*Parameter p;
                std::cout << tsL << " " << p.tempoSyncNotationValue(tsL) << std::endl;
                */
                tsR = temposync_support::roundTemposync(params[TIME_R].getValue());
            }
        }
        modulationAssistant.updateValues(this);
        auto il = inputs[INPUT_L].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
        auto ir = inputs[INPUT_R].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;

        if (!inputs[INPUT_R].isConnected())
            ir = il;

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

        auto dl = lineL->read(tl);
        auto dr = lineR->read(tr);
        auto wl{0.f}, wr{0.f};

        float fb = modulationAssistant.values[FEEDBACK];
        float cf = modulationAssistant.values[CROSSFEED];

        switch (currentClipMode)
        {
        case TRANSPARENT:
            break;
        case HARDCLIP_DELAYLINE_10V:
            // write the clean signal, clamp the output at 10v
            dl = std::clamp(dl, -2.f, 2.f); // 10V
            dr = std::clamp(dr, -2.f, 2.f);
            break;
        case SOFTCLIP_DELAYLINE_5V:
            // Write the clean signal softclip the output
            dl = std::clamp(dl, -1.5f, 1.5f); // 5V since this yields +/- 1
            dr = std::clamp(dr, -1.5f, 1.5f);
            dl = dl - 4.0f / 27.0f * dl * dl * dl;
            dr = dr - 4.0f / 27.0f * dr * dr * dr;
            break;
        case SOFTCLIP_DELAYLINE_10V:
            // Write the clean signal softclip the output
            dl = std::clamp(dl, -3.f, 3.f); // 10V since this yields +/- 2
            dr = std::clamp(dr, -3.f, 3.f);
            dl = dl * 0.5;
            dr = dr * 0.5;
            dl = 2.0f * dl - 8.0f / 27.0f * dl * dl * dl;
            dr = 2.0f * dr - 8.0f / 27.0f * dr * dr * dr;
            break;
        }

        wl = il + fb * dl + cf * dr;
        wr = ir + fb * dr + cf * dl;

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

    void moduleSpecificSampleRateChange() override
    {
        clockProc.setSampleRate(APP->engine->getSampleRate());
    }
};
} // namespace sst::surgext_rack::delay
#endif
