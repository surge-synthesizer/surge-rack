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
#include "BiquadFilter.h"

/*
 * FB Filter
 * Display Area
 * LintBuddy
 */

namespace sst::surgext_rack::delay
{

struct DelayLineByFreqExpanded : modules::XTModule
{
    static constexpr int n_mod_inputs{4};
    static constexpr int n_mod_params{7};
    enum ParamIds
    {
        // Modulatable Set
        VOCT,
        VOCT_FINE_LEFT,
        VOCT_FINE_RIGHT,
        FB_ATTENUATION,
        FILTER_LP_CUTOFF_DIFF,
        FILTER_HP_CUTOFF_DIFF,
        FILTER_MIX,

        // Non-modulatable set
        FILTER_LP_ON,
        FILTER_HP_ON,
        CORRECTION,
        FB_EXTEND,

        MOD_PARAM_0,

        CLAMP_BEHAVIOR = MOD_PARAM_0 + n_mod_params * n_mod_inputs,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        INPUT_VOCT,
        INPUT_FBL,
        INPUT_FBR,
        INPUT_EXCITER_AMP,

        MOD_INPUT_0,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs
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

    enum ClampBehavior
    {
        HARD_20 = 0,
        HARD_10,
        HARD_5
    } clampBehavior{HARD_10};
    float clampLevel{10.f};

    // (2^(-2/6.02))^(1/3) so a 2db range when attenuated
    // static constexpr float attenBase{0.92611164457}, attenScale{1.0f - attenBase};
    // x = 1-sqrt(1-(2^(-1/6.02))) so a 2db range when attenuated
    static constexpr float attenBase{0.67021327021}, attenScale{1.0f - attenBase};

    struct FBAttenPQ : rack::ParamQuantity
    {
        std::string getDisplayValueString() override
        {
            auto m = module;
            auto p = getParam();
            if (!m || !p)
                return {};

            auto rbr = m->params[FB_EXTEND].getValue() > 0.5;
            auto v = getValue();

            if (!rbr)
            {
                v = v * attenScale + attenBase;
            }
            // v = v * v * v;
            v = 1 - v;
            v = 1 - v * v;
            if (v < 0.0001)
                return "-inf dB";
            auto dbv = 6.02 * std::log2(v);
            return fmt::format("{:.4} dB", dbv);
        }

        void setDisplayValueString(std::string s) override
        {
            if (s.find("-inf") != std::string::npos)
            {
                setValue(0.f);
                return;
            }

            auto q = std::atof(s.c_str());
            auto v = pow(2.f, q / 6.02);

            // v = pow(v, 1.0 / 3.0);
            v = 1 - sqrt(1 - v);

            auto m = module;
            if (!m)
                return;
            auto rbr = m->params[FB_EXTEND].getValue() > 0.5;
            if (!rbr)
            {
                v = (v - attenBase) / attenScale;
            }

            setValue(v);
        }
    };
    DelayLineByFreqExpanded() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(VOCT, -5, 5, 0, "V/Oct Center");
        auto pq = configParam(CORRECTION, 0, 20, 1, "Sample Correction");
        pq->snapEnabled = true;

        for (int i = 0; i < MAX_POLY; ++i)
        {
            lineL[i] = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
            lineR[i] = std::make_unique<SSESincDelayLine<delayLineLength>>(storage->sinctable);
        }

        configParam(VOCT_FINE_LEFT, -100, 100, 0, "Fine Left Tune", " Cents");
        configParam(VOCT_FINE_RIGHT, -100, 100, 0, "Fine Left Tune", " Cents");

        configParam<FBAttenPQ>(FB_ATTENUATION, 0, 1, 0.98, "Feedback Level");
        configParam(FILTER_LP_CUTOFF_DIFF, -80, 30, 30, "LP Cutoff to Pitch Offset", " Semitones");
        configParam(FILTER_HP_CUTOFF_DIFF, -110, 0, -110, "HP Cutoff to Pitch Offset",
                    " Semitones");
        configParam(FILTER_MIX, 0, 1, 1, "Signal/Filter Wet/Dry Mix");

        configOnOff(FILTER_LP_ON, 0, "LowPass Filter Active");
        configOnOff(FILTER_HP_ON, 0, "HighPass Filter Active");

