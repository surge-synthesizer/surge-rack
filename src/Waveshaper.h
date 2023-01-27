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

#ifndef SURGE_RACK_SURGEWAVESHAPER_HPP
#define SURGE_RACK_SURGEWAVESHAPER_HPP

#include "SurgeXT.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>
#include "globals.h"
#include <sst/waveshapers.h>
#include "BiquadFilter.h"

namespace sst::surgext_rack::waveshaper
{
struct WaveshaperTypeParamQuanity : rack::ParamQuantity
{
    std::string getLabel() override { return "Waveshaper Model"; }
    std::string getDisplayValueString() override
    {
        int val = (int)std::round(getValue());
        return sst::waveshapers::wst_names[val];
    }
};

struct Waveshaper : public modules::XTModule
{
    static constexpr int n_wshp_params{5};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        DRIVE,
        BIAS,
        OUT_GAIN,
        LOCUT,
        HICUT,

        WSHP_MOD_PARAM_0,

        WSHP_TYPE = WSHP_MOD_PARAM_0 + n_wshp_params * n_mod_inputs,
        LOCUT_ENABLED,
        HICUT_ENABLED,

        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,

        WSHP_MOD_INPUT,
        NUM_INPUTS = WSHP_MOD_INPUT + n_mod_inputs,

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
        int offset = baseParam - DRIVE;
        return WSHP_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    modules::ModulationAssistant<Waveshaper, 5, DRIVE, n_mod_inputs, WSHP_MOD_INPUT>
        modulationAssistant;

    Waveshaper() : XTModule()
    {
        std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        setupSurgeCommon(NUM_PARAMS, false, false);

        // FIXME attach formatters here
        configParam(DRIVE, -24, 24, 0, "Drive", "dB"); // UNITS
        configParam(BIAS, -1, 1, 0, "Bias", "V", 0, 5);
        configParamNoRand<modules::DecibelParamQuantity>(OUT_GAIN, 0, 2, 1, "Gain");
        configParam<modules::MidiNoteParamQuantity<69>>(LOCUT, -60, 70, -60, "Low Cut");
        configParam<modules::MidiNoteParamQuantity<69>>(HICUT, -60, 70, 70, "High Cut");
        configOnOff(LOCUT_ENABLED, 0, "Enable Low Cut");
        configOnOff(HICUT_ENABLED, 0, "Enable High Cut");

        configParam<WaveshaperTypeParamQuanity>(
            WSHP_TYPE, 0, (int)sst::waveshapers::WaveshaperType::n_ws_types,
            (int)sst::waveshapers::WaveshaperType::wst_ojd, "Waveshaper Function");

        for (int i = 0; i < n_wshp_params * n_mod_inputs; ++i)
        {
            int tp = paramModulatedBy(i + WSHP_MOD_PARAM_0);
            auto lb = paramQuantities[tp]->getLabel();
            std::string name = std::string("Mod ") + std::to_string(i % 4 + 1) + " to " + lb;

            configParamNoRand(WSHP_MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
        }

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        for (int m = 0; m < n_mod_inputs; ++m)
        {
            auto s = std::string("Modulation Signal ") + std::to_string(m + 1);
            configInput(WSHP_MOD_INPUT + m, s);
        }
        configOutput(OUTPUT_L, "Left");
        configOutput(OUTPUT_R, "Right");

        processPosition = BLOCK_SIZE;

        for (int i = 0; i < MAX_POLY; ++i)
        {
            lpPost[i] = std::make_unique<BiquadFilter>(storage.get());
            lpPost[i]->suspend();
            hpPost[i] = std::make_unique<BiquadFilter>(storage.get());
            hpPost[i]->suspend();
        }

        restackSIMD();
        resetWaveshaperRegisters();

        modulationAssistant.initialize(this);

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
    }

    std::string getName() override { return "WSHP"; }

    std::array<std::unique_ptr<BiquadFilter>, MAX_POLY> lpPost, hpPost;

    bool isBipolar(int paramId) override
    {
        if (paramId == DRIVE || paramId == BIAS || paramId == OUT_GAIN)
            return true;
        return false;
    }

    inline int polyChannelCount()
    {
        return std::max({1, inputs[INPUT_L].getChannels(), inputs[INPUT_R].getChannels()});
    }

    static constexpr int nQFUs = MAX_POLY >> 1; // >> 2 for SIMD <<1 for stereo
    bool locutOn{false}, hicutOn{false};

    void moduleSpecificSampleRateChange() override { resetWaveshaperRegisters(); }

    void resetWaveshaperRegisters()
    {
        auto wstype =
            (sst::waveshapers::WaveshaperType)(int)(std::round(params[WSHP_TYPE].getValue()));

        float R[4];

        initializeWaveshaperRegister(wstype, R);

        for (int q = 0; q < nQFUs; ++q)
        {
            for (int i = 0; i < sst::waveshapers::n_waveshaper_registers; ++i)
            {
                wss[q].R[i] = _mm_set1_ps(R[i]);
            }

            wss[q].init = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()); // better way?
        }
    }

