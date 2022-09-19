#pragma once
#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "dsp/Oscillator.h"
#include "rack.hpp"
#include <cstring>
#include <sst/filters/HalfRateFilter.h>

namespace sst::surgext_rack::vco
{
template <int oscType> struct VCO;

template <int oscType> struct VCOConfig
{
    static constexpr bool supportsUnison() { return false; }
    static constexpr bool requiresWavetables() { return false; }
    static constexpr bool supportsAudioIn() { return false; }

    struct KnobDef
    {
        enum Type
        {
            PARAM,
            INPUT,
            BLANK
        } type{PARAM};
        std::string name;
        int colspan{1};
        int id{-1};

        KnobDef(int kid, const std::string &nm) : type(PARAM), id(kid), name(nm) {}
        KnobDef(Type t, int kid, const std::string &nm) : type(t), id(kid), name(nm) {}
        KnobDef(Type t, int kid, const std::string &nm, int colspan) : type(t), id(kid), name(nm), colspan(colspan) {}
        KnobDef(Type t) : type(t) {}
    };
    typedef std::vector<KnobDef> knobs_t;
    static knobs_t getKnobs() { return {}; }

    typedef std::vector<std::pair<int,int>> lightOnTo_t;
    static lightOnTo_t getLightsOnKnobsTo() { return {}; }
    enum LightType {
        POWER,
        ABSOLUTE,
        EXTENDED
    };
    static LightType getLightTypeFor(int idx) { return POWER; }

    static int getMenuLightID() { return -1; }
    static std::string getMenuLightString() { return ""; }

    static int rightMenuParamId() { return -1; }
    static std::function<std::string(const std::string &)> rightMenuTransformFunction()
    {
        return [](auto &s) { return s; };
    }

    static void oscillatorSpecificSetup(VCO<oscType> *) {}
    static void processLightParameters(VCO<oscType> *) {}
    static void configureArbitrarySwitches(VCO<oscType> *);
    static std::string retriggerLabel() { return "RESET";}

    /*
     * Wavetable Updates from the UI to the Processing Thread.
     * This queue can be pretty small. You can't change more quickly than we
     * load em really
     */
    static constexpr int wavetableQueueSize() { return requiresWavetables() ? 32 : 1; }

};

template <int oscType> struct VCO : public modules::XTModule
{
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        PITCH_0,

        OSC_CTRL_PARAM_0,

        OSC_MOD_PARAM_0 = OSC_CTRL_PARAM_0 + n_osc_params,

        OCTAVE_SHIFT = OSC_MOD_PARAM_0 + (n_osc_params + 1) * n_mod_inputs,

        ARBITRARY_SWITCH_0,

        NUM_PARAMS = ARBITRARY_SWITCH_0 + n_arbitrary_switches
    };
    enum InputIds
    {
        PITCH_CV,

        RETRIGGER,

        OSC_MOD_INPUT,

        AUDIO_INPUT = OSC_MOD_INPUT + n_mod_inputs,
        NUM_INPUTS
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

    static constexpr const char *name = osc_type_names[oscType];
    std::array<std::string, n_osc_params> paramNames;
    modules::ModulationAssistant<VCO<oscType>, n_osc_params + 1,
        PITCH_CV, n_mod_inputs, OSC_MOD_INPUT> modAssist;

