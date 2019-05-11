/*
** A common base class for all of our Modules, introducing core functions to
** make sure surge is correctly configured in your module
*/

#pragma once
#include "Surge.hpp"
#include "SurgeStorage.h"
#include "rack.hpp"

#include <map>
#include <vector>

struct SurgeModuleCommon : virtual public rack::Module {
#if RACK_V1
    SurgeModuleCommon() : rack::Module() {  }
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

    std::string getVersion() {
        char version[1024];
        snprintf(version, 1023, "%s: %s.%s.%s",
#if WINDOWS
                 "win",
#endif
#if MAC
                 "macos",
#endif
#if LINUX
                 "linux",
#endif
                 TOSTRING(SURGE_RACK_BASE_VERSION),
                 TOSTRING(SURGE_RACK_PLUG_VERSION),
                 TOSTRING(SURGE_RACK_SURGE_VERSION));
        return std::string(version);
    }

    void showVersion() {
        INFO( "[SurgeRack] Instance: Module=%s Version=%s", getName().c_str(), getVersion().c_str() );
    }

    virtual std::string getName() = 0;
    
    virtual void onSampleRateChange() override {
#if RACK_V1
        float sr = rack::APP->engine->getSampleRate();
#else
        float sr = rack::engineGetSampleRate();
#endif
        INFO("[SurgeRack] Setting SampleRate to %lf", sr);
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

        showVersion();
        INFO("[SurgeRack] SurgeStorage::dataPath = %s", dataPath.c_str());

        // TODO: Have a mode where these paths come from res/
        storage.reset(new SurgeStorage(dataPath));
        onSampleRateChange();
    }

    inline float getParam(int id) {
#if RACK_V1
        return this->params[id].getValue();
#else
        return this->params[id].value;
#endif
    }

    inline void setParam(int id, float v) {
#if RACK_V1
        this->params[id].setValue(v);
#else
        this->params[id].value = v;
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

    std::string comment = "No Comment";
    virtual json_t *makeCommonDataJson() {
        json_t *rootJ = json_object();
        json_object_set_new( rootJ, "comment", json_string( comment.c_str() ) );
        json_object_set_new( rootJ, "buildVersion", json_string( getVersion().c_str() ) );
        return rootJ;
    }

    virtual void readCommonDataJson(json_t *rootJ) {
        json_t *com = json_object_get(rootJ, "comment" );
        const char* comchar = json_string_value(com);
        comment = comchar;
    }
    
#if RACK_V1
    virtual json_t *dataToJson() override {
        json_t *rootJ = makeCommonDataJson();
        return rootJ;
    }
    virtual void dataFromJson(json_t *rootJ) override {
        readCommonDataJson(rootJ);
    }
#else
    virtual json_t *toJson() override {
        json_t *rootJ = makeCommonDataJson();
        return rootJ;
    }

    virtual void fromJson( json_t *rootJ ) override {
        readCommonDataJson(rootJ);
    }
#endif    

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

    float get(int i) const { return cache[i]; }

    bool changed(int i, SurgeModuleCommon *m) const { return cache[i] != m->getParam(i); }
    bool changedInt(int i, SurgeModuleCommon *m) const { return (int)cache[i] != (int)m->getParam(i); }
};


/*
** Bind a surge parameter to a param/cv_id combo. If you only have a knob (no cv) set the cv_id to -1
*/
struct RackSurgeParamBinding {
    Parameter *p;
    int param_id, cv_id;

    StringCache valCache;
    StringCache nameCache;

    bool forceRefresh = false;

    
    RackSurgeParamBinding(Parameter *_p, int _param_id, int _cv_id) {
        this->p = _p;
        this->cv_id = _cv_id;
        this->param_id = _param_id;
        valCache.reset( "value" );
        nameCache.reset( "name" );
        forceRefresh = true;
    }

    ~RackSurgeParamBinding() {
    }

    void update(const ParamCache &pc, SurgeModuleCommon *m) {
        bool paramChanged = false;
        if(pc.changed(param_id,m) || forceRefresh)
        {
            char txt[1024];
            p->set_value_f01(m->getParam(param_id));
            p->get_display(txt, false, 0);
            valCache.reset(txt);
            paramChanged = true;
        }
        if(forceRefresh)
        {
            nameCache.reset(p->get_name());
        }

        if( paramChanged || forceRefresh || m->inputConnected(cv_id) )
            p->set_value_f01(m->getParam(param_id) + m->getInput(cv_id) / 10.0);
    }
};

struct ParamValueStateSaver {
    std::map<int, std::map<int, float>> valueStates;
    ParamValueStateSaver() { }

    void storeParams(int index, int paramStart, int paramEndInclusive, SurgeModuleCommon *m) {
        std::map<int,float> cache;
        for( auto i=paramStart; i<=paramEndInclusive; ++i )
        {
            cache[i] = m->getParam(i);
        }
        valueStates[index] = cache;
    }

    bool hasStoredAtIndex(int index) {
        return valueStates.find(index) != valueStates.end();
    }

    void applyFromIndex(int index, SurgeModuleCommon *m) {
        if( !hasStoredAtIndex(index) ) return;
        
        auto cache = valueStates[index];
        for(auto pair : cache )
        {
            m->setParam(pair.first, pair.second );
        }
    }
};

// This comes from surge unitconversion.h which is not used anywhere; but also which doesn't compile
inline char* get_notename(char* s, int i_value)
{
   int octave = (i_value / 12) - 2;
   char notenames[12][3] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};
   sprintf(s, "%s%i", notenames[i_value % 12], octave);
   return s;
}
