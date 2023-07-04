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

#ifndef SURGE_XT_RACK_SRC_XTMODULE_H
#define SURGE_XT_RACK_SRC_XTMODULE_H
#include <iostream>
#include <locale>
#include <string>
#include <array>
#include <atomic>

#include "SurgeXT.h"
#include "SurgeStorage.h"
#include "rack.hpp"
#include "XTStyle.h"

#include "filesystem/import.h"
#include <fmt/core.h>
#include <thread>

#include <sst/plugininfra/cpufeatures.h>
#include "TemposyncSupport.h"

namespace logger = rack::logger;
using rack::appGet;

#include <map>
#include <vector>

namespace sst::surgext_rack::modules
{
struct XTModule : public rack::Module
{
    static std::mutex xtSurgeCreateMutex;

    XTModule() : rack::Module() { storage.reset(nullptr); }

    std::string getBuildInfo()
    {
        char version[1024];
        snprintf(version, 1023, "os:%s pluggit:%s surgegit:%s buildtime=%s %s",
#if WINDOWS
                 "win",
#endif
#if MAC
                 "macos",
#endif
#if LINUX
                 "linux",
#endif
                 TOSTRING(SURGE_RACK_PLUG_VERSION), TOSTRING(SURGE_RACK_SURGE_VERSION), __DATE__,
                 __TIME__);
        return std::string(version);
    }

    void showBuildInfo()
    {
        INFO("[SurgeXTRack] Instance: Module=%s BuildInfo=%s", getName().c_str(),
             getBuildInfo().c_str());
    }

    virtual std::string getName() = 0;

    virtual void onSampleRateChange() override
    {
        float sr = APP->engine->getSampleRate();
        if (storage)
        {
            storage->setSamplerate(sr);
            storage->init_tables();
            updateBPMFromClockCV(lastClockCV, storage->samplerate_inv, sr, true);
            moduleSpecificSampleRateChange();
        }
    }

    virtual void moduleSpecificSampleRateChange() {}

    static std::atomic<bool> showedPathsOnce;

    void setupSurgeCommon(int NUM_PARAMS, bool loadWavetables, bool loadFX)
    {
        SurgeStorage::SurgeStorageConfig config;
        config.suppliedDataPath = SurgeStorage::skipPatchLoadDataPathSentinel;
        config.createUserDirectory = false;

        if (loadWavetables || loadFX)
        {
            config.suppliedDataPath = rack::asset::plugin(pluginInstance, "build/surge-data/");
            config.extraThirdPartyWavetablesPath =
                fs::path{rack::asset::user("SurgeXTRack/SurgeXTRack_ExtraContent")};
            config.scanWavetableAndPatches = loadWavetables;
        }

        showBuildInfo();
        storage = std::make_unique<SurgeStorage>(config);
        storage->getPatch().init_default_values();
        storage->getPatch().copy_globaldata(storage->getPatch().globaldata);
        storage->getPatch().copy_scenedata(storage->getPatch().scenedata[0], 0);
        storage->getPatch().copy_scenedata(storage->getPatch().scenedata[1], 1);

        onSampleRateChange();
    }

    float lastBPM = -1, lastClockCV = -100;
    float dPhase = 0;
    inline bool updateBPMFromClockCV(float clockCV, float sampleTime, float sampleRate,
                                     bool force = false)
    {
        if (!force && clockCV == lastClockCV)
            return false;

        lastClockCV = clockCV;
        float clockTime = powf(2.0f, clockCV);
        dPhase = clockTime * sampleTime;
        float samplesPerBeat = 1.0 / dPhase;
        float secondsPerBeat = samplesPerBeat / sampleRate;
        float beatsPerMinute = 60.0 / secondsPerBeat;

        // Folks can put in insane BPMs if they mis-wire their rack. Lets
        // put in a rack::clamp for well beyond the usable range
        beatsPerMinute = rack::clamp(beatsPerMinute, 1.f, 1024.f);

        lastBPM = beatsPerMinute;

        if (storage.get())
        {
            // FIX ME new API
            storage->temposyncratio = beatsPerMinute / 120.0;
            storage->temposyncratio_inv = 1.f / storage->temposyncratio;
        }
        return true;
    }

    virtual bool isBipolar(int paramId) { return false; }
    virtual float modulationDisplayValue(int paramId) { return 0; }