    VCO() : XTModule(), halfbandIN(6, true)
    {
        surge_osc.fill(nullptr);
        lastUnison.fill(-1);

        memset(audioInBuffer, 0, BLOCK_SIZE_OS * sizeof(float));
        setupSurgeCommon(NUM_PARAMS, VCOConfig<oscType>::requiresWavetables());

        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        oscstorage_display = &(storage->getPatch().scene[0].osc[1]);
        oscstorage->type.val.i = oscType;
        oscstorage_display->type.val.i = oscType;
        setupStorageRanges(&(oscstorage->type), &(oscstorage->retrigger));

        if constexpr (VCOConfig<oscType>::requiresWavetables())
        {
            if (!oscstorage->wt.everBuilt)
            {
                oscstorage->wt.queue_id = 0;
                oscstorage_display->wt.queue_id = 0;
                storage->perform_queued_wtloads();
                wavetableIndex = oscstorage->wt.current_id;
            }
        }

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        auto config_osc = spawn_osc(oscType, storage.get(), oscstorage,
                                    storage->getPatch().scenedata[0], oscdisplaybuffer[0]);
        config_osc->init(72.0);
        config_osc->init_ctrltypes();
        config_osc->init_default_values();

        auto display_osc = spawn_osc(oscType, storage.get(), oscstorage_display,
                                            storage->getPatch().scenedata[0], oscdisplaybuffer[1]);
        display_osc->init(72.0, true);
        display_osc->init_ctrltypes();
        display_osc->init_default_values();

        VCOConfig<oscType>::oscillatorSpecificSetup(this);

        for (int i = 0; i < n_osc_params; ++i)
        {
            paramNames[i] = oscstorage->p[i].get_name();
        }

        configParam<modules::MidiNoteParamQuantity<0>>(PITCH_0, 1, 127, 60, "Pitch");
        auto os = configParam(OCTAVE_SHIFT, -3, 3, 0, "Octave Shift");
        os->snapEnabled = true;

        for (int i = 0; i < n_osc_params + 1; ++i)
        {
            configParam<modules::SurgeParameterParamQuantity>(
                OSC_CTRL_PARAM_0 + i, 0, 1, oscstorage->p[i].get_value_f01());
        }

        for (int i = OSC_MOD_PARAM_0; i < OSC_MOD_PARAM_0 + (n_osc_params + 1) * n_mod_inputs; ++i)
        {
            configParam(i, -1, 1, 0);
        }

        VCOConfig<oscType>::configureArbitrarySwitches(this);
        config_osc->~Oscillator();
        display_osc->~Oscillator();

        for (int i = 0; i < MAX_POLY; ++i)
        {
            halfbandOUT.emplace_back(6, true);
            halfbandOUT[i].reset();
        }

        halfbandIN.reset();

        configInput(PITCH_CV, "V/Oct");
        configInput(RETRIGGER, "Reset/Retrigger");
        for (int m=0; m < n_mod_inputs; ++m)
        {
            auto s = std::string( "Modulation Signal " ) + std::to_string(m+1);
            configInput(OSC_MOD_INPUT + m, s);
        }
        configOutput(OUTPUT_L, "Left (or Mono merged)");
        configOutput(OUTPUT_R, "Right");

        memset(modulationDisplayValues, 0, (n_osc_params + 1) * sizeof(float));
        modAssist.initialize(this);
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - PITCH_0;
        return OSC_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    ~VCO()
    {
        for (int i = 0; i < MAX_POLY; ++i)
        {
            if (surge_osc[i])
                surge_osc[i]->~Oscillator();
            surge_osc[i] = nullptr;
        }
    }

    inline int polyChannelCount()
    {
        return std::max({1, inputs[PITCH_CV].getChannels(), inputs[RETRIGGER].getChannels()});
    }

    std::string getName() override
    {
        return std::string("VCO<") + osc_type_names[oscType] + ">";
    }

    bool isBipolar(int paramId) override {
        if (paramId >= OSC_CTRL_PARAM_0 && paramId <= OSC_CTRL_PARAM_0 + n_osc_params)
        {
            return oscstorage->p[paramId-OSC_CTRL_PARAM_0].is_bipolar();
        }
        return false;
    }
    float modulationDisplayValues[n_osc_params + 1];
    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - PITCH_0;
        if (idx < 0 || idx >= n_osc_params + 1)
            return 0;
        return modulationDisplayValues[idx];
    }

    Parameter *surgeParameterForParamId(int paramId) override {
        if (paramId < OSC_CTRL_PARAM_0 || paramId >= OSC_CTRL_PARAM_0 + n_osc_params)
            return nullptr;

        return &oscstorage->p[paramId-OSC_CTRL_PARAM_0];
    }


