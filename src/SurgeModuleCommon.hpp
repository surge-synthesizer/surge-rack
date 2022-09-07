/*
** A common base class for all of our Modules, introducing core functions to
** make sure surge is correctly configured in your module
*/

#pragma once
#include <iostream>
#include <locale>
#include <string>

#include "SurgeXT.hpp"
#include "SurgeStorage.h"
#include "rack.hpp"

#include "filesystem/import.h"

namespace logger = rack::logger;
using rack::appGet;

#include <map>
#include <vector>

/*
** Bind a surge parameter to a param/cv_id combo. If you only have a knob (no
*cv) set the cv_id to -1
*/
struct SurgeRackParamBinding;
struct SurgeModuleCommon;

struct ParamCache
{
    std::vector<float> cache;
    int np;
    ParamCache()
    {
        np = 0;
        resize(np);
    }

    void resize(int n)
    {
        np = n;
        cache.resize(n);
        for (int i = 0; i < n; ++i)
            cache[i] = /* float min */ -1328142.0;
    }

    void update(rack::Module *m)
    {
        for (auto i = 0; i < np; ++i)
        {
            cache[i] = m->params[i].getValue();
        }
    }

    float get(int i) const { return cache[i]; }

    bool changed(int i, rack::Module *m) const { return cache[i] != m->params[i].getValue(); }
    bool changedInt(int i, rack::Module *m) const
    {
        return (int)cache[i] != (int)m->params[i].getValue();
    }
    bool changedAndIsNonZero(int i, rack::Module *m) const
    {
        auto r = m->params[i].getValue();
        return cache[i] != r && r > 0.5;
    }
};

struct SurgeModuleCommon : public rack::Module
{
    SurgeModuleCommon() : rack::Module() { storage.reset(nullptr); }

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
        INFO("[SurgeRack] Instance: Module=%s BuildInfo=%s", getName().c_str(),
             getBuildInfo().c_str());
    }

    virtual std::string getName() = 0;

    virtual void onSampleRateChange() override
    {
        float sr = APP->engine->getSampleRate();
        if (storage)
        {
            storage->setSamplerate(sr);
            // samplerate = sr;
            // dsamplerate = sr;
            // samplerate_inv = 1.0 / sr;
            // dsamplerate_inv = 1.0 / sr;
            // dsamplerate_os = dsamplerate * OSC_OVERSAMPLING;
            // dsamplerate_os_inv = 1.0 / dsamplerate_os;

            storage->init_tables();
            updateBPMFromClockCV(lastClockCV, storage->samplerate_inv, sr, true);
            moduleSpecificSampleRateChange();
        }
    }

    virtual void moduleSpecificSampleRateChange() {}

    void setupSurgeCommon(int NUM_PARAMS);

    std::vector<std::shared_ptr<SurgeRackParamBinding>> pb;
    ParamCache pc;

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
            storage->temposyncratio = beatsPerMinute / 120.0;
            storage->temposyncratio_inv = 1.f / storage->temposyncratio;
        }
        return true;
    }

    // These are vestigal shortcuts from when this code supported 0.6.2 and 1.0
    inline float getParam(int id) { return this->params[id].getValue(); }

    inline void setParam(int id, float v) { this->params[id].setValue(v); }

    inline void setLight(int id, float val) { this->lights[id].setBrightness(val); }

    inline bool inputConnected(int id) { return this->inputs[id].isConnected(); }

    inline bool outputConnected(int id) { return this->outputs[id].isConnected(); }

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

    std::unique_ptr<SurgeStorage> storage;
    int storage_id_start, storage_id_end;

    std::string comment = "No Comment";
    virtual json_t *makeCommonDataJson()
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "comment", json_string(comment.c_str()));
        json_object_set_new(rootJ, "buildInfo", json_string(getBuildInfo().c_str()));
        return rootJ;
    }

    bool firstRespawnIsFromJSON = false;

    virtual void readCommonDataJson(json_t *rootJ)
    {
        firstRespawnIsFromJSON = true;
        json_t *com = json_object_get(rootJ, "comment");
        const char *comchar = json_string_value(com);
        comment = comchar;
    }

    virtual json_t *dataToJson() override
    {
        json_t *rootJ = makeCommonDataJson();
        return rootJ;
    }
    virtual void dataFromJson(json_t *rootJ) override { readCommonDataJson(rootJ); }
};

struct StringCache
{
    std::string value;
    bool dirty;
    std::function<std::string()> getValue;
    std::function<bool()> getDirty;

    StringCache()
    {
        value = "";
        dirty = true;
        getValue = [this]() { return this->value; };
        getDirty = [this]() {
            auto res = dirty;
            dirty = false;
            return res;
        };
    }

    void reset(std::string s)
    {
        value = s;
        dirty = true;
    }

    void resetCheck(std::string s)
    {
        if (s != value)
            reset(s);
    }
};

struct SurgeRackParamBinding
{
    Parameter *p;
    int param_id, cv_id, ts_id, ext_id, deact_id;

    typedef enum UpdateType
    {
        FLOAT,
        INT,
        BOOL,
        BOOL_NOT
    } UpdateType;

    UpdateType updateType;
    StringCache valCache;
    StringCache nameCache;

    bool forceRefresh = false;
    bool tsbpmLabel = false;
    enum
    {
        CONSTANT,
        PARAM
    } deactivationMode = CONSTANT;
    bool deactivationAlways = true;