    void copyScenedataSubset(int scene, int start, int end)
    {
        int s = storage->getPatch().scene_start[scene];
        for (int i = start; i < end; ++i)
        {
            storage->getPatch().scenedata[scene][i - s].i = storage->getPatch().param_ptr[i]->val.i;
        }
    }

    void copyGlobaldataSubset(int start, int end)
    {
        for (int i = start; i < end; ++i)
        {
            storage->getPatch().globaldata[i].i = storage->getPatch().param_ptr[i]->val.i;
        }
    }

    void setupStorageRanges(Parameter *start, Parameter *endIncluding)
    {
        int min_id = 100000, max_id = -1;
        Parameter *oap = start;
        while (oap <= endIncluding)
        {
            if (oap->id >= 0)
            {
                if (oap->id > max_id)
                    max_id = oap->id;
                if (oap->id < min_id)
                    min_id = oap->id;
            }
            oap++;
        }

        storage_id_start = min_id;
        storage_id_end = max_id + 1;
    }

    virtual Parameter *surgeDisplayParameterForParamId(int paramId) { return nullptr; }
    virtual Parameter *surgeDisplayParameterForModulatorParamId(int paramId) { return nullptr; }

    std::unique_ptr<SurgeStorage> storage;
    int storage_id_start, storage_id_end;

    json_t *makeCommonDataJson()
    {
        json_t *rootJ = json_object();
        // For future use
        json_object_set_new(rootJ, "streamingVersion", json_integer(1));
        json_object_set_new(rootJ, "buildInfo", json_string(getBuildInfo().c_str()));
        json_object_set_new(rootJ, "isCoupledToGlobalStyle", json_boolean(isCoupledToGlobalStyle));
        json_object_set_new(rootJ, "localStyle", json_integer(localStyle));
        json_object_set_new(rootJ, "localDisplayRegionColor",
                            json_integer(localDisplayRegionColor));
        json_object_set_new(rootJ, "localModulationColor", json_integer(localModulationColor));
        json_object_set_new(rootJ, "localControlValueColor", json_integer(localControlValueColor));
        json_object_set_new(rootJ, "localPowerButtonColor", json_integer(localPowerButtonColor));
        return rootJ;
    }

    void readCommonDataJson(json_t *commonJ)
    {
        auto icg = json_object_get(commonJ, "isCoupledToGlobalStyle");
        if (icg)
            isCoupledToGlobalStyle = json_boolean_value(icg);

        auto ls = json_object_get(commonJ, "localStyle");
        if (ls)
            localStyle = (style::XTStyle::Style)json_integer_value(ls);
        auto ll = json_object_get(commonJ, "localDisplayRegionColor");
        if (ll)
            localDisplayRegionColor = (style::XTStyle::LightColor)json_integer_value(ll);
        auto lm = json_object_get(commonJ, "localModulationColor");
        if (lm)
            localModulationColor = (style::XTStyle::LightColor)json_integer_value(lm);
        lm = json_object_get(commonJ, "localControlValueColor");
        if (lm)
            localControlValueColor = (style::XTStyle::LightColor)json_integer_value(lm);
        lm = json_object_get(commonJ, "localPowerButtonColor");
        if (lm)
            localPowerButtonColor = (style::XTStyle::LightColor)json_integer_value(lm);
    }

    virtual json_t *makeModuleSpecificJson() { return nullptr; }
    virtual void readModuleSpecificJson(json_t *modJ) {}

    virtual json_t *dataToJson() override
    {
        json_t *commonJ = makeCommonDataJson();
        json_t *moduleSpecificJ = makeModuleSpecificJson();

        json_t *rootJ = json_object();
        if (commonJ)
        {
            json_object_set_new(rootJ, "xtshared", commonJ);
            commonJ = nullptr;
        }
        if (moduleSpecificJ)
        {
            json_object_set_new(rootJ, "modulespecific", moduleSpecificJ);
            moduleSpecificJ = nullptr;
        }
        return rootJ;
    }
    virtual void dataFromJson(json_t *rootJ) override
    {
        auto commonJ = json_object_get(rootJ, "xtshared");
        auto specificJ = json_object_get(rootJ, "modulespecific");
        if (commonJ)
            readCommonDataJson(commonJ);
        if (specificJ)
            readModuleSpecificJson(specificJ);
    }

