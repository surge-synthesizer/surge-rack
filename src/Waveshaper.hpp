//
// Created by Paul Walker on 8/29/22.
//

#ifndef SURGE_RACK_SURGEWAVESHAPER_HPP
#define SURGE_RACK_SURGEWAVESHAPER_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "globals.h"
#include <sst/waveshapers.h>

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
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // FIXME attach formatters here
        configParam<modules::DecibelParamQuantity>(DRIVE, 0, 2, 1); // UNITS
        configParam(BIAS, -1, 1, 0);
        configParam<modules::DecibelParamQuantity>(OUT_GAIN, 0, 2, 1);
        configParam(LOCUT, 0, 1, 0);
        configParam(HICUT, 0, 1, 1);
        configParam(LOCUT_ENABLED, 0, 1, 0);
        configParam(HICUT_ENABLED, 0, 1, 0);

        configParam<WaveshaperTypeParamQuanity>(WSHP_TYPE, 0,
                                                (int)sst::waveshapers::WaveshaperType::n_ws_types,
                                                (int)sst::waveshapers::WaveshaperType::wst_ojd);

        for (int i = 0; i < n_wshp_params * n_mod_inputs; ++i)
        {
            configParam(WSHP_MOD_PARAM_0 + i, -1, 1, 0);
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

        setupSurge();

        modulationAssistant.initialize(this);
    }

    std::string getName() override { return "WSHP"; }

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
    sst::filters::QuadFilterUnitState qfus[nQFUs];
    sst::filters::FilterCoefficientMaker<SurgeStorage> coefMaker[MAX_POLY];
    float delayBuffer[nQFUs][4][sst::filters::utilities::MAX_FB_COMB +
                                sst::filters::utilities::SincTable::FIRipol_N];

    void setupSurge()
    {
        processPosition = BLOCK_SIZE;

        restackSIMD();

        for (auto i = 0; i < MAX_POLY; ++i)
            coefMaker[i].setSampleRateAndBlockSize(APP->engine->getSampleRate(), BLOCK_SIZE);
        resetWaveshaperRegisters();
    }

    void moduleSpecificSampleRateChange() override
    {
        for (auto i = 0; i < MAX_POLY; ++i)
            coefMaker[i].setSampleRateAndBlockSize(APP->engine->getSampleRate(), BLOCK_SIZE);
        resetWaveshaperRegisters();
    }

    void resetWaveshaperRegisters()
    {
        auto wstype = (sst::waveshapers::WaveshaperType)(int)(std::round(params[WSHP_TYPE].getValue()));

        float R[4];

        initializeWaveshaperRegister(wstype, R);

        for (int q=0; q<nQFUs; ++q)
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
    int qfuIndexForVoice[MAX_POLY << 1][2];  // L-R Voice, {QFU, SIMD Slot}
    int voiceIndexForPolyPos[MAX_POLY << 1]; // only used in stereo mode

    int lastPolyL{-2}, lastPolyR{-2};
    int monoChannelOffset{0};

    void restackSIMD()
    {
        stereoStack = false;
        nVoices = 0;
        nSIMDSlots = 0;
        for (int c = 0; c < MAX_POLY << 1; ++c)
        {
            qfuIndexForVoice[c][0] = -1;
            qfuIndexForVoice[c][0] = -1;
        }

        memset(voiceIndexForPolyPos, 0, sizeof(voiceIndexForPolyPos));

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
                auto qid = c >> 2;
                auto qslt = c % 4;
                qfuIndexForVoice[c][0] = qid;
                qfuIndexForVoice[c][1] = qslt;
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
                auto qid = idx >> 2;
                auto qslt = idx % 4;
                qfuIndexForVoice[idx][0] = qid;
                qfuIndexForVoice[idx][1] = qslt;
                voiceIndexForPolyPos[idx] = l;

                idx++;
            }
            for (int r = 0; r < lastPolyR; ++r)
            {
                auto qid = idx >> 2;
                auto qslt = idx % 4;
                qfuIndexForVoice[idx][0] = qid;
                qfuIndexForVoice[idx][1] = qslt;
                voiceIndexForPolyPos[idx] = r;
                idx++;
            }
        }

        // reset all filters
        resetWaveshaperRegisters();
    }

    sst::waveshapers::WaveshaperType lastType = sst::waveshapers::WaveshaperType::wst_none;
    sst::filters::FilterUnitQFPtr filterPtrLo{nullptr};
    sst::filters::FilterUnitQFPtr filterPtrHi{nullptr};
    sst::waveshapers::QuadWaveshaperPtr wsPtr{nullptr};
    sst::waveshapers::QuadWaveshaperState wss[nQFUs];


    static __m128 filterDupInToOut(sst::filters::QuadFilterUnitState *__restrict, __m128 in)
    {
        return in;
    }


    static __m128 wsDupInToOut(sst::waveshapers::QuadWaveshaperState *__restrict, __m128 in, __m128 drive)
    {
        return in;
    }


    void process(const typename rack::Module::ProcessArgs &args) override
    {
        auto wstype = (sst::waveshapers::WaveshaperType)(int)(std::round(params[WSHP_TYPE].getValue()));

        if (processPosition >= BLOCK_SIZE)
        {
            modulationAssistant.setupMatrix(this);
            modulationAssistant.updateValues(this);

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

#if 0
            filterPtr = sst::filters::GetQFPtrFilterUnit(ftype, fsubtype);
            if (!filterPtr)
            {
                filterPtr = dupInToOut;
            }
#endif

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

#if 0
            bool calculated[MAX_POLY];
            std::fill(calculated, calculated + MAX_POLY, false);

            for (int v = 0; v < nVoices; ++v)
            {
                int qf = qfuIndexForVoice[v][0];
                int qp = qfuIndexForVoice[v][1];
                int pv = voiceIndexForPolyPos[v];

                if (qf < 0 || qf >= nQFUs)
                    continue; // shouldn't happen

                if (!calculated[pv])
                {
                    for (int f = 0; f < sst::filters::n_cm_coeffs; ++f)
                    {
                        coefMaker[pv].C[f] = qfus[qf].C[f][qp];
                    }
                    coefMaker[pv].MakeCoeffs(modulationAssistant.values[FREQUENCY - FREQUENCY][pv],
                                            modulationAssistant.values[RESONANCE - FREQUENCY][pv],
                                            ftype, fsubtype, storage.get(), false);
                    calculated[pv] = true;
                }
                coefMaker[pv].updateState(qfus[qf], qp);
            }
#endif

            processPosition = 0;
        }

        if (stereoStack)
        {
            // We can make this more efficient with smarter SIMD loads later
            float tmpVal alignas(16)[MAX_POLY << 2];
            float tmpValOut alignas(16)[MAX_POLY << 2];
            float *ivl = inputs[INPUT_L].getVoltages();
            float *ivr = inputs[INPUT_R].getVoltages();
            float *ovl = outputs[OUTPUT_L].getVoltages();
            float *ovr = outputs[OUTPUT_R].getVoltages();
            int idx = 0;
            for (int l=0; l<lastPolyL; ++l)
            {
                tmpVal[idx] = ivl[l];
                idx++;
            }
            for (int r=0; r<lastPolyR; ++r)
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
                    for (int p=0; p< n_wshp_params; ++p)
                    {
                        // std::cout << "modsRaw[" << p << "][" << v << "] = ma[" << p << "][" << vc << "]" << std::endl;
                        modsRaw[p][v] = modulationAssistant.values[p][vc];
                    }
                }
                for (int p=0; p< n_wshp_params; ++p)
                    mods[p] = _mm_load_ps(modsRaw[p]);

                auto in = _mm_mul_ps(_mm_loadu_ps(tmpVal + (i << 2)), _mm_set1_ps(RACK_TO_SURGE_OSC_MUL));
                in = _mm_add_ps(in, mods[BIAS - DRIVE]);
                auto fin = wsPtr(&wss[i], in, mods[0 /* DRIVE - DRIVE */]);
                fin = _mm_mul_ps(fin, mods[OUT_GAIN - DRIVE]);
                fin = _mm_mul_ps(fin, _mm_set1_ps(SURGE_TO_RACK_OSC_MUL));
                _mm_storeu_ps(tmpValOut + (i << 2), fin);
            }

            idx = 0;
            for (int l=0; l<lastPolyL; ++l)
            {
                ovl[l] = tmpValOut[idx];
                idx++;
            }
            for (int r=0; r<lastPolyR; ++r)
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
                auto in = _mm_mul_ps(_mm_loadu_ps(iv + (i << 2)), _mm_set1_ps(RACK_TO_SURGE_OSC_MUL));
                in = _mm_add_ps(in, mvsse[BIAS - DRIVE][i]);
                auto fin = wsPtr(&wss[i], in, mvsse[0 /* DRIVE - DRIVE */][i]);
                fin = _mm_mul_ps(fin, mvsse[OUT_GAIN - DRIVE][i]);
                fin = _mm_mul_ps(fin, _mm_set1_ps(SURGE_TO_RACK_OSC_MUL));
                _mm_storeu_ps(ov + (i << 2), fin);
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
};

} // namespace sst::surgext_rack::waveshaper

#endif // SURGE_RACK_SURGEWSHP_HPP
