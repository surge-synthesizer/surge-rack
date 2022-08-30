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

template <int oscType> struct SurgeOSCSingle : virtual public SurgeModuleCommon
{
    static constexpr int n_mod_inputs{4};

    enum ParamIds
    {
        OUTPUT_GAIN,

        PITCH_0,

        OSC_CTRL_PARAM_0,

        OSC_MOD_PARAM_0 = OSC_CTRL_PARAM_0 + n_osc_params,

        NUM_PARAMS = OSC_MOD_PARAM_0 + n_osc_params * n_mod_inputs
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

    ParamValueStateSaver knobSaver;

    static constexpr const char *name = osc_type_names[oscType];
    std::array<std::string, n_osc_params> paramNames;

    SurgeOSCSingle() : SurgeModuleCommon()
    {
        surge_osc.fill(nullptr);
        lastUnison.fill(-1);

        setupSurgeCommon(NUM_PARAMS);

        oscstorage = &(storage->getPatch().scene[0].osc[0]);
        oscstorage->type.val.i = oscType;
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

        configParam(OUTPUT_GAIN, 0, 1, 1, "Output Gain");
        configParam(PITCH_0, 1, 127, 60, "Pitch in Midi Note");

        for (int i = 0; i < n_osc_params; ++i)
        {
            configParam<SurgeRackOSCParamQuantity<SurgeOSCSingle>>(
                OSC_CTRL_PARAM_0 + i, 0, 1, oscstorage->p[i].get_value_f01());
            for (int m=0; m < n_mod_inputs; ++m)
            {
                configParam(OSC_MOD_PARAM_0 + i + m *n_mod_inputs, -1, 1, 0);
            }
        }

        pc.update(this);

        config_osc->~Oscillator();

        for (int i = 0; i < MAX_POLY; ++i)
        {
            halfbandOUT.emplace_back(6, true);
            halfbandOUT[i].reset();
        }
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

            for (int c = 0; c < nChan; ++c)
            {
                bool needsReInit{false};

                if (inputConnected(RETRIGGER) &&
                    reTrigger[c].process(inputs[RETRIGGER].getVoltage(c)))
                {
                    needsReInit = true;
                }

                if (outputConnected(OUTPUT_L) || outputConnected(OUTPUT_R))
                {
                    for (int i = 0; i < n_osc_params; ++i)
                    {
                        oscstorage->p[i].set_value_f01(getParam(OSC_CTRL_PARAM_0 + i));
/*+
                                                       inputs[OSC_CTRL_CV_0 + i].getPolyVoltage(c) *
                                                           RACK_TO_SURGE_CV_MUL); */
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
                    float pitch0 = getParam(PITCH_0) + inputs[PITCH_CV].getVoltage(c) * 12.0;

                    if (needsReInit)
                    {
                        surge_osc[c]->init(pitch0);
                    }
                    surge_osc[c]->process_block(pitch0, 0, true);
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
                               0.5 * SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN);
                outputs[OUTPUT_L].setVoltage(output, c);
            }
            else
            {
                if (outputConnected(OUTPUT_L))
                    outputs[OUTPUT_L].setVoltage(osc_downsample[0][c][processPosition] *
                                                     SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN),
                                                 c);

                if (outputConnected(OUTPUT_R))
                    outputs[OUTPUT_R].setVoltage(osc_downsample[1][c][processPosition] *
                                                     SURGE_TO_RACK_OSC_MUL * getParam(OUTPUT_GAIN),
                                                 c);
            }
        }

        processPosition++;
    }

    // With surge-xt the oscillator memory is owned by the synth after spawn
    std::array<Oscillator *, MAX_POLY> surge_osc;
    unsigned char oscbuffer alignas(16)[MAX_POLY][oscillator_buffer_size];
    unsigned char oscdisplaybuffer alignas(16)[oscillator_buffer_size];

    OscillatorStorage *oscstorage;
    float osc_downsample alignas(16)[2][MAX_POLY][BLOCK_SIZE_OS];
    std::vector<sst::filters::HalfRate::HalfRateFilter> halfbandOUT;

    rack::dsp::SchmittTrigger reTrigger[MAX_POLY];
};