    template <typename T = rack::ParamQuantity, typename... Args> T *configParamNoRand(Args... args)
    {
        auto *res = configParam<T>(args...);
        res->randomizeEnabled = false;
        return res;
    }

    template <typename T = rack::SwitchQuantity>
    T *configOnOff(int paramId, float defaultValue, const std::string &name)
    {
        return configSwitch<T>(paramId, 0, 1, defaultValue, name, {"Off", "On"});
    }
    template <typename T = rack::SwitchQuantity>
    T *configOnOffNoRand(int paramId, float defaultValue, const std::string &name)
    {
        auto r = configSwitch<T>(paramId, 0, 1, defaultValue, name, {"Off", "On"});
        r->randomizeEnabled = false;
        return r;
    }

    void snapCalculatedNames();

    bool isCoupledToGlobalStyle{true};
    style::XTStyle::Style localStyle{style::XTStyle::LIGHT};
    style::XTStyle::LightColor localDisplayRegionColor{style::XTStyle::ORANGE},
        localModulationColor{style::XTStyle::BLUE}, localControlValueColor{style::XTStyle::ORANGE},
        localPowerButtonColor{style::XTStyle::GREEN};
};

struct CalculatedName
{
    virtual ~CalculatedName() = default;
    virtual std::string getCalculatedName() = 0;
};

struct SurgeParameterParamQuantity : public rack::engine::ParamQuantity, CalculatedName
{
    inline XTModule *xtm() { return static_cast<XTModule *>(module); }
    inline Parameter *surgepar()
    {
        auto mc = xtm();
        if (!mc)
        {
            return nullptr;
        }
        auto par = mc->surgeDisplayParameterForParamId(paramId);
        return par;
    }

    std::function<void(SurgeParameterParamQuantity *)> customRandomize{nullptr};
    void randomize() override
    {
        if (customRandomize)
            customRandomize(this);
        else
            ParamQuantity::randomize();
    }

    virtual void setDisplayValueString(std::string s) override
    {
        auto par = surgepar();
        if (!par)
        {
            ParamQuantity::setDisplayValueString(s);
            return;
        }

        std::string emsg;
        par->set_value_from_string(s, emsg);
        setValue(par->get_value_f01());
    }

    virtual std::string getLabel() override
    {
        auto par = surgepar();
        if (!par)
        {
            return ParamQuantity::getLabel();
        }

        return par->get_name();
    }

    std::string getCalculatedName() override
    {
        auto par = surgepar();
        if (!par)
        {
            return "Surge Parameter";
        }

        return par->get_name();
    }

    virtual std::string getDisplayValueString() override
    {
        return getDisplayValueStringForValue(getValue());
    }
    virtual std::string getDisplayValueStringForValue(float f)
    {
        auto par = surgepar();
        if (!par)
        {
            return ParamQuantity::getDisplayValueString();
        }

        /* So the param quantity has the value of the knob but that gets rounded
         * to the nearest temposync in setval_f01. Fine whatever except when stringifying
         * then we need to use the rounded value to match.
         */
        if (par->temposync)
            f = par->get_value_f01();

        char txt[256];
        par->get_display(txt, true, f);
        char talt[256];
        par->get_display_alt(talt, true, f);
        if (strlen(talt))
        {
            if (std::string(talt) == " ")
                return std::string(txt);
            return std::string(txt) + " (" + talt + ")";
        }

        if (par->temposync)
        {
            return std::string(txt) + " @ " +
                   fmt::format("{:.1f}bpm", xtm()->storage->temposyncratio * 120);
        }
        return txt;
    }
};

struct SurgeParameterModulationQuantity : public rack::engine::ParamQuantity, CalculatedName
{
    bool abbreviate = false;
    inline XTModule *xtm() { return static_cast<XTModule *>(module); }
    inline Parameter *surgepar()
    {
        auto mc = xtm();
        if (!mc)
        {
            return nullptr;
        }
        auto par = mc->surgeDisplayParameterForModulatorParamId(paramId);
        return par;
    }

    virtual void setDisplayValueString(std::string s) override
    {
        auto par = surgepar();
        if (!par)
        {
            ParamQuantity::setDisplayValueString(s);
            return;
        }

        std::string emsg;
        bool valid{false};
        float v = par->calculate_modulation_value_from_string(s, emsg, valid);
        if (valid && par->extend_range)
            v = par->get_extended(v);
        if (valid)
            setValue(v);
    }

    std::string baseName{"MOD_ERROR"};