        configSwitch(FB_EXTEND, 0, 1, 0, "Feedback Range",
                     {"Compact Range (90-100% for Waveguide)", "Full range (0-100%)"});
        configSwitch(CLAMP_BEHAVIOR, HARD_20, HARD_5, HARD_10, "Clamp Beeavior",
                     {"Hard Clamp @ +/-20V", "Hard Clamp @ +/- 10V", "Hard Clamp @ +/- 5V"});

        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
        {
            int tp = paramModulatedBy(i + MOD_PARAM_0);
            auto lb = paramQuantities[tp]->getLabel();
            std::string name = std::string("Mod ") + std::to_string(i % 4 + 1) + " to " + lb;

            configParamNoRand(MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
        }

        configInput(INPUT_L, "In Left");
        configInput(INPUT_R, "In Right");
        configInput(INPUT_VOCT, "Delay Time as Frequency in v/oct");

        configInput(INPUT_FBL, "Feedback Input Left");
        configInput(INPUT_FBR, "Feedback Input RIGHT");
        configInput(INPUT_EXCITER_AMP, "Noise Source Amplitude");

        for (int i = 0; i < n_mod_inputs; ++i)
        {
            configInput(MOD_INPUT_0 + i, "Mod " + std::to_string(i + 1) + " Input");
        }

        configOutput(OUTPUT_L, "Out Left");
        configOutput(OUTPUT_R, "Out Right");

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);

        gen = std::default_random_engine();
        gen.seed(storage->rand_u32());
        distro = std::uniform_real_distribution<float>(-1.f, 1.f);

        for (int i = 0; i < MAX_POLY; ++i)
        {
            lpFB[i] = std::make_unique<BiquadFilter>(storage.get());
            lpFB[i]->suspend();
            hpFB[i] = std::make_unique<BiquadFilter>(storage.get());
            hpFB[i]->suspend();

            vuLevel[i] = 0.f;
        }

        modAssist.initialize(this);
    }
    std::string getName() override { return "DelayLineByFreqExpanded"; }

    std::default_random_engine gen;
    std::uniform_real_distribution<float> distro;
    static constexpr size_t delayLineLength = 1 << 14;
    std::array<std::unique_ptr<SSESincDelayLine<delayLineLength>>, MAX_POLY> lineL, lineR;

    modules::ModulationAssistant<DelayLineByFreqExpanded, n_mod_params, VOCT, n_mod_inputs,
                                 MOD_INPUT_0>
        modAssist;

    std::array<std::unique_ptr<BiquadFilter>, MAX_POLY> lpFB, hpFB;

    bool isBipolar(int paramId) override
    {
        if (paramId == VOCT || paramId == VOCT_FINE_LEFT || paramId == VOCT_FINE_RIGHT)
            return true;
        return false;
    }

