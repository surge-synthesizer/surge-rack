/*
** A common base class for all of our Modules, introducing core functions to
** make sure surge is correctly configured in your module
*/

#pragma once
#include <iostream>
#include <locale>
#include <string>
#include <array>
#include <atomic>

#include "SurgeXT.hpp"
#include "SurgeStorage.h"
#include "rack.hpp"
#include "XTStyle.hpp"

#include "filesystem/import.h"
#include <fmt/core.h>

namespace logger = rack::logger;
using rack::appGet;

#include <map>
#include <vector>

namespace sst::surgext_rack::modules
{
struct XTModule : public rack::Module
{
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

    void setupSurgeCommon(int NUM_PARAMS, bool loadWavetables)
    {
        std::string dataPath = SurgeStorage::skipPatchLoadDataPathSentinel;

        if (loadWavetables)
            dataPath = rack::asset::plugin(pluginInstance, "build/surge-data/");

        showBuildInfo();
        storage = std::make_unique<SurgeStorage>(dataPath);

        std::atomic<bool> showedPathsOnce{false};
        if (!showedPathsOnce)
        {
            showedPathsOnce = true;
            INFO("[SurgeXTRack] storage::dataPath = '%s'", storage->datapath.c_str());
            INFO("[SurgeXTRack] storage::userDataPath = '%s'", storage->userDataPath.c_str());
        }

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

    std::unique_ptr<SurgeStorage> storage;
    int storage_id_start, storage_id_end;

    json_t *makeCommonDataJson()
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "buildInfo", json_string(getBuildInfo().c_str()));
        return rootJ;
    }

    void readCommonDataJson(json_t *commonJ) {}

    virtual json_t *makeModuleSpecificJson() { return nullptr; }
    virtual void readModuleSpecificJson(json_t *modJ) {}

    virtual json_t *dataToJson() override
    {
        json_t *commonJ = makeCommonDataJson();
        json_t *moduleSpecificJ = makeModuleSpecificJson();

        json_t *rootJ = json_object();
        if (commonJ)
        {
            json_object_set(rootJ, "xtshared", commonJ);
        }
        if (moduleSpecificJ)
        {
            json_object_set(rootJ, "modulespecific", moduleSpecificJ);
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
};

struct SurgeParameterParamQuantity : public rack::engine::ParamQuantity
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
    virtual std::string getDisplayValueString() override
    {
        auto par = surgepar();
        if (!par)
        {
            return ParamQuantity::getDisplayValueString();
        }

        char txt[256];
        par->get_display(txt, true, getValue());
        char talt[256];
        par->get_display_alt(talt, true, getValue());
        if (strlen(talt))
        {
            return std::string(txt) + " (" + talt + ")";
        }

        if (par->temposync)
        {
            return std::string(txt) + " @ " +
                   fmt::format("{:.1f}bpm", xtm()->storage->temposyncratio * 120);
        }
        return txt;
    }

    bool isCoupledToGlobalStyle{true};
    style::XTStyle::Style localStyle;
    style::XTStyle::LightColor localLightColor, localModLightColor;
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
    std::string getDisplayValueString() override
    {
        auto v = getValue();
        if (v < 0.0001)
            return "-inf dB";
        auto dbv = 18.0 * std::log2(v);
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
        auto v = pow(2.f, q / 18.0);
        if (v >= 0 && v <= 2)
        {
            setValue(v);
            return;
        }

        setValue(1.f);
    }
};

template <typename M, uint32_t nPar, uint32_t par0, uint32_t nInputs, uint32_t input0>
struct MonophonicModulationAssistant
{
    float f[nPar], fInv[nPar];
    float mu[nPar][nInputs];
    __m128 muSSE[nPar][nInputs];
    float values alignas(16)[nPar];
    float basevalues alignas(16)[nPar];
    float modvalues alignas(16)[nPar];
    bool connected[nInputs];
    void initialize(M *m)
    {
        for (auto p = 0; p < nPar; ++p)
        {
            auto pq = m->paramQuantities[p + par0];
            f[p] = (pq->maxValue - pq->minValue);
            fInv[p] = 1.0 / f[p];
        }
        setupMatrix(m);
    }

    void setupMatrix(M *m)
    {
        for (auto p = 0; p < nPar; ++p)
        {
            for (auto i = 0; i < nInputs; ++i)
            {
                auto idx = m->modulatorIndexFor(p + par0, i);
                mu[p][i] = m->params[idx].getValue() * f[p];
            }
        }
    }

