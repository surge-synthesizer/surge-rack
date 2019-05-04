#pragma once
#include "Surge.hpp"
#include "rack.hpp"
#include "SurgeStorage.h"

struct SurgeModuleCommon : virtual public rack::Module
{
#if RACK_V1    
    SurgeModuleCommon() : rack::Module() {
    }
#else
    SurgeModuleCommon(int NUM_P, int NUM_I, int NUM_O, int NUM_L) : rack::Module(NUM_P,NUM_I,NUM_O,NUM_L) {
        
        if( this->params.size() == 0)
        {
            // FIXME - for some reason the base class constructor isn't called reliably through the templates in V6
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
        INFO( "Setting SampleRate to %lf", sr );
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
        dataPath = rack::asset::plugin( pluginInstance, "surge-data/" );
#else
        dataPath = "";
#endif

        INFO( "setupSurgeCommon| SurgeStorage::dataPath = %s", dataPath.c_str() );
        
        // TODO: Have a mode where these paths come from res/
        storage.reset(new SurgeStorage(dataPath));
        onSampleRateChange();
        INFO( "setupSurgeCommon| Completed common setion" );
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


    std::unique_ptr<SurgeStorage> storage;
};

struct StringCache
{
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
        
struct ParamCache
{
    std::vector<float> cache;
    int np;
    ParamCache() {
        np = 0;
        cache.resize(np);
    }

    void resize(int n) {
        np = n;
        cache.resize(n);
    }
    
    void update(rack::Module *m) {
        for( auto i=0; i<np; ++i ) {
#if RACK_V1
            cache[i] = m->params[i].getValue();
#else
            cache[i] = m->params[i].value;
#endif
        }
    }
    float get(int i) {
        return cache[ i ];
    }
};