    int processPosition;
    bool stereoStack{false};
    int nVoices{0}, nSIMDSlots{0};
    int voiceIndexForPolyPos[MAX_POLY << 1]; // only used in stereo mode

    int lastPolyL{-2}, lastPolyR{-2};
    int monoChannelOffset{0};

    std::atomic<bool> doDCBlock{true};
    bool wasDoDCBlock{true};
    /*
     * This is a bit annoying - i don't want to break 2.0.3.0 patches by turning on
     * a dc blocker but they had no module.json so if you unstream one of them
     * you don't even get called in moduleFromJSON. So I need to know the state of
     * having been constructed and having no json delivered to me rather than having a json
     * block without a dodc in it (which means I am on the new software).
     */
    bool wasDoDCBlockSetByJSON{false};
    modules::DCBlockerSIMD4 blockers[MAX_POLY << 1]; // stereo

    void restackSIMD()
    {
        stereoStack = false;
        nVoices = 0;
        nSIMDSlots = 0;

        memset(voiceIndexForPolyPos, 0, sizeof(voiceIndexForPolyPos));

        for (int i = 0; i < MAX_POLY << 1; ++i)
        {
            blockers[i].reset();
        }

        if (lastPolyL == -1 && lastPolyR == -1)
        {
        }
        else if (lastPolyR == -1 || lastPolyL == -1)
        {
            int channels = std::max(lastPolyL, lastPolyR);
            monoChannelOffset = (lastPolyR == -1) ? 0 : 1;
            int poly = (lastPolyR == -1) ? lastPolyL : lastPolyR;

            nVoices = poly;
            nSIMDSlots = (poly - 1) / 4 + 1;
            stereoStack = false;

            for (int c = 0; c < channels; ++c)
            {
                voiceIndexForPolyPos[c] = c;
            }
        }
        else
        {
            // So the items come in SSE order from the inputs so we want
            // to preserve that. Which means stack all the Ls first and Rs
            // second.
            stereoStack = true;
            nVoices = lastPolyR + lastPolyL;
            nSIMDSlots = (nVoices - 1) / 4 + 1;

            int idx = 0;
            for (int l = 0; l < lastPolyL; ++l)
            {
                voiceIndexForPolyPos[idx] = l;

                idx++;
            }
            for (int r = 0; r < lastPolyR; ++r)
            {
                voiceIndexForPolyPos[idx] = r;
                idx++;
            }
        }

        // reset all filters
        resetWaveshaperRegisters();
    }

    sst::waveshapers::WaveshaperType lastType = sst::waveshapers::WaveshaperType::wst_none;
    sst::waveshapers::QuadWaveshaperPtr wsPtr{nullptr};
    sst::waveshapers::QuadWaveshaperState wss[nQFUs];

    static __m128 filterDupInToOut(sst::filters::QuadFilterUnitState *__restrict, __m128 in)
    {
        return in;
    }

    static __m128 wsDupInToOut(sst::waveshapers::QuadWaveshaperState *__restrict, __m128 in,
                               __m128 drive)
    {
        return in;
    }

    const __m128 rackToSurgeOsc{_mm_set1_ps(RACK_TO_SURGE_OSC_MUL)};
    const __m128 surgeToRackOsc{_mm_set1_ps(SURGE_TO_RACK_OSC_MUL)};

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        auto wstype =
            (sst::waveshapers::WaveshaperType)(int)(std::round(params[WSHP_TYPE].getValue()));
        auto lc = inputs[INPUT_L].isConnected() ? inputs[INPUT_L].getChannels() : 0;
        auto rc = inputs[INPUT_R].isConnected() ? inputs[INPUT_R].getChannels() : 0;