    void updateValues(M *m)
    {
        for (int i = 0; i < nInputs; ++i)
        {
            connected[i] = m->inputs[i + input0].isConnected();
        }

        float inp[nInputs];
        for (int i = 0; i < nInputs; ++i)
        {
            inp[i] = m->inputs[i + input0].getVoltage(0) * RACK_TO_SURGE_CV_MUL;
        }
        for (int p = 0; p < nPar; ++p)
        {
            // Set up the base values
            auto mv = 0.f;
            for (int i = 0; i < nInputs; ++i)
            {
                mv += connected[i] * mu[p][i] * inp[i];
            }
            modvalues[p] = mv;
            basevalues[p] = m->params[p + par0].getValue();
            ;
            values[p] = mv + basevalues[p];
        }
    }
};

template <typename M, uint32_t nPar, uint32_t par0, uint32_t nInputs, uint32_t input0>
struct ModulationAssistant
{
    float f[nPar], fInv[nPar];
    float mu[nPar][nInputs];
    __m128 muSSE[nPar][nInputs];
    float values alignas(16)[nPar][MAX_POLY];
    float basevalues alignas(16)[nPar];
    float modvalues alignas(16)[nPar][MAX_POLY];
    __m128 valuesSSE[nPar][MAX_POLY >> 2];
    float animValues[nPar];
    bool connected[nInputs];
    void initialize(M *m)
    {
        for (auto p = 0; p < nPar; ++p)
        {
            auto pq = m->paramQuantities[p + par0];
            f[p] = (pq->maxValue - pq->minValue);
            fInv[p] = 1.0 / f[p];
        }
        setupMatrix(m);
    }

    void setupMatrix(M *m)
    {
        for (auto p = 0; p < nPar; ++p)
        {
            for (auto i = 0; i < nInputs; ++i)
            {
                auto idx = m->modulatorIndexFor(p + par0, i);
                mu[p][i] = m->params[idx].getValue() * f[p];
                muSSE[p][i] = _mm_load1_ps(&mu[p][i]);
            }
        }
    }

    void updateValues(M *m)
    {
        auto ci = 0;
        bool broadcast[nInputs];
        auto chans = std::max({1, m->polyChannelCount(), ci});

        for (int i = 0; i < nInputs; ++i)
        {
            connected[i] = m->inputs[i + input0].isConnected();
            if (connected[i])
            {
                auto ch = m->inputs[i + input0].getChannels();
                ci = std::max(ci, ch);
                broadcast[i] = ch != chans;
            }
            else
            {
                broadcast[i] = false; // to have a value at least
            }
        }

        if (chans == 1)
        {
            // Special case: chans = 1 can skip all the channel loops
            float inp[nInputs];
            for (int i = 0; i < nInputs; ++i)
            {
                inp[i] = m->inputs[i + input0].getVoltage(0) * RACK_TO_SURGE_CV_MUL;
            }
            for (int p = 0; p < nPar; ++p)
            {
                // Set up the base values
                auto mv = 0.f;
                for (int i = 0; i < nInputs; ++i)
                {
                    mv += connected[i] * mu[p][i] * inp[i];
                }
                modvalues[p][0] = mv;
                basevalues[p] = m->params[p + par0].getValue();
                ;
                values[p][0] = mv + basevalues[p];
                animValues[p] = fInv[p] * mv;
            }
        }
        else
        {
            // general poly-poly case. So what do we need to do?
            // We we have the matrix as SSEs so we need to make
            // that from the inputs and voia. But we need to deal with
            // the broadcast inputs first.
            //
            // This is structured so we can do SIMD later but lets
            // do regular way first
            float snapInputs[nInputs][MAX_POLY];
            for (int i = 0; i < nInputs; ++i)
            {
                if (broadcast[i])
                {
                    auto iv =
                        connected[i] * m->inputs[i + input0].getVoltage(0) * RACK_TO_SURGE_CV_MUL;
                    for (int c = 0; c < chans; ++c)
                    {
                        snapInputs[i][c] = iv;
                    }
                }
                else
                {
                    // This loop can SIMD-ize
                    for (int c = 0; c < chans; ++c)
                    {
                        auto iv = connected[i] * m->inputs[i + input0].getVoltage(c) *
                                  RACK_TO_SURGE_CV_MUL;
                        snapInputs[i][c] = iv;
                    }
                }
            }
            for (int p = 0; p < nPar; ++p)
            {
                float mv[MAX_POLY];
                memset(mv, 0, chans * sizeof(float));

                for (int i = 0; i < nInputs; ++i)
                {
                    // This is the loop we will simd-4 stride
                    for (int c = 0; c < chans; ++c)
                    {
                        mv[c] += connected[i] * mu[p][i] * snapInputs[i][c];
                    }
                }
                auto v0 = m->params[p + par0].getValue();
                for (int c = 0; c < chans; ++c)
                {
                    modvalues[p][c] = mv[c];
                    values[p][c] = mv[c] + v0;
                }
                basevalues[p] = v0;
                animValues[p] = fInv[p] * mv[0];
            }
        }

        for (int p = 0; p < nPar; ++p)
        {
            for (auto csse = 0; csse < (MAX_POLY >> 2); csse++)
            {
                valuesSSE[p][csse] = _mm_load_ps(&values[p][csse << 2]);
            }
        }
    }
};

template <typename T> struct ClockProcessor
{
    rack::dsp::SchmittTrigger trig;

    float sampleRate{1}, sampleRateInv{1};
    int timeSinceLast{-1};
    float lastBPM{-1};
    void setSampleRate(float sr)
    {
        sampleRate = sr;
        sampleRateInv = 1.f / sr;
    }
    inline void process(T *m, int inputId)
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
    inline void disconnect(T *m)
    {
        if (timeSinceLast >= 0)
            m->deactivateTempoSync();

        timeSinceLast = -1;
    }
};
} // namespace sst::surgext_rack::modules