    virtual std::string getLabel() override
    {
        auto par = surgepar();
        if (!par)
        {
            return ParamQuantity::getLabel() + " SOFTWARE ERROR";
        }

        return getCalculatedName();
    }

    std::string getCalculatedName() override
    {
        auto par = surgepar();
        if (!par)
        {
            return baseName + " to Unkown Surge Parameter";
        }

        return baseName + " to " + par->get_name();
    }

    virtual std::string getDisplayValueString() override
    {
        auto par = surgepar();
        if (!par)
        {
            return ParamQuantity::getDisplayValueString();
        }

        char txt[256], txt2[256];
        ModulationDisplayInfoWindowStrings iw;
        auto norm = surgepar()->val_max.f - surgepar()->val_min.f;
        par->get_display_of_modulation_depth(txt, getValue() * norm, true,
                                             Parameter::ModulationDisplayMode::InfoWindow, &iw);
        par->get_display_of_modulation_depth(txt2, getValue() * norm, true,
                                             Parameter::ModulationDisplayMode::Menu);

        if (iw.val.empty())
            return txt2;

        std::ostringstream oss;
        oss << iw.dvalplus << "\n"
            << iw.val << " @ 0v\n"
            << iw.valplus << " @ 10v\n"
            << iw.valminus << " @ -10v";
        if (abbreviate)
            return iw.dvalplus;
        return oss.str();
    }
};

template <int centerOffset> struct VOctParamQuantity : public rack::engine::ParamQuantity
{
    void setDisplayValueString(std::string s) override
    {
        auto f = std::atof(s.c_str());
        if (f > 0)
        {
            auto midiNote = 12 * log2(f / 440) + 69;
            setValue((midiNote - centerOffset) / 12.f);
        }
        else if ((s[0] >= 'A' && s[0] <= 'G') || (s[0] >= 'a' && s[0] <= 'g'))
        {
            int opos = 1;
            int halfOff = 0;
            if (s[1] == '#')
            {
                halfOff = 1;
                opos++;
            }
            if (s[1] == 'b')
            {
                halfOff = -1;
                opos++;
            }
            int octave = std::atoi(s.c_str() + opos);

            int ws = 0;
            switch (std::toupper(s[0]))
            {
            case 'C':
                ws = 0;
                break;
            case 'D':
                ws = 2;
                break;
            case 'E':
                ws = 4;
                break;
            case 'F':
                ws = 5;
                break;
            case 'G':
                ws = 7;
                break;
            case 'A':
                ws = 9;
                break;
            case 'B':
                ws = 11;
                break;
            }
            auto mnote = (octave + 1) * 12 + ws + halfOff;
            setValue((mnote - centerOffset) / 12.f);
        }
        else
        {
            setValue(0);
        }
    }

    virtual std::string getDisplayValueString() override
    {
        auto note = getValue() * 12 + centerOffset;
        auto freq = 440.0 * pow(2.0, (note - 69) / 12);

        auto noteR = std::round(note);
        auto noteO = (int)(noteR) % 12;
        int oct = (int)std::round((noteR - noteO) / 12 - 1);

        static constexpr std::array<char[3], 12> names{"C",  "C#", "D",  "D#", "E",  "F",
                                                       "F#", "G",  "G#", "A",  "A#", "B"};

        return fmt::format("{:6.2f} Hz (~{}{})", freq, names[noteO], oct);
    }
};

template <int centerOffset> struct MidiNoteParamQuantity : public rack::engine::ParamQuantity
{
    void setDisplayValueString(std::string s) override
    {
        auto f = std::atof(s.c_str());
        if (f > 0)
        {
            auto val = 12 * log2(f / 440) + 69 - centerOffset;
            setValue(val);
        }
        else if ((s[0] >= 'A' && s[0] <= 'G') || (s[0] >= 'a' && s[0] <= 'g'))
        {
            int opos = 1;
            int halfOff = 0;
            if (s[1] == '#')
            {
                halfOff = 1;
                opos++;
            }
            if (s[1] == 'b')
            {
                halfOff = -1;
                opos++;
            }
            int octave = std::atoi(s.c_str() + opos);

            int ws = 0;
            switch (std::toupper(s[0]))
            {
            case 'C':
                ws = 0;
                break;
            case 'D':
                ws = 2;
                break;
            case 'E':
                ws = 4;
                break;
            case 'F':
                ws = 5;
                break;
            case 'G':
                ws = 7;
                break;
            case 'A':
                ws = 9;
                break;
            case 'B':
                ws = 11;
                break;
            }
            auto mnote = (octave + 1) * 12 + ws + halfOff;
            setValue(mnote - centerOffset);
        }
        else
        {
            setValue(centerOffset);
        }
    }

    virtual std::string getDisplayValueString() override
    {
        auto note = getValue() + centerOffset;
        auto freq = 440.0 * pow(2.0, (note - 69) / 12);

        auto noteR = std::round(note);
        auto noteO = (int)(noteR) % 12;
        int oct = (int)std::round((noteR - noteO) / 12 - 1);

        static constexpr std::array<char[3], 12> names{"C",  "C#", "D",  "D#", "E",  "F",
                                                       "F#", "G",  "G#", "A",  "A#", "B"};

        return fmt::format("{:6.2f} Hz (~{}{})", freq, names[noteO], oct);
    }
};

struct DecibelParamQuantity : rack::engine::ParamQuantity
{
    static float ampToLinear(float xin)
    {
        auto x = std::max(0.f, xin);
        return x * x * x;
    }
    static __m128 ampToLinearSSE(__m128 xin)
    {
        auto x = _mm_max_ss(xin, _mm_setzero_ps());
        return _mm_mul_ps(x, _mm_mul_ps(x, x));
    }
    static float linearToAmp(float x)
    {
        // display only so don't need an SSE version of this
        return powf(std::max(x, 0.f), 1.f / 3.f);
    }

