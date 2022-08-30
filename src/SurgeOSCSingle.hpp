#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/Oscillator.h"
#include "rack.hpp"
#include <cstring>
#include <sst/filters/HalfRateFilter.h>

template <int type> struct SingleConfig
{
    static constexpr bool supportsUnison() { return false; }
};

template<int oscType> struct KnobConfiguration
{
    typedef std::vector<std::pair<int, std::string>> knobs_t;
    static knobs_t getKnobs() { return {}; }
};

template <int oscType> struct SurgeOSCSingle : virtual public SurgeModuleCommon
{
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        PITCH_0,

        OSC_CTRL_PARAM_0,

        OSC_MOD_PARAM_0 = OSC_CTRL_PARAM_0 + n_osc_params,

        NUM_PARAMS = OSC_MOD_PARAM_0 + (n_osc_params+1) * n_mod_inputs
    };
    enum InputIds
    {
        PITCH_CV,

        RETRIGGER,

        OSC_MOD_INPUT,

        NUM_INPUTS = OSC_MOD_INPUT + n_mod_inputs
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

    SurgeOSCSingle() : SurgeModuleCommon()
    {
        surge_osc.fill(nullptr);
        lastUnison.fill(-1);

        setupSurgeCommon(NUM_PARAMS);

        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        oscstorage_nomod = &(storage->getPatch().scene[0].osc[1]);
        oscstorage->type.val.i = oscType;
        oscstorage_nomod->type.val.i = oscType;
        setupStorageRanges(&(oscstorage->type), &(oscstorage->retrigger));

        auto config_osc = spawn_osc(oscType, storage.get(), oscstorage,
                                    storage->getPatch().scenedata[0], oscdisplaybuffer);
        config_osc->init(72.0);
        config_osc->init_ctrltypes();
        config_osc->init_default_values();
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < n_osc_params; ++i)
        {
            paramNames[i] = oscstorage->p[i].get_name();
        }

        configParam(PITCH_0, 1, 127, 60, "Pitch in Midi Note");

        for (int i = 0; i < n_osc_params + 1; ++i)
        {
            configParam<SurgeRackOSCParamQuantity<SurgeOSCSingle>>(
                OSC_CTRL_PARAM_0 + i, 0, 1, oscstorage->p[i].get_value_f01());
        }

        for (int i=OSC_MOD_PARAM_0; i<OSC_MOD_PARAM_0 + (n_osc_params+1)*n_mod_inputs; ++i)
        {
            configParam(i, -1, 1, 0);
        }

        pc.update(this);

        config_osc->~Oscillator();

        for (int i = 0; i < MAX_POLY; ++i)
        {
            halfbandOUT.emplace_back(6, true);
            halfbandOUT[i].reset();
        }
    }

    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - PITCH_0;
        return OSC_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    ~SurgeOSCSingle()
    {
        for (int i = 0; i < MAX_POLY; ++i)
        {
            if (surge_osc[i])
                surge_osc[i]->~Oscillator();
            surge_osc[i] = nullptr;
        }
    }

    virtual std::string getName() override
    {
        return std::string("OSCSingle") + std::to_string(oscType);
    }

    StringCache paramNameCache[n_osc_params];

    int processPosition = BLOCK_SIZE + 1;

    virtual void moduleSpecificSampleRateChange() override { forceRespawnDueToSampleRate = true; }

    std::array<int, MAX_POLY> lastUnison{-1};
    int lastNChan{-1};
    bool forceRespawnDueToSampleRate = false;
    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = std::max(1, inputs[PITCH_CV].getChannels());
        outputs[OUTPUT_L].setChannels(nChan);
        outputs[OUTPUT_R].setChannels(nChan);
        std::array<bool, MAX_POLY> respawned;
        respawned.fill(false);

        if (nChan != lastNChan || forceRespawnDueToSampleRate)
        {
            lastNChan = nChan;
            // Set up unmodulated values
            for (int i = 0; i < n_osc_params; ++i)
            {
                oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                oscstorage_nomod->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
            }

            copyScenedataSubset(0, storage_id_start, storage_id_end);

            for (int c = 0; c < nChan; ++c)
            {
                float pitch0 = getParam(PITCH_0) + inputs[PITCH_CV].getVoltage(c) * 12;
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
                respawned[c] = true;
            }
            forceRespawnDueToSampleRate = false;
            processPosition = BLOCK_SIZE + 1;
        }

        if (processPosition >= BLOCK_SIZE)
        {
            // As @Vortico says "think like a hardware engineer; only snap
            // values when you need them".
            processPosition = 0;

            float modMatrix[n_osc_params+1][n_mod_inputs];

            for (int i=0; i<n_osc_params+1; ++i)
            {
                for (int m=0; m<n_mod_inputs; ++m)
                {
                    modMatrix[i][m] = 0.f;
                }
            }
            const auto &knobConfig = KnobConfiguration<oscType>::getKnobs();
            for (const auto &[pid, label] : knobConfig)
            {
                int id = pid - PITCH_0;
                for (int m=0; m<n_mod_inputs; ++m)
                {
                    int modid = modulatorIndexFor(pid, m);
                    modMatrix[id][m] = getParam(modid);
                }
            }

            for (int m=0; m<n_mod_inputs; ++m)
                modMatrix[0][m] *= 12; // volts per octave

            for (int c = 0; c < nChan; ++c)
            {
                bool needsReInit{false};

                if (inputConnected(RETRIGGER) &&
                    reTrigger[c].process(inputs[RETRIGGER].getVoltage(c)))
                {
                    needsReInit = true;
                }

                float modValue[n_osc_params + 1];
                float pitch0 = getParam(PITCH_0) + inputs[PITCH_CV].getVoltage(c) * 12.0;

                modValue[0] = pitch0;
                for (int i = 0; i < n_osc_params; ++i)
                {
                    modValue[i+1] = getParam(OSC_CTRL_PARAM_0 + i);
                }

                for (int i=0; i<n_osc_params+1; ++i)
                {
                    for (int m=0; m<n_mod_inputs; ++m)
                    {
                        modValue[i] += modMatrix[i][m] * inputs[OSC_MOD_INPUT + m].getVoltage(c) * RACK_TO_SURGE_CV_MUL;
                    }
                }


                for (int i = 0; i < n_osc_params; ++i)
                {
                    oscstorage_nomod->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
                }

                if (outputConnected(OUTPUT_L) || outputConnected(OUTPUT_R))
                {
                    for (int i = 0; i < n_osc_params; ++i)
                    {
                        oscstorage->p[i].set_value_f01(modValue[i+1]);
                    }
                    if (SingleConfig<oscType>::supportsUnison())
                    {
                        if (oscstorage->p[n_osc_params - 1].val.i != lastUnison[c])
                        {
                            lastUnison[c] = oscstorage->p[n_osc_params - 1].val.i;
                            needsReInit = true;
                        }
                    }

                    copyScenedataSubset(0, storage_id_start, storage_id_end);

                    if (needsReInit)
                    {
                        surge_osc[c]->init(modValue[0]);
                    }
                    surge_osc[c]->process_block(modValue[0], 0, true);
                    copy_block(surge_osc[c]->output, osc_downsample[0][c], BLOCK_SIZE_OS_QUAD);
                    copy_block(surge_osc[c]->outputR, osc_downsample[1][c], BLOCK_SIZE_OS_QUAD);
                    halfbandOUT[c].process_block_D2(osc_downsample[0][c], osc_downsample[1][c]);
                }
            }
            pc.update(this);
        }

        for (int c = 0; c < nChan; ++c)
        {
            if (outputConnected(OUTPUT_L) && !outputConnected(OUTPUT_R))
            {
                // Special mono mode
                float output = (osc_downsample[0][c][processPosition] +
                                osc_downsample[1][c][processPosition]) *
                               0.5 * SURGE_TO_RACK_OSC_MUL;
                outputs[OUTPUT_L].setVoltage(output, c);
            }
            else
            {
                if (outputConnected(OUTPUT_L))
                    outputs[OUTPUT_L].setVoltage(osc_downsample[0][c][processPosition] *
                                                     SURGE_TO_RACK_OSC_MUL,
                                                 c);

                if (outputConnected(OUTPUT_R))
                    outputs[OUTPUT_R].setVoltage(osc_downsample[1][c][processPosition] *
                                                     SURGE_TO_RACK_OSC_MUL,
                                                 c);
            }
        }

        processPosition++;
    }

    // With surge-xt the oscillator memory is owned by the synth after spawn
    std::array<Oscillator *, MAX_POLY> surge_osc;
    unsigned char oscbuffer alignas(16)[MAX_POLY][oscillator_buffer_size];
    unsigned char oscdisplaybuffer alignas(16)[oscillator_buffer_size];

    OscillatorStorage *oscstorage, *oscstorage_nomod;
    float osc_downsample alignas(16)[2][MAX_POLY][BLOCK_SIZE_OS];
    std::vector<sst::filters::HalfRate::HalfRateFilter> halfbandOUT;

    rack::dsp::SchmittTrigger reTrigger[MAX_POLY];
};