    int nChan{1};
    int processCount{BLOCK_SIZE};
    int polyChannelCount() { return nChan; }
    bool useLP{false}, useHP{false};
    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - VOCT;
        return MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }
    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_mod_params) || offset < 0)
            return -1;
        return offset / n_mod_inputs + VOCT;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - VOCT;
        if (idx < 0 || idx >= n_mod_params)
            return 0;
        return modAssist.animValues[idx];
    }

    float vuLevel[MAX_POLY];
    float vuFalloff{0.999};

    void process(const ProcessArgs &args) override
    {
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();
        int lc = inputs[INPUT_L].getChannels();
        int rc = inputs[INPUT_R].getChannels();

        int lf = inputs[INPUT_FBL].getChannels();
        int rf = inputs[INPUT_FBR].getChannels();

        auto fbr = params[FB_EXTEND].getValue() > 0.5;

        if (processCount == BLOCK_SIZE)
        {
            int cc = std::max({lc, rc, inputs[INPUT_VOCT].getChannels(), 1});
            nChan = cc;

            modAssist.setupMatrix(this);
            modAssist.updateValues(this);

            auto cv = (ClampBehavior)params[CLAMP_BEHAVIOR].getValue();
            switch (cv)
            {
            case HARD_20:
                clampLevel = 20;
                break;
            case HARD_10:
                clampLevel = 10;
                break;
            case HARD_5:
                clampLevel = 5;
                break;
            }

            // Filter Coefficients go here
            auto tLP = params[FILTER_LP_ON].getValue() > 0.5;
            auto tHP = params[FILTER_HP_ON].getValue() > 0.5;
            bool lpToggle{false}, hpToggle{false};
            if (tLP != useLP)
            {
                for (int p = 0; p < MAX_POLY; ++p)
                {
                    lpFB[p]->suspend();
                }
                useLP = tLP;
                lpToggle = true;
            }

            if (tHP != useHP)
            {
                for (int p = 0; p < MAX_POLY; ++p)
                {
                    hpFB[p]->suspend();
                }
                useHP = tHP;
                hpToggle = true;
            }

            for (int i = 0; i < nChan; ++i)
            {
                float pitch0 =
                    (modAssist.values[VOCT][i] + 5) * 12 + inputs[INPUT_VOCT].getVoltage(i) * 12;

                if (useLP)
                {
                    auto pval = pitch0 + modAssist.values[FILTER_LP_CUTOFF_DIFF][i];
                    lpFB[i]->coeff_LP2B(lpFB[i]->calc_omega(pval / 12.0), 0.707);
                    if (lpToggle)
                        lpFB[i]->coeff_instantize();
                }
                if (useHP)
                {
                    auto pval = pitch0 + modAssist.values[FILTER_HP_CUTOFF_DIFF][i];
                    hpFB[i]->coeff_HP(hpFB[i]->calc_omega(pval / 12.0), 0.707);
                    if (hpToggle)
                        hpFB[i]->coeff_instantize();
                }
            }

            processCount = 0;
        }
        else
        {
            // Do this every one so we can do like voct fm and stuff
            modAssist.updateValues(this);
        }

        // If LC or RC are 1 we want to braodcast that input to all poly channels
        // so set up a multiplier for channel in the get below
        auto lm = (lc == 1 ? 0 : 1);
        auto rm = (rc == 1 ? 0 : 1);

        auto lfm = (lf == 1 ? 0 : 1);
        auto rfm = (rf == 1 ? 0 : 1);

        outputs[OUTPUT_L].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);

        auto rInput = inputs[INPUT_R].isConnected() ? INPUT_R : INPUT_L;
        if (rInput == INPUT_L)
            rm = lm;

        auto rFbInput = inputs[INPUT_FBR].isConnected() ? INPUT_FBR : INPUT_FBL;
        if (rFbInput == INPUT_FBL)
            rfm = lfm;

        for (int i = 0; i < nChan; ++i)
        {
            float pitch0 =
                (modAssist.values[VOCT][i] + 5) * 12 + inputs[INPUT_VOCT].getVoltage(i) * 12;

            auto n2pL = storage->note_to_pitch_inv_ignoring_tuning(
                            pitch0 + modAssist.values[VOCT_FINE_LEFT][i] * 0.01) /
                        Tunings::MIDI_0_FREQ;
            auto n2pR = storage->note_to_pitch_inv_ignoring_tuning(
                            pitch0 + modAssist.values[VOCT_FINE_RIGHT][i] * 0.01) /
                        Tunings::MIDI_0_FREQ;
            float tmL = storage->samplerate * n2pL - params[CORRECTION].getValue();
            float tmR = storage->samplerate * n2pR - params[CORRECTION].getValue();

            tmL = std::clamp(tmL, FIRipol_N * 1.f, (delayLineLength - FIRipol_N) * 1.f);
            tmR = std::clamp(tmR, FIRipol_N * 1.f, (delayLineLength - FIRipol_N) * 1.f);

            auto dl = lineL[i]->read(tmL);
            auto dr = lineR[i]->read(tmR);

            auto fba = modAssist.values[FB_ATTENUATION][i];
            if (!fbr)
            {
                // 0 -> .9
                // 1 -> 1
                // fba * 0.1 + 0.9
                fba = std::clamp(fba * attenScale + attenBase, 0.0f, 1.0f);
            }
            // fba = fba * fba * fba
            auto omfba = 1.f - fba;
            fba = 1.f - omfba * omfba;

            auto fbl = fba * inputs[INPUT_FBL].getVoltage(lfm * i);
            auto fbr = fba * inputs[rFbInput].getVoltage(rfm * i);

            float ex = inputs[INPUT_EXCITER_AMP].getVoltage(i) * 0.1;
            if (ex > 1e-5 && inputs[INPUT_EXCITER_AMP].isConnected())
            {
                fbl += ex * distro(gen);
                fbr += ex * distro(gen);
            }

            if (useHP || useLP)
            {
                auto fvl = fbl;
                auto fvr = fbr;

                auto mv = modAssist.values[FILTER_MIX][i];
                mv = mv * mv * mv;

                if (useLP)
                {
                    lpFB[i]->process_sample(fvl, fvr, fvl, fvr);
                }

                if (useHP)
                {
                    hpFB[i]->process_sample(fvl, fvr, fvl, fvr);
                }

                fbl = mv * fvl + (1 - mv) * fbl;
                fbr = mv * fvr + (1 - mv) * fbr;
            }

            auto il = inputs[INPUT_L].getVoltage(lm * i) + fbl;
            auto ir = inputs[rInput].getVoltage(rm * i) + fbr;

            // avoid feedback blowouts with a hard clamp
            lineL[i]->write(std::clamp(il, -clampLevel, clampLevel));
            lineR[i]->write(std::clamp(ir, -clampLevel, clampLevel));

            if (processCount == 0)
            {
                // every 8th sample for VU should be fine
                vuLevel[i] = std::clamp(
                    std::max(vuFalloff * vuLevel[i], (std::fabs(dl) + std::fabs(dr))), 0.f, 10.f);
            }

            outputs[INPUT_L].setVoltage(dl, i);
            outputs[INPUT_R].setVoltage(dr, i);
        }

        processCount++;
    }

    void moduleSpecificSampleRateChange() override
    {
        vuFalloff = exp(-2.0 * M_PI * 8 / APP->engine->getSampleRate());
    }
};
} // namespace sst::surgext_rack::delay
#endif