    std::string getDisplayValueString() override
    {
        auto v = getValue();
        if (v < 0.0001)
            return "-inf dB";
        auto dbv = 6.0 * std::log2(ampToLinear(v));
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
        auto v = linearToAmp(pow(2.f, q / 6.0));
        if (v >= 0 && v <= 2)
        {
            setValue(v);
            return;
        }

        setValue(1.f);
    }
};

template <typename M> struct DecibelModulatorParamQuantity : rack::ParamQuantity
{
    inline M *xtm() { return static_cast<M *>(module); }
    inline ParamQuantity *under()
    {
        auto m = xtm();
        if (!m)
            return nullptr;

        auto underParamId = m->paramModulatedBy(paramId);
        if (underParamId < 0)
            return nullptr;

        return m->paramQuantities[underParamId];
    }
    std::string getLabel() override
    {
        auto upq = under();
        if (!upq)
            return ParamQuantity::getLabel();
        return ParamQuantity::getLabel() + " to " + upq->getLabel();
    }
};

template <typename M, uint32_t nPar, uint32_t par0, uint32_t nInputs, uint32_t input0>
struct MonophonicModulationAssistant
{
    float f[nPar], fInv[nPar];
    float mu[nPar][nInputs];
    float values alignas(16)[nPar];
    float basevalues alignas(16)[nPar];
    float modvalues alignas(16)[nPar];
    void initialize(M *m)
    {
        for (auto p = 0U; p < nPar; ++p)
        {
            auto pq = m->paramQuantities[p + par0];
            f[p] = (pq->maxValue - pq->minValue);
            fInv[p] = 1.0 / f[p];
        }
        setupMatrix(m);
    }

    void setupMatrix(M *m)
    {
        for (auto p = 0U; p < nPar; ++p)
        {
            for (auto i = 0U; i < nInputs; ++i)
            {
                auto idx = m->modulatorIndexFor(p + par0, i);
                mu[p][i] = m->params[idx].getValue() * f[p];
            }
        }
    }

    void updateValues(M *m)
    {
        float inp[4];
        for (auto i = 0U; i < nInputs; ++i)
        {
            inp[i] = m->inputs[i + input0].isConnected() * m->inputs[i + input0].getVoltage(0) *
                     RACK_TO_SURGE_CV_MUL;
        }
        for (auto p = 0U; p < nPar; ++p)
        {
            // Set up the base values
            auto mv = 0.f;
            for (auto i = 0U; i < nInputs; ++i)
            {
                mv += (mu[p][i] * inp[i]);
            }
            modvalues[p] = mv;
            basevalues[p] = m->params[p + par0].getValue();
            values[p] = mv + basevalues[p];
        }
    }
};

template <typename M, uint32_t nPar, uint32_t par0, uint32_t nInputs, uint32_t input0>
struct ModulationAssistant
{
    float f alignas(16)[nPar], fInv alignas(16)[nPar];
    float mu alignas(16)[nPar][nInputs];
    float values alignas(16)[nPar][MAX_POLY];
    float basevalues alignas(16)[nPar];
    float modvalues alignas(16)[nPar][MAX_POLY];
    __m128 valuesSSE alignas(16)[nPar][MAX_POLY >> 2];
    __m128 muSSE alignas(16)[nPar][nInputs];
    float animValues alignas(16)[nPar];

