/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
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

/*
 * ToDos
 *
 * - Display Area on Widget
 * - Character Filter -> SSE version
 * - On Sample Rate for the character filter
 */

#ifndef SURGE_XT_RACK_SRC_UNISONHELPER_H
#define SURGE_XT_RACK_SRC_UNISONHELPER_H

#include "SurgeXT.h"
#include "dsp/Effect.h"
#include "dsp/oscillators/OscillatorCommonFunctions.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"
#include "FxPresetAndClipboardManager.h"

#include "LayoutEngine.h"
#include "ADSRModulationSource.h"

namespace sst::surgext_rack::unisonhelper
{
struct UnisonHelper : modules::XTModule
{
    static constexpr int n_mod_params{2};
    static constexpr int n_mod_inputs{4};

    static constexpr int n_sub_vcos{4};

    enum ParamIds
    {
        DETUNE,
        DRIFT,
        VOICE_COUNT,
        DETUNE_EXTEND,
        CHARACTER,

        MOD_PARAM_0,
        NUM_PARAMS = MOD_PARAM_0 + n_mod_params * n_mod_inputs
    };

    enum InputIds
    {
        INPUT_VOCT,
        INPUT_SUB1,

        MOD_INPUT_0 = INPUT_SUB1 + n_sub_vcos,
        NUM_INPUTS = MOD_INPUT_0 + n_mod_inputs,
    };

    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,

        OUTPUT_VOCT_SUB1,
        NUM_OUTPUTS = OUTPUT_VOCT_SUB1 + n_sub_vcos
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::ModulationAssistant<UnisonHelper, n_mod_params, DETUNE, n_mod_inputs, MOD_INPUT_0>
        modAssist;

    struct DetuneParamQuantity : public rack::engine::ParamQuantity
    {
        inline UnisonHelper *dm() { return static_cast<UnisonHelper *>(module); }
        virtual std::string getDisplayValueString() override
        {
            auto m = dm();
            if (!m)
                return "ERROR";

            auto v = getValue();
            if (m->params[DETUNE_EXTEND].getValue() > 0.5)
            {
                v = v * 1200;
            }
            else
            {
                v = v * 100;
            }

            return fmt::format("{:8.2f} cents", v);
        }

        void setDisplayValue(float displayValue) override
        {
            auto m = dm();
            if (!m)
                return;

            auto v = displayValue;
            if (m->params[DETUNE_EXTEND].getValue() > 0.5)
            {
                v = v / 1200;
            }
            else
            {
                v = v / 100;
            }
            setValue(v);
        }
    };
    UnisonHelper() : XTModule()
    {
        {
            std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
            setupSurge();
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<DetuneParamQuantity>(DETUNE, 0, 1, 0.1, "Detune");
        configSwitch(DETUNE_EXTEND, 0, 1, 0, "Detune Range", {"+/- 100 cents", "+/- 1200 cents"});
        configParam(DRIFT, 0, 1, 0, "Drift");
        configSwitch(CHARACTER, 0, 2, 1, "Character", {"Warm", "Neutral", "Bright"});
        auto pq = configParam(VOICE_COUNT, 1, 9, 3, "Voice Count", " Voices");
        pq->snapEnabled = true;

        for (int i = 0; i < n_mod_params * n_mod_inputs; ++i)
        {
            int tp = paramModulatedBy(i + MOD_PARAM_0);
            auto lb = paramQuantities[tp]->getLabel();
            std::string name = std::string("Mod ") + std::to_string(i % 4 + 1) + " to " + lb;

            configParamNoRand(MOD_PARAM_0 + i, -1, 1, 0, name, "%", 0, 100);
        }

        modAssist.initialize(this);
        modAssist.setupMatrix(this);
        modAssist.updateValues(this);

        snapCalculatedNames();

        for (auto &d : driftLFO)
            d.init(false);
    }

    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false, false);
        for (auto &cf : characterFilter)
        {
            cf.storage = storage.get();
            cf.init(0);
        }
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        return nullptr;
    }