        if (processPosition >= BLOCK_SIZE)
        {
            modulationAssistant.setupMatrix(this);
            modulationAssistant.updateValues(this);

            if (!wasDoDCBlockSetByJSON)
            {
                // See comment above
                doDCBlock = false;
                wasDoDCBlockSetByJSON = true;
            }

            if (doDCBlock && !wasDoDCBlock)
            {
                for (int i = 0; i < MAX_POLY << 1; ++i)
                {
                    blockers[i].reset();
                }
            }
            wasDoDCBlock = doDCBlock;

            auto loOn = params[LOCUT_ENABLED].getValue() > 0.5;
            auto hiOn = params[HICUT_ENABLED].getValue() > 0.5;

            if (loOn)
            {
                if (!locutOn)
                {
                    for (int p = 0; p < MAX_POLY; ++p)
                        hpPost[p]->suspend();
                }
                for (int p = 0; p < lc; ++p)
                {
                    hpPost[p]->coeff_HP(
                        hpPost[p]->calc_omega(modulationAssistant.values[LOCUT][p] / 12.0), 0.707);
                    if (!locutOn)
                        hpPost[p]->coeff_instantize();
                }

                locutOn = true;
            }
            else
            {
                locutOn = false;
            }

            if (hiOn)
            {
                if (!hicutOn)
                {
                    for (int p = 0; p < MAX_POLY; ++p)
                        lpPost[p]->suspend();
                }
                for (int p = 0; p < lc; ++p)
                {
                    lpPost[p]->coeff_LP2B(
                        lpPost[p]->calc_omega(modulationAssistant.values[HICUT][p] / 12.0), 0.707);
                    if (!hicutOn)
                        lpPost[p]->coeff_instantize();
                }

                hicutOn = true;
            }
            else
            {
                hicutOn = false;
            }

            if (wstype != lastType)
            {
                resetWaveshaperRegisters();
            }
            lastType = wstype;
            wsPtr = sst::waveshapers::GetQuadWaveshaper(wstype);
            if (!wsPtr)
            {
                wsPtr = wsDupInToOut;
            }

            int thisPolyL{-1}, thisPolyR{-1};
            if (inputs[INPUT_L].isConnected())
            {
                thisPolyL = inputs[INPUT_L].getChannels();
            }
            if (inputs[INPUT_R].isConnected())
            {
                thisPolyR = inputs[INPUT_R].getChannels();
            }

            if (thisPolyL != lastPolyL || thisPolyR != lastPolyR)
            {
                lastPolyL = thisPolyL;
                lastPolyR = thisPolyR;

                restackSIMD();
            }

            outputs[OUTPUT_L].setChannels(std::max(1, thisPolyL));
            outputs[OUTPUT_R].setChannels(std::max(1, thisPolyR));

            processPosition = 0;
        }
        else
        {
            modulationAssistant.updateValues(this);
        }