    bool connected[nInputs];
    bool connectedParameter[nPar];
    bool broadcast[nInputs];
    int chans{1};
    bool anyConnected{false};
    void initialize(M *m)
    {
        for (auto p = 0U; p < nPar; ++p)
        {
            auto pq = m->paramQuantities[p + par0];
            f[p] = (pq->maxValue - pq->minValue);
            fInv[p] = 1.0 / f[p];
        }
        setupMatrix(m);
    }

    void setupMatrix(M *m)
    {
        chans = std::max(1, m->polyChannelCount());

        anyConnected = false;
        for (auto i = 0U; i < nInputs; ++i)
        {
            connected[i] = m->inputs[i + input0].isConnected();
            anyConnected = anyConnected || connected[i];
            if (connected[i])
            {
                auto ch = m->inputs[i + input0].getChannels();
                broadcast[i] = ch == 1 && chans != 1;
            }
            else
            {
                broadcast[i] = false; // to have a value at least
            }
        }

        for (auto p = 0U; p < nPar; ++p)
        {
            auto sm = 0.f;
            for (auto i = 0U; i < nInputs; ++i)
            {
                auto idx = m->modulatorIndexFor(p + par0, i);
                mu[p][i] = m->params[idx].getValue() * f[p];
                sm += fabs(mu[p][i]);
                muSSE[p][i] = _mm_set1_ps(mu[p][i]);
            }
            connectedParameter[p] = (sm > 1e-6f) && anyConnected;
        }
    }

