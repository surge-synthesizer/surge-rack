/*
** A common base class for all of our Modules, introducing core functions to
** make sure surge is correctly configured in your module
*/

#pragma once
#include "Surge.hpp"
#include "SurgeStorage.h"
#include "rack.hpp"

struct SurgeModuleCommon : virtual public rack::Module {
#if RACK_V1
    SurgeModuleCommon() : rack::Module() {}
#else
    SurgeModuleCommon(int NUM_P, int NUM_I, int NUM_O, int NUM_L)
        : rack::Module(NUM_P, NUM_I, NUM_O, NUM_L) {

        if (this->params.size() == 0) {
            // FIXME - for some reason the base class constructor isn't called
            // reliably through the templates in V6
            this->params.resize(NUM_P);
            this->inputs.resize(NUM_I);
            this->outputs.resize(NUM_O);
            this->lights.resize(NUM_L);
        }
    }
#endif

    virtual void onSampleRateChange() override {
#if RACK_V1
        float sr = rack::APP->engine->getSampleRate();
#else
        float sr = rack::engineGetSampleRate();
#endif
        INFO("Setting SampleRate to %lf", sr);
        samplerate = sr;
        dsamplerate = sr;
        samplerate_inv = 1.0 / sr;
        dsamplerate_inv = 1.0 / sr;
        dsamplerate_os = dsamplerate * OSC_OVERSAMPLING;
        dsamplerate_os_inv = 1.0 / dsamplerate_os;
        storage->init_tables();
    }

    void setupSurgeCommon() {
        std::string dataPath;
#if RACK_V1
        dataPath = rack::asset::plugin(pluginInstance, "surge-data/");
#else
        dataPath = "";
#endif

        INFO("setupSurgeCommon| SurgeStorage::dataPath = %s", dataPath.c_str());

        // TODO: Have a mode where these paths come from res/
        storage.reset(new SurgeStorage(dataPath));
        onSampleRateChange();
        INFO("setupSurgeCommon| Completed common setion");
    }

    inline float getParam(int id) {
#if RACK_V1
        return this->params[id].getValue();
#else
        return this->params[id].value;
#endif
    }

    inline float getInput(int id) {
#if RACK_V1
        return this->inputs[id].getVoltage();
#else
        return this->inputs[id].value;
#endif
    }

    inline void setOutput(int id, float v) {
#if RACK_V1
        this->outputs[id].setVoltage(v);
#else
        this->outputs[id].value = v;
#endif
    }

    inline void setLight(int id, float val) {
#if RACK_V1
        this->lights[id].setBrightness(val);
#else
        this->lights[id].value = val;
#endif
    }

    inline bool inputConnected(int id) {
#if RACK_V1
        return this->inputs[id].isConnected();
#else
        return this->inputs[id].active;
#endif        
    }

    inline bool outputConnected(int id) {
#if RACK_V1
        return this->outputs[id].isConnected();
#else
        return this->outputs[id].active;
#endif        
    }

    void copyScenedataSubset(int scene, int start, int end) {
        int s = storage->getPatch().scene_start[scene];
        for(int i=start; i<end; ++i )
        {
            storage->getPatch().scenedata[scene][i-s].i =
                storage->getPatch().param_ptr[i]->val.i;
        }
    }

    void copyGlobaldataSubset(int start, int end) {
        for(int i=start; i<end; ++i )
        {
            storage->getPatch().globaldata[i].i =
                storage->getPatch().param_ptr[i]->val.i;
        }
    }

    void setupStorageRanges(Parameter *start, Parameter *endIncluding) {
        int min_id = 100000, max_id = -1;
        Parameter *oap = start;
        while( oap <= endIncluding )
        {
            if( oap->id > max_id ) max_id = oap->id;
            if( oap->id < min_id ) min_id = oap->id;
            oap++;
        }

        storage_id_start = min_id;
        storage_id_end = max_id + 1;        
    }
    
    std::unique_ptr<SurgeStorage> storage;
    int storage_id_start, storage_id_end;
};

struct StringCache {
    std::string value;
    bool dirty;
    std::function<std::string()> getValue;
    std::function<bool()> getDirty;

    StringCache() {
        value = "";
        dirty = true;
        getValue = [this]() { return this->value; };
        getDirty = [this]() {
            auto res = dirty;
            dirty = false;
            return res;
        };
    }

    void reset(std::string s) {
        value = s;
        dirty = true;
    }
};

struct ParamCache {
    std::vector<float> cache;
    int np;
    ParamCache() {
        np = 0;
        resize(np);
    }

    void resize(int n) {
        np = n;
        cache.resize(n);
        for (int i = 0; i < n; ++i)
            cache[i] = /* float min */ -1328142.0;
    }

    void update(rack::Module *m) {
        for (auto i = 0; i < np; ++i) {
#if RACK_V1
            cache[i] = m->params[i].getValue();
#else
            cache[i] = m->params[i].value;
#endif
        }
    }

    float get(int i) { return cache[i]; }

    bool changed(int i, SurgeModuleCommon *m) { return cache[i] != m->getParam(i); }
    bool changedInt(int i, SurgeModuleCommon *m) { return (int)cache[i] != (int)m->getParam(i); }
};