        if (stereoStack && lastPolyL == lastPolyR && lastPolyR == 1)
        {
            float iv alignas(16)[4]{0, 0, 0, 0}, ov alignas(16)[4]{0, 0, 0, 0};
            iv[0] = inputs[INPUT_L].getVoltage(0);
            iv[1] = inputs[INPUT_R].getVoltage(0);

            const auto &mv = modulationAssistant.values;

            float dt alignas(16)[4];

            auto drive = _mm_set1_ps(storage->db_to_linear(mv[0][0]));
            auto in = _mm_mul_ps(_mm_load_ps(iv), rackToSurgeOsc);
            in = _mm_add_ps(in, _mm_set1_ps(mv[BIAS - DRIVE][0]));
            auto fin = wsPtr(&wss[0], in, drive);
            fin = _mm_mul_ps(fin, modules::DecibelParamQuantity::ampToLinearSSE(
                                      _mm_set1_ps(mv[OUT_GAIN - DRIVE][0])));

            if (doDCBlock)
            {
                fin = blockers[0].filter(fin);
            }
            fin = _mm_mul_ps(fin, surgeToRackOsc);
            _mm_store_ps(ov, fin);

            outputs[OUTPUT_L].setVoltage(ov[0]);
            outputs[OUTPUT_R].setVoltage(ov[1]);
        }
        else if (stereoStack)
        {
            // We can make this more efficient with smarter SIMD loads later
            float tmpVal alignas(16)[MAX_POLY << 2];
            float tmpValOut alignas(16)[MAX_POLY << 2];
            float *ivl = inputs[INPUT_L].getVoltages();
            float *ivr = inputs[INPUT_R].getVoltages();
            float *ovl = outputs[OUTPUT_L].getVoltages();
            float *ovr = outputs[OUTPUT_R].getVoltages();
            int idx = 0;
            for (int l = 0; l < lastPolyL; ++l)
            {
                tmpVal[idx] = ivl[l];
                idx++;
            }
            for (int r = 0; r < lastPolyR; ++r)
            {
                tmpVal[idx] = ivr[r];
                idx++;
            }

            for (int i = 0; i < nSIMDSlots; ++i)
            {
                float modsRaw alignas(16)[n_wshp_params][4];
                __m128 mods[n_wshp_params];
                int vidx = i << 2;
                for (int v = 0; v < 4; ++v)
                {
                    auto vc = voiceIndexForPolyPos[v + vidx];
                    modsRaw[0][v] = storage->db_to_linear(modulationAssistant.values[0][vc]);
                    // modsRaw[0][v] =
                    // sst::filters::db_to_linear(modulationAssistant.values[0][vc]);
                    for (int p = 1; p < n_wshp_params; ++p)
                    {
                        // std::cout << "modsRaw[" << p << "][" << v << "] = ma[" << p << "][" << vc
                        // << "]" << std::endl;
                        modsRaw[p][v] = modulationAssistant.values[p][vc];
                    }
                }
                for (int p = 0; p < n_wshp_params; ++p)
                    mods[p] = _mm_load_ps(modsRaw[p]);

                auto in = _mm_mul_ps(_mm_loadu_ps(tmpVal + (i << 2)), rackToSurgeOsc);
                in = _mm_add_ps(in, mods[BIAS - DRIVE]);
                auto fin = wsPtr(&wss[i], in, mods[0 /* DRIVE - DRIVE */]);
                fin = _mm_mul_ps(
                    fin, modules::DecibelParamQuantity::ampToLinearSSE(mods[OUT_GAIN - DRIVE]));

                if (doDCBlock)
                {
                    fin = blockers[i].filter(fin);
                }

                fin = _mm_mul_ps(fin, surgeToRackOsc);
                _mm_storeu_ps(tmpValOut + (i << 2), fin);
            }

            idx = 0;
            for (int l = 0; l < lastPolyL; ++l)
            {
                ovl[l] = tmpValOut[idx];
                idx++;
            }
            for (int r = 0; r < lastPolyR; ++r)
            {
                ovr[r] = tmpValOut[idx];
                idx++;
            }
        }
        else
        {
            float *iv = inputs[INPUT_L + monoChannelOffset].getVoltages();
            float *ov = outputs[OUTPUT_L + monoChannelOffset].getVoltages();

            const auto &mvsse = modulationAssistant.valuesSSE;

            for (int i = 0; i < nSIMDSlots; ++i)
            {
                float dt alignas(16)[4];

                _mm_store_ps(dt, mvsse[0][i]);
                for (int v = 0; v < 4; ++v)
                {
                    dt[v] = storage->db_to_linear(dt[v]);
                    // dt[v] = sst::filters::db_to_linear(dt[v]);
                }
                auto drive = _mm_load_ps(dt);
                auto in = _mm_mul_ps(_mm_loadu_ps(iv + (i << 2)), rackToSurgeOsc);
                in = _mm_add_ps(in, mvsse[BIAS - DRIVE][i]);
                auto fin = wsPtr(&wss[i], in, drive);
                fin = _mm_mul_ps(
                    fin, modules::DecibelParamQuantity::ampToLinearSSE(mvsse[OUT_GAIN - DRIVE][i]));

                if (doDCBlock)
                {
                    fin = blockers[i].filter(fin);
                }

                fin = _mm_mul_ps(fin, surgeToRackOsc);
                _mm_storeu_ps(ov + (i << 2), fin);
            }
        }

        if (hicutOn)
        {
            auto L = outputs[OUTPUT_L].getVoltages();
            auto R = outputs[OUTPUT_R].getVoltages();
            for (int i = 0; i < std::max(lc, rc); ++i)
            {
                lpPost[i]->process_sample(L[i], R[i], L[i], R[i]);
            }
        }
        if (locutOn)
        {
            auto L = outputs[OUTPUT_L].getVoltages();
            auto R = outputs[OUTPUT_R].getVoltages();
            for (int i = 0; i < std::max(lc, rc); ++i)
            {
                hpPost[i]->process_sample(L[i], R[i], L[i], R[i]);
            }
        }

        processPosition++;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - DRIVE;
        if (idx < 0 || idx >= n_wshp_params)
            return 0;
        return modulationAssistant.animValues[idx];
    }

    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - WSHP_MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_wshp_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs + DRIVE;
    }

    json_t *makeModuleSpecificJson() override
    {
        auto ws = json_object();
        json_object_set_new(ws, "doDCBlock", json_boolean(doDCBlock));
        return ws;
    }

    void readModuleSpecificJson(json_t *modJ) override
    {
        auto ddb = json_object_get(modJ, "doDCBlock");
        if (ddb)
        {
            wasDoDCBlockSetByJSON = true; // see comment above
            doDCBlock = json_boolean_value(ddb);
        }
        else
        {
            doDCBlock = true;
        }
    }
};

} // namespace sst::surgext_rack::waveshaper

#endif // SURGE_RACK_SURGEWSHP_HPP