    int processPosition = BLOCK_SIZE + 1;

    void moduleSpecificSampleRateChange() override { forceRespawnDueToExternality = true; }

    struct WavetableMessage
    {
        int index{-1};
        char filename[256];
    };
    rack::dsp::RingBuffer<WavetableMessage, VCOConfig<oscType>::wavetableQueueSize()> wavetableQueue;
    std::atomic<int> wavetableIndex{-1};
    std::atomic<uint32_t> wavetableLoads{0};
    std::atomic<bool> draw3DWavetable{VCOConfig<oscType>::requiresWavetables()};

    std::string getWavetableName()
    {
        int idx = wavetableIndex;
        if (idx >= 0)
            return storage->wt_list[idx].name;
        else
            return oscstorage->wavetable_display_name;
    }

    std::array<int, MAX_POLY> lastUnison{-1};
    int lastNChan{-1};
    bool forceRespawnDueToExternality = false;
    static constexpr int checkWaveTableEvery{512};
    int checkedWaveTable{checkWaveTableEvery};
    static constexpr int calcModMatrixEvery{256};
    int calcedModMatrix{calcModMatrixEvery};
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = polyChannelCount();
        outputs[OUTPUT_L].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);

        bool reInitEveryOSC{false};
        if constexpr (VCOConfig<oscType>::requiresWavetables())
        {
            if (checkedWaveTable >= checkWaveTableEvery)
            {
                checkedWaveTable = 0;

                bool read{false};
                WavetableMessage msg;
                while (!wavetableQueue.empty())
                {
                    msg = wavetableQueue.shift();
                    read = true;
                }

                if (read)
                {
                    if (msg.index >= 0)
                    {
                        // We really should do this off audio thread but for now
                        auto nid = std::clamp((int)msg.index, (int)0, (int)storage->wt_list.size());
                        oscstorage->wt.queue_id = nid;
                        oscstorage_display->wt.queue_id = nid;
                        storage->perform_queued_wtloads();

                        wavetableIndex = oscstorage->wt.current_id;
                    }
                    else
                    {
                        strncpy(oscstorage->wt.queue_filename, msg.filename, 256);
                        strncpy(oscstorage_display->wt.queue_filename, msg.filename, 256);
                        storage->perform_queued_wtloads();

                        wavetableIndex = -1;
                    }
                    wavetableLoads++;
                    reInitEveryOSC = true;
                }
            }
        }

        if (nChan != lastNChan || forceRespawnDueToExternality)
        {
            lastNChan = nChan;
            // Set up unmodulated values
            for (int i = 0; i < n_osc_params; ++i)
            {
                oscstorage->p[i].set_value_f01(params[OSC_CTRL_PARAM_0 + i].getValue());
                oscstorage_display->p[i].set_value_f01(params[OSC_CTRL_PARAM_0 + i].getValue());
            }

            copyScenedataSubset(0, storage_id_start, storage_id_end);

            for (int c = 0; c < nChan; ++c)
            {
                float pitch0 = params[PITCH_0].getValue() + (params[OCTAVE_SHIFT].getValue() + inputs[PITCH_CV].getVoltage(c)) * 12;
                if (!surge_osc[c])
                {
                    surge_osc[c] = spawn_osc(oscType, storage.get(), oscstorage,
                                             storage->getPatch().scenedata[0], oscbuffer[c]);

                    surge_osc[c]->init(pitch0);
                    surge_osc[c]->init_ctrltypes();
                }
                else
                {
                    surge_osc[c]->init(pitch0);
                }
            }
            forceRespawnDueToExternality = false;
            processPosition = BLOCK_SIZE + 1;
        }

        if (processPosition >= BLOCK_SIZE)
        {
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);
            // As @Vortico says "think like a hardware engineer; only snap
            // values when you need them".
            processPosition = 0;