    SurgeRackParamBinding(UpdateType t, Parameter *_p, int _param_id, int _cv_id = -1)
    {
        this->updateType = t;
        this->p = _p;
        this->cv_id = _cv_id;
        this->param_id = _param_id;
        this->ts_id = -1;
        this->deact_id = -1;
        this->ext_id = -1;
        valCache.reset("value");
        nameCache.reset("name");
        forceRefresh = true;
    }

    SurgeRackParamBinding(Parameter *_p, int _param_id, int _cv_id = -1)
    {
        this->updateType = FLOAT;
        this->p = _p;
        this->cv_id = _cv_id;
        this->param_id = _param_id;
        this->ts_id = -1;
        this->deact_id = -1;
        this->ext_id = -1;
        valCache.reset("value");
        nameCache.reset("name");
        forceRefresh = true;
    }

    ~SurgeRackParamBinding() {}

    void setTemposync(int i, bool label)
    {
        ts_id = i;
        tsbpmLabel = label;
    }

    void setExtend(int i) { this->ext_id = i; }

    void setActivate(int i)
    {
        this->deact_id = i;
        deactivationMode = PARAM;
    }

    void setDeactivationAlways(bool b)
    {
        deactivationMode = CONSTANT;
        deactivationAlways = b;
    }

    void update(const ParamCache &pc, SurgeModuleCommon *m) { update(pc, 0, m); }

    void update(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m)
    {
        switch (updateType)
        {
        case FLOAT:
            updateFloat(pc, polyChannel, m);
            break;
        case BOOL:
            updateBool(pc, polyChannel, m, false);
            break;
        case BOOL_NOT:
            updateBool(pc, polyChannel, m, true);
            break;
        case INT:
            updateInt(pc, polyChannel, m);
            break;
        }
        forceRefresh = false;

        if (p->can_deactivate())
        {
            switch (deactivationMode)
            {
            case CONSTANT:
                p->deactivated = deactivationAlways;
                break;
            case PARAM:
                // Remeber we bind activation here
                if (m->getParam(deact_id) > 0.5)
                {
                    p->deactivated = false;
                }
                else
                {
                    p->deactivated = true;
                }
                break;
            }
        }

        if (p->can_extend_range())
        {
            if (m->getParam(ext_id) > 0.5)
            {
                p->extend_range = true;
            }
            else
            {
                p->extend_range = false;
            }
        }
    }

    void updateFloat(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m);
    void updateInt(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m);
    void updateBool(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m, bool n);
};

struct ParamValueStateSaver
{
    std::map<int, std::map<int, float>> valueStates;
    ParamValueStateSaver() {}

    void storeParams(int index, int paramStart, int paramEndInclusive, SurgeModuleCommon *m)
    {
        std::map<int, float> cache;
        for (auto i = paramStart; i <= paramEndInclusive; ++i)
        {
            cache[i] = m->getParam(i);
        }
        valueStates[index] = cache;
    }

    bool hasStoredAtIndex(int index) { return valueStates.find(index) != valueStates.end(); }

    void applyFromIndex(int index, SurgeModuleCommon *m)
    {
        if (!hasStoredAtIndex(index))
            return;

        auto cache = valueStates[index];
        for (auto pair : cache)
        {
            m->setParam(pair.first, pair.second);
        }
    }
};

struct SurgeRackParamQuantity : public rack::engine::ParamQuantity
{
    int ts_companion = -2;

    virtual void setDisplayValueString(std::string s) override;
    virtual std::string getLabel() override;
    virtual std::string getDisplayValueString() override;
};

template <typename T> struct SurgeRackOSCParamQuantity : public rack::engine::ParamQuantity
{
    virtual void setDisplayValueString(std::string s) override
    {
        T *mc = dynamic_cast<T *>(module);
        if (mc)
        {
            int opid = paramId - T::OSC_CTRL_PARAM_0;
            auto *p = &(mc->oscstorage->p[opid]);

            std::string emsg;
            p->set_value_from_string(s, emsg);
            setValue(p->get_value_f01());
            return;
        }
        ParamQuantity::setDisplayValueString(s);
    }

    virtual std::string getLabel() override
    {
        T *mc = dynamic_cast<T *>(module);
        if (mc)
        {
            int opid = paramId - T::OSC_CTRL_PARAM_0;
            auto *p = &(mc->oscstorage->p[opid]);
            return p->get_name();
        }
        return ParamQuantity::getLabel();
    }
    virtual std::string getDisplayValueString() override
    {
        T *mc = dynamic_cast<T *>(module);
        if (mc)
        {
            int opid = paramId - T::OSC_CTRL_PARAM_0;
            auto *p = &(mc->oscstorage->p[opid]);
            char txt[256];
            p->get_display(txt);
            return txt;
        }
        return ParamQuantity::getDisplayValueString();
    }
};

// This comes from surge unitconversion.h which is not used anywhere; but also
// which doesn't compile
inline char *get_notename(char *s, int i_value)
{
    int octave = (i_value / 12) - 2;
    char notenames[12][3] = {"C ", "C#", "D ", "D#", "E ", "F ",
                             "F#", "G ", "G#", "A ", "A#", "B "};
    sprintf(s, "%s%i", notenames[i_value % 12], octave);
    return s;
}