    void updateValues(M *m)
    {
        if (chans == 1)
        {
            // Special case: chans = 1 can skip all the channel loops
            float inp[nInputs];
            for (auto i = 0U; i < nInputs; ++i)
            {
                inp[i] = connected[i] * m->inputs[i + input0].getVoltage(0) * RACK_TO_SURGE_CV_MUL;
            }
            for (auto p = 0U; p < nPar; ++p)
            {
                // Set up the base values
                auto mv = 0.f;
                if (connectedParameter[p])
                {
                    for (auto i = 0U; i < nInputs; ++i)
                    {
                        mv += mu[p][i] * inp[i];
                    }
                }
                modvalues[p][0] = mv;
                basevalues[p] = m->params[p + par0].getValue();
                values[p][0] = mv + basevalues[p];
                valuesSSE[p][0] = _mm_set1_ps(values[p][0]);

                animValues[p] = fInv[p] * mv;
            }
        }
        else
        {
            const auto r2scv = _mm_set1_ps(RACK_TO_SURGE_CV_MUL);
            int polyChans = (chans - 1) / 4 + 1;
            __m128 snapInputs[nInputs][MAX_POLY >> 2];
            for (auto i = 0U; i < nInputs; ++i)
            {
                if (!connected[i])
                {
                    for (int c = 0; c < polyChans; ++c)
                    {
                        snapInputs[i][c] = _mm_setzero_ps();
                    }
                }
                else if (broadcast[i])
                {
                    auto iv = m->inputs[i + input0].getVoltage(0) * RACK_TO_SURGE_CV_MUL;
                    for (int c = 0; c < polyChans; ++c)
                    {
                        snapInputs[i][c] = _mm_set1_ps(iv);
                    }
                }
                else
                {
                    // This loop can SIMD-ize
                    for (int c = 0; c < polyChans; ++c)
                    {
                        auto v = _mm_loadu_ps(m->inputs[i + input0].getVoltages(c * 4));
                        v = _mm_mul_ps(v, r2scv);
                        snapInputs[i][c] = v;
                    }
                }
            }
            for (auto p = 0U; p < nPar; ++p)
            {
                if (!connectedParameter[p])
                {
                    basevalues[p] = m->params[p + par0].getValue();
                    auto v0 = _mm_set1_ps(basevalues[p]);

                    for (int c = 0; c < polyChans; ++c)
                    {
                        _mm_store_ps(&modvalues[p][c * 4], _mm_setzero_ps());
                        valuesSSE[p][c] = v0;
                        _mm_store_ps(&values[p][c * 4], valuesSSE[p][c]);
                    }

                    animValues[p] = fInv[p] * modvalues[p][0];
                }
                else
                {
                    __m128 mv[MAX_POLY >> 2];
                    memset(mv, 0, polyChans * sizeof(__m128));

                    for (auto i = 0U; i < nInputs; ++i)
                    {
                        if (!connected[i])
                            continue;

                        // This is the loop we will simd-4 stride
                        for (int c = 0; c < polyChans; ++c)
                        {
                            mv[c] = _mm_add_ps(mv[c], _mm_mul_ps(muSSE[p][i], snapInputs[i][c]));
                        }
                    }

                    basevalues[p] = m->params[p + par0].getValue();
                    auto v0 = _mm_set1_ps(basevalues[p]);

                    for (int c = 0; c < polyChans; ++c)
                    {
                        _mm_store_ps(&modvalues[p][c * 4], mv[c]);
                        valuesSSE[p][c] = _mm_add_ps(v0, mv[c]);
                        _mm_store_ps(&values[p][c * 4], valuesSSE[p][c]);
                    }

                    animValues[p] = fInv[p] * modvalues[p][0];
                }
            }
        }
    }
};

template <typename T> struct ClockProcessor
{
    enum ClockStyle
    {
        QUARTER_NOTE,
        BPM_VOCT
    } clockStyle{QUARTER_NOTE};

    rack::dsp::SchmittTrigger trig;

    float sampleRate{1}, sampleRateInv{1};
    int timeSinceLast{-1};
    float lastBPM{-1}, lastBPMVolts{-11};
    bool bpmConnected{false};

    inline void process(T *m, int inputId)
    {
        assert(sampleRate > 100);

        if (clockStyle == BPM_VOCT)
        {
            if (!bpmConnected)
                m->activateTempoSync();
            bpmConnected = true;
            auto iv = m->inputs[inputId].getVoltage();
            if (iv != lastBPMVolts)
            {
                auto bpmRatio = pow(2.0, iv);
                m->storage->temposyncratio = bpmRatio;
                m->storage->temposyncratio_inv = 1.f / bpmRatio;
            }
            lastBPMVolts = iv;
        }
        else
        {
            if (trig.process(m->inputs[inputId].getVoltage()))
            {
                // If we have 10bpm don't update BPM. It's probably someone stopping
                // their clock for a while.
                if (timeSinceLast > 0 && timeSinceLast < sampleRate * 6)
                {
                    auto bpm = 60 * sampleRate / timeSinceLast;

                    // OK we are going to make an assumption
                    // that BPM is *probably* integral at least
                    // if we are within a smidge of an integer
                    auto d = std::abs(bpm - std::round(bpm));
                    if (d < 0.015)
                    {
                        bpm = std::round(bpm);
                    }
                    if (bpm != lastBPM)
                    {
                        m->storage->temposyncratio = bpm / 120.f;
                        m->storage->temposyncratio_inv = 120.f / bpm;
                    }
                    lastBPM = bpm;
                }
                else
                {
                    m->activateTempoSync();
                }
                timeSinceLast = 0;
            }
            timeSinceLast += (timeSinceLast >= 0);
        }
    }
    inline void disconnect(T *m)
    {
        if (timeSinceLast >= 0 || bpmConnected)
            m->deactivateTempoSync();

        timeSinceLast = -1;
        bpmConnected = false;
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr;
        sampleRateInv = 1.f / sr;
    }

    void toJson(json_t *onto)
    {
        json_object_set_new(onto, "clockStyle", json_integer((int)clockStyle));
    }

    void fromJson(json_t *modJ)
    {
        auto cs = json_object_get(modJ, "clockStyle");
        if (cs)
        {
            auto csv = json_integer_value(cs);
            clockStyle = static_cast<ClockStyle>(csv);
        }
    }
};

// A block wise single channel DC Blocker
struct DCBlocker
{
    float xN1{0}, yN1{0};
    float fac{0.9995};
    DCBlocker() { reset(); }
    void reset()
    {
        xN1 = 0.f;
        yN1 = 0.f;
    }

