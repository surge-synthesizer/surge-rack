//
// Created by Paul Walker on 8/29/22.
//

#ifndef SURGE_RACK_SURGEVCF_HPP
#define SURGE_RACK_SURGEVCF_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"


namespace sst::surgext_rack::vcf
{
struct VCF : public modules::XTModule
{
    static constexpr int n_vcf_params{5};
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        FREQUENCY,
        RESONANCE,
        IN_GAIN,
        MIX,
        OUT_GAIN,

        VCF_MOD_PARAM_0,

        NUM_PARAMS = VCF_MOD_PARAM_0 + n_vcf_params * n_mod_inputs
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,

        VCF_MOD_INPUT,
        NUM_INPUTS = VCF_MOD_INPUT + n_mod_inputs,

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
        int offset = baseParam - FREQUENCY;
        return VCF_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    VCF() : XTModule()
    {
        setupSurgeCommon(NUM_PARAMS, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // FIXME attach formatters here
        configParam<modules::MidiNoteParamQuantity<69>>(FREQUENCY, -60, 70, 0);
        configParam(RESONANCE, 0, 1, 0, "Resonance", "%", 0.f, 100.f);
        configParam(IN_GAIN, 0, 2, 1);
        configParam(MIX, 0, 1, 1, "Mix", "%", 0.f, 100.f);
        configParam(OUT_GAIN, 0, 2, 1);

        for (int i = 0; i < n_vcf_params * n_mod_inputs; ++i)
        {
            configParam(VCF_MOD_PARAM_0 + i, -1, 1, 0);
        }

        setupSurge();
    }

    std::string getName() override { return "VCF"; }

    static constexpr int nQFUs = MAX_POLY >> 1; // >> 2 for SIMD <<1 for stereo
    sst::filters::QuadFilterUnitState qfus[nQFUs];
    __m128 qfuRCache[sst::filters::n_filter_registers][nQFUs];
    sst::filters::FilterCoefficientMaker<SurgeStorage> coefMaker[nQFUs];
    float delayBuffer[nQFUs][4][sst::filters::utilities::MAX_FB_COMB +
                                sst::filters::utilities::SincTable::FIRipol_N];

    void setupSurge()
    {
        processPosition = BLOCK_SIZE;

        restackSIMD();

        for (auto c = 0; c < nQFUs; ++c)
        {
            coefMaker[c].setSampleRateAndBlockSize(APP->engine->getSampleRate(), BLOCK_SIZE);
            for (auto i = 0; i < 4; ++i)
            {
                qfus[c].DB[i] = &(delayBuffer[c][i][0]);
            }
        }
    }

    void moduleSpecificSampleRateChange() override
    {
        for (auto c = 0; c < nQFUs; ++c)
        {
            coefMaker[c].setSampleRateAndBlockSize(APP->engine->getSampleRate(), BLOCK_SIZE);
        }
    }

    void resetFilterRegisters()
    {
        for (auto c = 0; c < nQFUs; ++c)
        {
            std::fill(qfus[c].R, &qfus[c].R[sst::filters::n_filter_registers], _mm_setzero_ps());
            memcpy(qfuRCache[c], qfus[c].R, sst::filters::n_filter_registers * sizeof(__m128));
            for (int i = 0; i < 4; ++i)
            {
                qfus[c].WP[i] = 0;
                qfus[c].active[i] = 0xFFFFFFFF;
            }
        }
    }

    int processPosition;
    int32_t channelToSIMD[2 /* channels */][MAX_POLY][2 /* register and slot */];
    bool stereoStack{false};
    int nVoices{0}, nSIMDSlots{0};

    int lastPolyL{-2}, lastPolyR{-2};

    void restackSIMD()
    {
        for (int c = 0; c < 2; ++c)
        {
            for (int p = 0; p < MAX_POLY; ++p)
            {
                channelToSIMD[c][p][0] = -1;
                channelToSIMD[c][p][1] = -1;
            }
            stereoStack = false;
            nVoices = 0;
            nSIMDSlots = 0;
        }

        if (lastPolyL == -1 && lastPolyR == -1)
        {
            // blank is fine
        }
        else if (lastPolyR == -1 || lastPolyL == -1)
        {
            int channel = (lastPolyR == -1) ? 0 : 1;
            int poly = (lastPolyR == -1) ? lastPolyL : lastPolyR;

            for (int p = 0; p < poly; ++p)
            {
                channelToSIMD[channel][p][0] = p / 4;
                channelToSIMD[channel][p][1] = p % 4;
            }

            nVoices = poly;
            nSIMDSlots = (poly - 1) / 4 + 1;
        }
        else
        {
            int poly = std::max(lastPolyR, lastPolyL);
            stereoStack = true;
            for (int p = 0; p < poly; p++)
            {
                auto idx = p * 2;
                channelToSIMD[0][p][0] = idx / 4;
                channelToSIMD[0][p][1] = idx % 4;
                channelToSIMD[1][p][0] = (idx + 1) / 4;
                channelToSIMD[1][p][1] = (idx + 1) % 4;
            }
            nVoices = poly * 2;
            nSIMDSlots = (nVoices - 1) / 4 + 1;
        }

        // reset all filters
        resetFilterRegisters();
    }

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        static int dumpEvery = 0;

        if (processPosition >= BLOCK_SIZE)
        {
            for (int c = 0; c < nQFUs; ++c)
            {
                memcpy(qfuRCache[c], qfus[c].R, sst::filters::n_filter_registers * sizeof(__m128));
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

                outputs[OUTPUT_L].setChannels(std::max(1, thisPolyL));
                outputs[OUTPUT_R].setChannels(std::max(1, thisPolyR));

                restackSIMD();
            }

            // Setup Coefficients if changed or modulated etc

            // HAMMER - we can and must be smarter about this
            for (int c = 0; c < nQFUs; ++c)
            {
                for (int f = 0; f < sst::filters::n_cm_coeffs; ++f)
                {
                    coefMaker[c].C[f] = qfus[c].C[f][0];
                }

                coefMaker[c].MakeCoeffs(params[FREQUENCY].getValue(),
                                        params[RESONANCE].getValue(), sst::filters::fut_obxd_4pole,
                                        sst::filters::st_lpmoog_24dB, storage.get(), false);
                for (int p = 0; p < 4; ++p)
                {
                    coefMaker[c].updateState(qfus[c], p);
                }
                memcpy(qfus[c].R, qfuRCache[c], sst::filters::n_filter_registers * sizeof(__m128));
            }
            processPosition = 0;
        }
        float invalues alignas(16)[2 * MAX_POLY];
        float outvalues alignas(16)[2 * MAX_POLY];
        // fix me - not every sample pls
        std::memset(invalues, 0, 2 * MAX_POLY * sizeof(float));
        for (int c = 0; c < 2; ++c)
        {
            auto it = (c == 0 ? INPUT_L : INPUT_R);
            auto pl = (c == 0 ? lastPolyL : lastPolyR);
            for (int p = 0; p < pl; ++p)
            {
                auto idx = channelToSIMD[c][p][0] * 4 + channelToSIMD[c][p][1];
                invalues[idx] = inputs[it].getVoltage(p) * RACK_TO_SURGE_OSC_MUL;
            }
        }

        auto fptr = sst::filters::GetQFPtrFilterUnit(sst::filters::FilterType::fut_obxd_4pole,
                                                     sst::filters::FilterSubType::st_lpmoog_24dB);

        for (int s = 0; s < nSIMDSlots; ++s)
        {
            auto in = _mm_load_ps(&invalues[s * 4]);

            auto out = fptr(&qfus[s], in);
            _mm_store_ps(&outvalues[s * 4], out);
        }

        for (int c = 0; c < 2; ++c)
        {
            auto it = (c == 0 ? OUTPUT_L : OUTPUT_R);
            auto pl = (c == 0 ? lastPolyL : lastPolyR);
            for (int p = 0; p < pl; ++p)
            {
                auto idx = channelToSIMD[c][p][0] * 4 + channelToSIMD[c][p][1];
                outputs[it].setVoltage(outvalues[idx] * SURGE_TO_RACK_OSC_MUL, p);
            }
        }

        dumpEvery++;
        if (dumpEvery == 44000)
            dumpEvery = 0;
        processPosition++;
    }
};
}

#endif // SURGE_RACK_SURGEVCF_HPP