            if constexpr (VCOConfig<oscType>::supportsAudioIn())
            {
                halfbandIN.process_block_U2(audioInBuffer, audioInBuffer, storage->audio_in[0], storage->audio_in[1], BLOCK_SIZE_OS);
                memset(audioInBuffer, 0, BLOCK_SIZE_OS * sizeof(float));
            }

            VCOConfig<oscType>::processLightParameters(this);

            for (int i = 0; i < n_osc_params; ++i)
            {
                // This is the non-modulated version
                // oscstorage_display->p[i].set_value_f01(params[OSC_CTRL_PARAM_0 + i].getValue());
                oscstorage_display->p[i].set_value_f01(modAssist.values[i+1][0]);
            }

            // This is super gross and inefficient. Think about all of this
            memcpy(modulationDisplayValues, modAssist.animValues, sizeof(modAssist.animValues));

            for (int c = 0; c < nChan; ++c)
            {
                bool needsReInit{reInitEveryOSC};

                if (inputs[RETRIGGER].isConnected() &&
                    reTrigger[c].process(inputs[RETRIGGER].getVoltage(c)))
                {
                    needsReInit = true;
                }

                if (outputs[OUTPUT_L].isConnected() || outputs[OUTPUT_R].isConnected())
                {
                    for (int i = 0; i < n_osc_params; ++i)
                    {
                        oscstorage->p[i].set_value_f01(modAssist.values[i+1][c]);
                    }
                    if constexpr (VCOConfig<oscType>::supportsUnison())
                    {
                        if (oscstorage->p[n_osc_params - 1].val.i != lastUnison[c])
                        {
                            lastUnison[c] = oscstorage->p[n_osc_params - 1].val.i;
                            needsReInit = true;
                        }
                    }

                    float pitch0 = modAssist.values[0][c] + (params[OCTAVE_SHIFT].getValue() + inputs[PITCH_CV].getVoltage(c)) * 12;

                    copyScenedataSubset(0, storage_id_start, storage_id_end);
                    if (needsReInit)
                    {
                        surge_osc[c]->init(pitch0);
                    }
                    surge_osc[c]->process_block(pitch0, 0, true);
                    copy_block(surge_osc[c]->output, osc_downsample[0][c], BLOCK_SIZE_OS_QUAD);
                    copy_block(surge_osc[c]->outputR, osc_downsample[1][c], BLOCK_SIZE_OS_QUAD);
                    halfbandOUT[c].process_block_D2(osc_downsample[0][c], osc_downsample[1][c],
                                                    BLOCK_SIZE_OS);
                }
            }
            // pc.update(this);
        }

        for (int c = 0; c < nChan; ++c)
        {
            if (outputs[OUTPUT_L].isConnected() && !outputs[OUTPUT_R].isConnected())
            {
                // Special mono mode
                float output = (osc_downsample[0][c][processPosition] +
                                osc_downsample[1][c][processPosition]) *
                               0.5 * SURGE_TO_RACK_OSC_MUL;
                outputs[OUTPUT_L].setVoltage(output, c);
            }
            else
            {
                if (outputs[OUTPUT_L].isConnected())
                    outputs[OUTPUT_L].setVoltage(
                        osc_downsample[0][c][processPosition] * SURGE_TO_RACK_OSC_MUL, c);

                if (outputs[OUTPUT_R].isConnected())
                    outputs[OUTPUT_R].setVoltage(
                        osc_downsample[1][c][processPosition] * SURGE_TO_RACK_OSC_MUL, c);
            }
        }

        if constexpr (VCOConfig<oscType>::supportsAudioIn())
        {
            float val = 0;
            if (inputs[AUDIO_INPUT].isConnected())
            {
                val = inputs[AUDIO_INPUT].getVoltage() * RACK_TO_SURGE_OSC_MUL;
            }
            audioInBuffer[processPosition] = val;
        }

        processPosition++;
        checkedWaveTable++;
        calcedModMatrix++;
    }

    // With surge-xt the oscillator memory is owned by the synth after spawn
    std::array<Oscillator *, MAX_POLY> surge_osc;
    unsigned char oscbuffer alignas(16)[MAX_POLY][oscillator_buffer_size];
    unsigned char oscdisplaybuffer alignas(16)[2][oscillator_buffer_size];

    OscillatorStorage *oscstorage, *oscstorage_display;
    float osc_downsample alignas(16)[2][MAX_POLY][BLOCK_SIZE_OS];
    std::vector<sst::filters::HalfRate::HalfRateFilter> halfbandOUT;
    sst::filters::HalfRate::HalfRateFilter halfbandIN;
    float audioInBuffer[BLOCK_SIZE_OS];

    rack::dsp::SchmittTrigger reTrigger[MAX_POLY];

    json_t *makeModuleSpecificJson() override {
        auto vco = json_object();
        if (VCOConfig<oscType>::requiresWavetables())
        {
            auto *wtT = json_object();
            json_object_set(wtT, "draw3D",
                            json_boolean(draw3DWavetable));

            json_object_set(wtT, "display_name",
                            json_string(oscstorage->wavetable_display_name));

            auto &wt = oscstorage->wt;
            json_object_set(wtT, "n_tables", json_integer(wt.n_tables));
            json_object_set(wtT, "n_samples", json_integer(wt.size));
            json_object_set(wtT, "flags", json_integer(wt.flags));

            wt_header wth;
            memset(wth.tag, 0, 4);
            wth.n_samples = wt.size;
            wth.n_tables = wt.n_tables;
            wth.flags = wt.flags | wtf_int16;
            unsigned int wtsize =
                wth.n_samples * wt.n_tables * sizeof(uint16_t) +
                sizeof(wt_header);

            auto *data = new uint8_t[wtsize];
            auto *odata = data;
            memcpy(data, &wth, sizeof(wt_header));
            data += sizeof(wt_header);

            for (int j=0; j<wth.n_tables; ++j)
            {
                std::memcpy(data, &wt.TableI16WeakPointers[0][j][FIRoffsetI16],
                            wth.n_samples * sizeof(uint16_t));
                data += wth.n_samples * sizeof(uint16_t);
            }
            auto b64 = rack::string::toBase64(odata, wtsize);
            delete[] odata;
            json_object_set(wtT, "data", json_string(b64.c_str()));
            json_object_set(vco, "wavetable", wtT);
        }
        return vco;
    }
    void readModuleSpecificJson(json_t *modJ) override {
        if (VCOConfig<oscType>::requiresWavetables())
        {
            auto wtJ = json_object_get(modJ, "wavetable");
            if (!wtJ)
                return;
            auto dj = json_object_get(wtJ, "data");
            if (!dj)
                return;
            auto *sv = json_string_value(dj);
            if (!sv)
                return;

            auto d3 = json_object_get(wtJ, "draw3D");
            if (d3)
            {
                draw3DWavetable = json_boolean_value(d3);
            }

            auto dataV = rack::string::fromBase64(sv);
            auto *data = &dataV[0];

            wt_header wth;
            memcpy(&wth, data, sizeof(wt_header));
            data += sizeof(wt_header);
            storage->waveTableDataMutex.lock();
            oscstorage->wt.BuildWT(data, wth, false);
            oscstorage_display->wt.BuildWT(data, wth, false);
            storage->waveTableDataMutex.unlock();

            auto nm = json_object_get(wtJ, "display_name");
            if (nm)
            {
                oscstorage->wt.current_id = -1;
                strncpy(oscstorage->wavetable_display_name, json_string_value(nm), 256);

                oscstorage_display->wt.current_id = -1;
                strncpy(oscstorage_display->wavetable_display_name, json_string_value(nm), 256);

                wavetableIndex = -1;
            }
        }
    }
};

template<int oscType>
inline void VCOConfig<oscType>::configureArbitrarySwitches(VCO<oscType> *m)
{
    for (int i=0; i<VCO<oscType>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<oscType>::ARBITRARY_SWITCH_0 + i, 0, 1, 0);
    }
}
}