    inline void filter(float *x) // BLOCK_SIZE
    {
        for (auto i = 0; i < BLOCK_SIZE; ++i)
        {
            auto dx = x[i] - xN1;
            auto fv = dx + fac * yN1;

            xN1 = x[i];
            yN1 = fv;

            x[i] = fv;
        }
    }
};

// A sample-wise 4-across SIMD dc blocker
struct DCBlockerSIMD4
{
    __m128 fac, xN1, yN1;
    DCBlockerSIMD4()
    {
        fac = _mm_set1_ps(0.9995);
        reset();
    }
    void reset()
    {
        xN1 = _mm_setzero_ps();
        yN1 = _mm_setzero_ps();
    }

    inline __m128 filter(__m128 x) // BLOCK_SIZE
    {
        auto dx = _mm_sub_ps(x, xN1);
        auto fv = _mm_add_ps(dx, _mm_mul_ps(fac, yN1));
        xN1 = x;
        yN1 = fv;
        return fv;
    }
};

inline void XTModule::snapCalculatedNames()
{
    for (auto *pq : paramQuantities)
    {
        if (auto *s = dynamic_cast<modules::CalculatedName *>(pq))
        {
            pq->name = s->getCalculatedName();
        }
    }
}

struct TypeSwappingParameterQuantity : rack::ParamQuantity, modules::CalculatedName
{
    TypeSwappingParameterQuantity() {}

    virtual int mode() = 0;
    rack::ParamQuantity *under()
    {
        auto m = mode();
        auto f = impls.find(m);
        assert(f != impls.end());
        if (f == impls.end())
            return nullptr;
        if (f->second->module != module)
        {
            f->second->module = module;
            f->second->paramId = paramId;
        }
        return f->second.get();
    }

    std::unordered_map<int, std::unique_ptr<rack::ParamQuantity>> impls;
    template <typename T> void addImplementer(int mode) { impls[mode] = std::make_unique<T>(); }

    std::string getDisplayValueString() override
    {
        const auto u = under();
        if (u)
            return u->getDisplayValueString();
        return {};
    }

    std::string getLabel() override
    {
        const auto u = under();
        if (u)
            return u->getLabel();
        return {};
    }

    void randomize() override
    {
        const auto u = under();
        if (u)
            u->randomize();
    }

    void setDisplayValueString(std::string s) override
    {
        const auto u = under();
        if (u)
            u->setDisplayValueString(s);
    }

    std::string getCalculatedName() override
    {
        const auto u = under();
        const auto cn = dynamic_cast<modules::CalculatedName *>(u);
        if (cn)
            return cn->getCalculatedName();
        if (u)
            return u->name;
        return {};
    }
};

struct CTEnvTimeParamQuantity : rack::ParamQuantity, modules::CalculatedName
{
    static constexpr float defaultEtMin{-8}, defaultEtMax{3.32192809489}; // log2(10)
    float etMin{defaultEtMin}, etMax{defaultEtMax};

    std::string getLabel() override { return getCalculatedName(); }
    std::string getDisplayValueString() override
    {
        auto v = getValue() * (etMax - etMin) + etMin;

        if (getValue() < 0.0001)
        {
            std::string mv;
            if (getMinString(mv))
            {
                return mv;
            }
        }
        if (isTempoSync())
        {
            return temposync_support::temposyncLabel(v);
        }
        auto rs = fmt::format("{:.4f} s", pow(2, v));
        return rs;
    }
    void setDisplayValueString(std::string s) override
    {
        auto q = std::atof(s.c_str());
        auto v = log2(std::clamp(q, pow(2., etMin), pow(2., etMax)));
        auto vn = (v - etMin) / (etMax - etMin);
        setValue(vn);
    }

    virtual bool getMinString(std::string &s) { return false; }
    virtual bool isTempoSync() { return false; }
};

struct ModulateFromToParamQuantity : public rack::ParamQuantity, CalculatedName
{
    int modSource{0}, targetIndex{0};
    void setup(int ms, int ti)
    {
        this->modSource = ms;
        this->targetIndex = ti;
        this->name = getCalculatedName();
    }
    std::string getLabel() override { return getCalculatedName(); }
    std::string getCalculatedName() override
    {
        auto nm = "Mod " + std::to_string(modSource + 1) + " to " +
                  module->paramQuantities[targetIndex]->getLabel();
        return nm;
    }
};

} // namespace sst::surgext_rack::modules
#endif // SCXT_SRC_XTMODULE_H
