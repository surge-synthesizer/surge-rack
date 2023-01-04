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

#ifndef SURGE_RACK_SURGEVCF_HPP
#define SURGE_RACK_SURGEVCF_HPP

#include "SurgeXT.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::vcf
{
struct VCFTypeParamQuanity : rack::ParamQuantity
{
    std::string getLabel() override { return "Filter Model"; }
    std::string getDisplayValueString() override
    {
        int val = (int)std::round(getValue());
        return sst::filters::filter_type_names[val];
    }
};

struct VCFSubTypeParamQuanity : rack::ParamQuantity
{
    std::string getLabel() override { return "Filter SubType"; }
    std::string getDisplayValueString() override;
};

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

        VCF_TYPE = VCF_MOD_PARAM_0 + n_vcf_params * n_mod_inputs,
        VCF_SUBTYPE,
        NUM_PARAMS
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

    modules::ModulationAssistant<VCF, 5, FREQUENCY, n_mod_inputs, VCF_MOD_INPUT>
        modulationAssistant;

    std::array<int, sst::filters::num_filter_types> defaultSubtype;

    VCF() : XTModule()
    {
        std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        setupSurgeCommon(NUM_PARAMS, false, false);

        // FIXME attach formatters here
        configParam<modules::VOctParamQuantity<60>>(FREQUENCY, -4, 6, 0, "Frequency");
        configParam(RESONANCE, 0, 1, sqrt(2) * 0.5, "Resonance", "%", 0.f, 100.f);
        configParamNoRand<modules::DecibelParamQuantity>(IN_GAIN, 0, 2, 1, "Pre-Filter Gain");
        configParam(MIX, 0, 1, 1, "Mix", "%", 0.f, 100.f);
        configParamNoRand<modules::DecibelParamQuantity>(OUT_GAIN, 0, 2, 1, "Gain");

        configParam<VCFTypeParamQuanity>(VCF_TYPE, 0, sst::filters::num_filter_types - 1,
                                         sst::filters::fut_obxd_4pole, "Filter Model Type");

        int mfst = 0;
        for (auto fc : sst::filters::fut_subcount)
            mfst = std::max(mfst, fc);

        configParam<VCFSubTypeParamQuanity>(VCF_SUBTYPE, 0, mfst, 3, "Filter Model SubType");

        for (int i = 0; i < n_vcf_params * n_mod_inputs; ++i)
        {
            int tp = paramModulatedBy(i + VCF_MOD_PARAM_0);
            auto lb = paramQuantities[tp]->getLabel();
            std::string name = std::string("Mod ") + std::to_string(i % 4 + 1) + " to " + lb;

            if (tp == FREQUENCY)
                configParamNoRand(VCF_MOD_PARAM_0 + i, -1, 1, 0, name, " Oct/V");
            else
                configParamNoRand(VCF_MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
        }

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        for (int m = 0; m < n_mod_inputs; ++m)
        {
            auto s = std::string("Modulation Signal ") + std::to_string(m + 1);
            configInput(VCF_MOD_INPUT + m, s);
        }
        configOutput(OUTPUT_L, "Left");
        configOutput(OUTPUT_R, "Right");

        setupSurge();

        for (auto &st : defaultSubtype)
            st = 0;
        defaultSubtype[sst::filters::fut_obxd_4pole] = 3;
        defaultSubtype[sst::filters::fut_lpmoog] = 3;
        defaultSubtype[sst::filters::fut_comb_pos] = 1;
        defaultSubtype[sst::filters::fut_comb_neg] = 1;

        modulationAssistant.initialize(this);

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
    }

    std::string getName() override { return "VCF"; }

    bool isBipolar(int paramId) override
    {
        if (paramId == IN_GAIN || paramId == OUT_GAIN || paramId == FREQUENCY)
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
        resetFilterRegisters();
    }

    void moduleSpecificSampleRateChange() override
    {
        restackSIMD();
        for (auto i = 0; i < MAX_POLY; ++i)
            coefMaker[i].setSampleRateAndBlockSize(APP->engine->getSampleRate(), BLOCK_SIZE);
        resetFilterRegisters();
    }

    void resetFilterRegisters()
    {
        for (auto i = 0; i < MAX_POLY; ++i)
            coefMaker[i].Reset();
        for (auto c = 0; c < nQFUs; ++c)
        {
            std::fill(qfus[c].R, &qfus[c].R[sst::filters::n_filter_registers], _mm_setzero_ps());
            std::fill(qfus[c].C, &qfus[c].C[sst::filters::n_cm_coeffs], _mm_setzero_ps());
            for (int i = 0; i < 4; ++i)
            {
                qfus[c].WP[i] = 0;
                qfus[c].active[i] = 0xFFFFFFFF;
                qfus[c].DB[i] = &(delayBuffer[c][i][0]);
            }
        }

        for (int i = 0; i < MAX_POLY >> 2; ++i)
        {
            currentInGain[i] = modulationAssistant.valuesSSE[IN_GAIN][i];
            currentOutGain[i] = modulationAssistant.valuesSSE[OUT_GAIN][i];
            currentMix[i] = modulationAssistant.valuesSSE[MIX][i];
        }
    }

    int processPosition;
    bool stereoStack{false};
    int nVoices{0}, nSIMDSlots{0};
    int qfuIndexForVoice[MAX_POLY << 1][2];  // L-R Voice, {QFU, SIMD Slot}
    int voiceIndexForPolyPos[MAX_POLY << 1]; // only used in stereo mode

    int lastPolyL{-2}, lastPolyR{-2};
    int monoChannelOffset{0};

    const __m128 rackToSurgeOsc{_mm_set1_ps(RACK_TO_SURGE_OSC_MUL)};
    const __m128 surgeToRackOsc{_mm_set1_ps(SURGE_TO_RACK_OSC_MUL)};
    const __m128 oneSimd{_mm_set1_ps(1.f)};
    const __m128 oneOverBlock{_mm_set1_ps(1.f / BLOCK_SIZE)};
    __m128 currentInGain[MAX_POLY >> 2], dInGain[MAX_POLY >> 2];
    __m128 currentOutGain[MAX_POLY >> 2], dOutGain[MAX_POLY >> 2];
    __m128 currentMix[MAX_POLY >> 2], dMix[MAX_POLY >> 2];

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
            // mono case
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
        resetFilterRegisters();
    }

    sst::filters::FilterType lastType = sst::filters::FilterType::fut_none;
    sst::filters::FilterSubType lastSubType = sst::filters::FilterSubType::st_Standard;
    sst::filters::FilterUnitQFPtr filterPtr{nullptr};

    static __m128 dupInToOut(sst::filters::QuadFilterUnitState *__restrict, __m128 in)
    {
        return in;
    }

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        auto ftype = (sst::filters::FilterType)(int)(std::round(params[VCF_TYPE].getValue()));
        auto fsubtype =
            (sst::filters::FilterSubType)(int)(std::round(params[VCF_SUBTYPE].getValue()));

        if (processPosition >= BLOCK_SIZE)
        {
            modulationAssistant.setupMatrix(this);
            modulationAssistant.updateValues(this);

            if (ftype != lastType || fsubtype != lastSubType)
            {
                resetFilterRegisters();
            }
            lastType = ftype;
            lastSubType = fsubtype;
            if (lastSubType > sst::filters::fut_subcount[lastType])
            {
                params[VCF_SUBTYPE].setValue(0.f);
                lastSubType = sst::filters::FilterSubType::st_Standard;
            }

            defaultSubtype[lastType] = lastSubType;
            filterPtr = sst::filters::GetQFPtrFilterUnit(ftype, fsubtype);
            if (!filterPtr)
            {
                filterPtr = dupInToOut;
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
                    auto fvoct = modulationAssistant.values[FREQUENCY - FREQUENCY][pv];
                    auto fmidi = (fvoct + 5) * 12;
                    coefMaker[pv].MakeCoeffs(fmidi - 69,
                                             modulationAssistant.values[RESONANCE - FREQUENCY][pv],
                                             ftype, fsubtype, storage.get(), false);
                    calculated[pv] = true;
                }
                coefMaker[pv].updateState(qfus[qf], qp);
            }

            for (int i = 0; i < MAX_POLY >> 2; ++i)
            {
                auto tig = modules::DecibelParamQuantity::ampToLinearSSE(
                    modulationAssistant.valuesSSE[IN_GAIN][i]);
                dInGain[i] = _mm_mul_ps(_mm_sub_ps(tig, currentInGain[i]), oneOverBlock);

                auto tog = modules::DecibelParamQuantity::ampToLinearSSE(
                    modulationAssistant.valuesSSE[OUT_GAIN][i]);
                dOutGain[i] = _mm_mul_ps(_mm_sub_ps(tog, currentOutGain[i]), oneOverBlock);

                auto tmix = modulationAssistant.valuesSSE[MIX][i];
                dMix[i] = _mm_mul_ps(_mm_sub_ps(tmix, currentMix[i]), oneOverBlock);
            }

            processPosition = 0;
        }

        for (int i = 0; i < MAX_POLY >> 2; i++)
        {
            currentInGain[i] = _mm_add_ps(currentInGain[i], dInGain[i]);
            currentOutGain[i] = _mm_add_ps(currentOutGain[i], dOutGain[i]);
            currentMix[i] = _mm_add_ps(currentMix[i], dMix[i]);
        }

        if (stereoStack && lastPolyL == lastPolyR && lastPolyR == 1)
        {
            // dual poly case
            float iv alignas(16)[4]{0, 0, 0, 0}, ov alignas(16)[4]{0, 0, 0, 0};
            iv[0] = inputs[INPUT_L].getVoltage(0);
            iv[1] = inputs[INPUT_R].getVoltage(0);

            // const auto &mv = modulationAssistant.values;
            const auto ig =
                _mm_shuffle_ps(currentInGain[0], currentInGain[0], _MM_SHUFFLE(0, 0, 0, 0));
            const auto og =
                _mm_shuffle_ps(currentOutGain[0], currentOutGain[0], _MM_SHUFFLE(0, 0, 0, 0));
            const auto mx = _mm_shuffle_ps(currentMix[0], currentMix[0], _MM_SHUFFLE(0, 0, 0, 0));

            auto in = _mm_mul_ps(_mm_load_ps(iv), rackToSurgeOsc);

            auto pre = _mm_mul_ps(in, ig);
            auto filt = filterPtr(&qfus[0], pre);

            auto post = _mm_mul_ps(filt, og);
            auto omm = _mm_sub_ps(oneSimd, mx);

            auto fin = _mm_add_ps(_mm_mul_ps(mx, post), _mm_mul_ps(omm, in));
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

            float mvUnload alignas(16)[n_vcf_params][MAX_POLY];
            for (int i = 0; i < MAX_POLY >> 2; i++)
            {
                _mm_store_ps(&mvUnload[IN_GAIN][i << 2], currentInGain[i]);
                _mm_store_ps(&mvUnload[OUT_GAIN][i << 2], currentOutGain[i]);
                _mm_store_ps(&mvUnload[MIX][i << 2], currentMix[i]);
            }
            for (int i = 0; i < nSIMDSlots; ++i)
            {
                float modsRaw alignas(16)[n_vcf_params][4];
                __m128 mods[n_vcf_params];
                int vidx = i << 2;
                for (int v = 0; v < 4; ++v)
                {
                    auto vc = voiceIndexForPolyPos[v + vidx];
                    for (int p = IN_GAIN; p <= OUT_GAIN; ++p)
                    {
                        modsRaw[p][v] = mvUnload[p][vc];
                    }
                }
                for (int p = IN_GAIN; p <= OUT_GAIN; ++p)
                    mods[p] = _mm_load_ps(modsRaw[p]);

                auto in = _mm_mul_ps(_mm_loadu_ps(tmpVal + (i << 2)), rackToSurgeOsc);
                auto pre = _mm_mul_ps(in, mods[IN_GAIN - FREQUENCY]);
                auto filt = filterPtr(&qfus[i], pre);

                auto post = _mm_mul_ps(filt, mods[OUT_GAIN - FREQUENCY]);
                auto omm = _mm_sub_ps(oneSimd, mods[MIX - FREQUENCY]);

                auto fin = _mm_add_ps(_mm_mul_ps(mods[MIX - FREQUENCY], post), _mm_mul_ps(omm, in));

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
                auto in = _mm_mul_ps(_mm_loadu_ps(iv + (i << 2)), rackToSurgeOsc);

                auto pre = _mm_mul_ps(in, currentInGain[i]);
                auto filt = filterPtr(&qfus[i], pre);

                auto post = _mm_mul_ps(filt, currentOutGain[i]);
                auto omm = _mm_sub_ps(oneSimd, currentMix[i]);

                auto fin = _mm_add_ps(_mm_mul_ps(currentMix[i], post), _mm_mul_ps(omm, in));
                fin = _mm_mul_ps(fin, surgeToRackOsc);
                _mm_storeu_ps(ov + (i << 2), fin);
            }
        }

        processPosition++;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - FREQUENCY;
        if (idx < 0 || idx >= n_vcf_params)
            return 0;
        return modulationAssistant.animValues[idx];
    }

    static std::string subtypeLabel(int type, int subtype)
    {
        using sst::filters::FilterType;
        int i = subtype;
        const auto fType = (FilterType)type;
        if (sst::filters::fut_subcount[type] == 0)
        {
            return "None";
        }
        else
        {
            switch (fType)
            {
            case FilterType::fut_lpmoog:
            case FilterType::fut_diode:
                return sst::filters::fut_ldr_subtypes[i];
                break;
            case FilterType::fut_notch12:
            case FilterType::fut_notch24:
            case FilterType::fut_apf:
                return sst::filters::fut_notch_subtypes[i];
                break;
            case FilterType::fut_comb_pos:
            case FilterType::fut_comb_neg:
                return sst::filters::fut_comb_subtypes[i];
                break;
            case FilterType::fut_vintageladder:
                return sst::filters::fut_vintageladder_subtypes[i];
                break;
            case FilterType::fut_obxd_2pole_lp:
            case FilterType::fut_obxd_2pole_hp:
            case FilterType::fut_obxd_2pole_n:
            case FilterType::fut_obxd_2pole_bp:
                return sst::filters::fut_obxd_2p_subtypes[i];
                break;
            case FilterType::fut_obxd_4pole:
                return sst::filters::fut_obxd_4p_subtypes[i];
                break;
            case FilterType::fut_k35_lp:
            case FilterType::fut_k35_hp:
                return sst::filters::fut_k35_subtypes[i];
                break;
            case FilterType::fut_cutoffwarp_lp:
            case FilterType::fut_cutoffwarp_hp:
            case FilterType::fut_cutoffwarp_n:
            case FilterType::fut_cutoffwarp_bp:
            case FilterType::fut_cutoffwarp_ap:
            case FilterType::fut_resonancewarp_lp:
            case FilterType::fut_resonancewarp_hp:
            case FilterType::fut_resonancewarp_n:
            case FilterType::fut_resonancewarp_bp:
            case FilterType::fut_resonancewarp_ap:
                // "i & 3" selects the lower two bits that represent the stage count
                // "(i >> 2) & 3" selects the next two bits that represent the
                // saturator
                return fmt::format("{} {}", sst::filters::fut_nlf_subtypes[i & 3],
                                   sst::filters::fut_nlf_saturators[(i >> 2) & 3]);
                break;
            // don't default any more so compiler catches new ones we add
            case FilterType::fut_none:
            case FilterType::fut_lp12:
            case FilterType::fut_lp24:
            case FilterType::fut_bp12:
            case FilterType::fut_bp24:
            case FilterType::fut_hp12:
            case FilterType::fut_hp24:
            case FilterType::fut_SNH:
                return sst::filters::fut_def_subtypes[i];
                break;
            case FilterType::fut_tripole:
                // "i & 3" selects the lower two bits that represent the filter mode
                // "(i >> 2) & 3" selects the next two bits that represent the
                // output stage
                return fmt::format("{} {}", sst::filters::fut_tripole_subtypes[i & 3],
                                   sst::filters::fut_tripole_output_stage[(i >> 2) & 3]);
                break;
            case FilterType::num_filter_types:
                return "ERROR";
                break;
            }
        }
        return "Error";
    }

    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - VCF_MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_vcf_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs + FREQUENCY;
    }
};

inline std::string VCFSubTypeParamQuanity::getDisplayValueString()
{
    if (!module)
        return "None";

    int type = (int)std::round(module->params[VCF::VCF_TYPE].getValue());
    int val = (int)std::round(getValue());
    return VCF::subtypeLabel(type, val);
}
} // namespace sst::surgext_rack::vcf

#endif // SURGE_RACK_SURGEVCF_HPP