    int polyChannelCount() { return nChan; }
    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_mod_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs + DETUNE;
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - DETUNE;
        auto res = MOD_PARAM_0 + offset * n_mod_inputs + modulator;
        return res;
    }

    float modulationDisplayValue(int paramId) override
    {
        // std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        int idx = paramId - DETUNE;
        if (idx < 0 || idx >= n_mod_params)
            return 0;

        return modAssist.modvalues[idx][0];
    }

    bool isBipolar(int paramId) override { return false; }

    void moduleSpecificSampleRateChange() override
    {
        for (auto &cf : characterFilter)
            cf.init(cf.type);
    }

    std::string getName() override { return std::string("UnisonHelper"); }

    int nChan{-1}, nVoices{1};
    Surge::Oscillator::UnisonSetup<float> unisonSetup{nVoices};

    std::array<Surge::Oscillator::DriftLFO, MAX_POLY> driftLFO{};
    std::array<sst::basic_blocks::dsp::SurgeLag<float, true>, MAX_POLY> driftLFOLag{};
    std::array<float, MAX_POLY> baseVOct{};
    std::array<int, n_sub_vcos> channelsPerSubOct{};
    std::array<bool, n_sub_vcos> connectedSet{};
    bool connectedSetChanged{false};
    int highestContiguousConnectedSub{-1};

    std::array<Surge::Oscillator::CharacterFilter<float>, MAX_POLY> characterFilter;

    /*
     * Data structures for voice mapping
     */
    std::array<std::array<int, MAX_POLY>, n_sub_vcos> subVcoToInputChannel{};
    std::array<std::array<int, MAX_POLY>, n_sub_vcos> indexToUnisonVoice{};
    int maxUsedSubVCO{1};
    std::string errorCondition;
    std::atomic<bool> isInErrorState{false};

    int samplePos{0};
    int priorChar{-1};

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int currChar = std::round(params[CHARACTER].getValue());
        if (priorChar != currChar)
        {
            priorChar = currChar;
            for (auto &c : characterFilter)
                c.init(currChar);
        }
        auto currChan = std::max({inputs[INPUT_VOCT].getChannels(), 1});
        int currV = std::round(params[VOICE_COUNT].getValue());
        if (currChan != nChan || currV != nVoices || connectedSetChanged)
        {
            nChan = currChan;
            nVoices = currV;
            connectedSetChanged = false;
            unisonSetup = Surge::Oscillator::UnisonSetup<float>(nVoices);
            isInErrorState = false;

            // FIXME should really be connected vcos not subs
            if (nVoices * currChan > (highestContiguousConnectedSub + 1) * MAX_POLY)
            {
                isInErrorState = true;
                errorCondition = fmt::format("{} * {} too many voices for {} sub-vco(s)", nVoices,
                                             currChan, highestContiguousConnectedSub + 1);
            }

            int curVoice{0}, curSub{0};

            for (auto &a : subVcoToInputChannel)
                std::fill(a.begin(), a.end(), -1);
            for (auto &a : indexToUnisonVoice)
                std::fill(a.begin(), a.end(), -1);

            for (int cc = 0; cc < nChan; ++cc)
            {
                for (int v = 0; v < nVoices; ++v)
                {
                    subVcoToInputChannel[curSub][curVoice] = cc;
                    indexToUnisonVoice[curSub][curVoice] = v;

                    curVoice++;
                    if (curVoice == MAX_POLY)
                    {
                        curSub++;
                        curVoice = 0;
                        if (curSub == n_sub_vcos && v != nVoices - 1 && cc != nChan - 1)
                        {
                            isInErrorState = true;
                            errorCondition =
                                fmt::format("{} * {} too many voices for any vco configuration",
                                            nVoices, currChan);
                            curSub = 0;
                        }
                    }
                }
            }
            maxUsedSubVCO = curSub;

            // This happens infrequently so this inefficient algo is fine
            for (int i = 0; i < n_sub_vcos; ++i)
            {
                channelsPerSubOct[i] = 0;
                for (int j = 0; j < MAX_POLY; ++j)
                {
                    if (subVcoToInputChannel[i][j] >= 0)
                        channelsPerSubOct[i] = j + 1;
                }
            }

#define DEBUG_LAYOUT 0
#if DEBUG_LAYOUT
            std::cout << "REBUILD STATUS\n";
            std::cout << "  errorCond  = " << isInErrorState << " "
                      << (isInErrorState ? errorCondition : "") << "\n";
            std::cout << "  maxUsedSub = " << maxUsedSubVCO << std::endl;
            std::cout << "  chanPerSub = ";
            for (auto &v : channelsPerSubOct)
                std::cout << v << " ";
            std::cout << "\n";

            std::cout << "  subVcoToInput:\n";
            for (auto &a : subVcoToInputChannel)
            {
                std::cout << "      | ";
                for (auto &v : a)
                {
                    std::cout << v << " ";
                }
                std::cout << "\n";
            }
            std::cout << "  indexToUnisonVoice:\n";
            for (auto &a : indexToUnisonVoice)
            {
                std::cout << "      | ";
                for (auto &v : a)
                {
                    std::cout << v << " ";
                }
                std::cout << "\n";
            }
            std::cout << std::endl;
#endif
        }

        outputs[OUTPUT_L].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);

        if (samplePos == 0)
        {
            updateConnectedSet();
        }

        for (int i = 0; i < currChan; ++i)
        {
            if (samplePos == 0)
            {
                driftLFO[i].next();
                driftLFOLag[i].newValue(driftLFO[i].val());
            }

            baseVOct[i] = inputs[INPUT_VOCT].getVoltage(i) +
                          params[DRIFT].getValue() * driftLFOLag[i].getTargetValue() / 12.0;
            driftLFOLag[i].process();
        }

        std::array<float, 16> outputL{}, outputR{};

        auto dt = params[DETUNE].getValue() / 12.0;
        if (params[DETUNE_EXTEND].getValue() > 0.5)
            dt = dt * 12;

        for (auto v = 0; v <= maxUsedSubVCO; ++v)
        {
            outputs[OUTPUT_VOCT_SUB1 + v].setChannels(channelsPerSubOct[v]);
            for (auto p = 0; p < MAX_POLY; ++p)
            {
                auto svi = subVcoToInputChannel[v][p];
                auto vi = indexToUnisonVoice[v][p];
                if (svi >= 0)
                {
                    auto vo = baseVOct[svi] + unisonSetup.detune(vi) * dt;
                    outputs[OUTPUT_VOCT_SUB1 + v].setVoltage(vo, p);

                    auto iv = inputs[INPUT_SUB1 + v].getVoltage(p);
                    float pL, pR;
                    unisonSetup.attenuatedPanLaw(vi, pL, pR);

                    outputL[svi] += iv * pL;
                    outputR[svi] += iv * pR;
                }
            }
        }

        for (int i = 0; i < currChan; ++i)
        {
            characterFilter[i].process_block_stereo(&outputL[i], &outputR[i], 1);
        }

        for (int i = 0; i < currChan; ++i)
        {
            outputs[OUTPUT_L].setVoltage(outputL[i], i);
            outputs[OUTPUT_R].setVoltage(outputR[i], i);
        }

        samplePos = (samplePos + 1) & (BLOCK_SIZE - 1);
    }

    void updateConnectedSet()
    {
        connectedSetChanged = false;
        auto connectedSoFar = true;
        highestContiguousConnectedSub = -1;
        for (int i = 0; i < n_sub_vcos; ++i)
        {
            auto tc =
                inputs[INPUT_SUB1 + i].isConnected() && outputs[OUTPUT_VOCT_SUB1 + i].isConnected();
            if (tc != connectedSet[i])
                connectedSetChanged = true;
            connectedSet[i] = tc;
            connectedSoFar = connectedSoFar && tc;
            if (connectedSoFar)
                highestContiguousConnectedSub = i;
        }
    }
};
} // namespace sst::surgext_rack::unisonhelper
#endif
