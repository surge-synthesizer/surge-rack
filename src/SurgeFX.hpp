#pragma once
#include "SurgeXT.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/Effect.h"
#include "rack.hpp"
#include <cstring>

#define NUM_FX_PARAMS 12

template <int effectNum> struct SurgeFXName
{
    static std::string getName() { return "GENERIC"; }
};

template <> struct SurgeFXName<fxt_delay>
{
    static std::string getName() { return "DELAY"; }
};
template <> struct SurgeFXName<fxt_eq>
{
    static std::string getName() { return "EQ"; }
};
template <> struct SurgeFXName<fxt_phaser>
{
    static std::string getName() { return "PHASER"; }
};
template <> struct SurgeFXName<fxt_rotaryspeaker>
{
    static std::string getName() { return "ROTARY"; }
};
template <> struct SurgeFXName<fxt_distortion>
{
    static std::string getName() { return "DISTORT"; }
};
template <> struct SurgeFXName<fxt_reverb>
{
    static std::string getName() { return "REVERB"; }
};
template <> struct SurgeFXName<fxt_reverb2>
{
    static std::string getName() { return "REVERB2"; }
};
template <> struct SurgeFXName<fxt_freqshift>
{
    static std::string getName() { return "FREQSHIFT"; }
};
template <> struct SurgeFXName<fxt_chorus4>
{
    static std::string getName() { return "CHORUS"; }
};
template <> struct SurgeFXName<fxt_conditioner>
{
    static std::string getName() { return "CONDITION"; }
};
template <> struct SurgeFXName<fxt_vocoder>
{
    static std::string getName() { return "VOC"; }
};
template <> struct SurgeFXName<fxt_flanger>
{
    static std::string getName() { return "FLANGER"; }
};
template <> struct SurgeFXName<fxt_ringmod>
{
    static std::string getName() { return "RINGMOD"; }
};

template <> struct SurgeFXName<fxt_resonator>
{
    static std::string getName() { return "RESONATOR"; }
};
template <> struct SurgeFXName<fxt_chow>
{
    static std::string getName() { return "CHOW"; }
};
template <> struct SurgeFXName<fxt_exciter>
{
    static std::string getName() { return "EXCITER"; }
};
template <> struct SurgeFXName<fxt_ensemble>
{
    static std::string getName() { return "ENSEMBLE"; }
};
template <> struct SurgeFXName<fxt_combulator>
{
    static std::string getName() { return "COMBULATOR"; }
};
template <> struct SurgeFXName<fxt_tape>
{
    static std::string getName() { return "TAPE"; }
};
template <> struct SurgeFXName<fxt_treemonster>
{
    static std::string getName() { return "TREEMONSTER"; }
};

template <int effectNum> struct SurgeFXTraits
{
    static bool constexpr hasModulatorSignal = false;
};

template <> struct SurgeFXTraits<fxt_vocoder>
{
    static bool constexpr hasModulatorSignal = true;
};

template <int effectNum> struct SurgeFX : virtual SurgeModuleCommon
{
    enum ParamIds
    {
        FX_PARAM_0 = 0,
        INPUT_GAIN = FX_PARAM_0 + NUM_FX_PARAMS,
        OUTPUT_GAIN,
        PARAM_TEMPOSYNC_0,
        MODULATOR_GAIN = PARAM_TEMPOSYNC_0 + NUM_FX_PARAMS,
        PARAM_EXTEND_0,
        PARAM_ACTIVATE_0 = PARAM_EXTEND_0 + NUM_FX_PARAMS,
        NUM_PARAMS = PARAM_ACTIVATE_0 + NUM_FX_PARAMS
    };
    enum InputIds
    {
        INPUT_L_OR_MONO,
        INPUT_R,

        FX_PARAM_INPUT_0,

        CLOCK_CV_INPUT = FX_PARAM_INPUT_0 + NUM_FX_PARAMS,

        MODULATOR_L_OR_MONO,
        MODULATOR_R,

        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_L_OR_MONO,
        OUTPUT_R,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        CAN_TEMPOSYNC_0,
        NUM_LIGHTS = CAN_TEMPOSYNC_0 + NUM_FX_PARAMS
    };

    StringCache groupCache[NUM_FX_PARAMS];

    SurgeFX() : SurgeModuleCommon()
    {
        setupSurge();

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = 0; i < 12; ++i)
        {
            configParam<SurgeRackParamQuantity>(FX_PARAM_0 + i, 0, 1, pb[i]->p->get_value_f01());
            configParam(PARAM_TEMPOSYNC_0 + i, 0, 1, 0, "TempoSync");
            configParam(PARAM_EXTEND_0 + i, 0, 1, 0, "Extend");
            configParam(PARAM_ACTIVATE_0 + i, 0, 1, 0, "Activate");
        }

        configParam(INPUT_GAIN, 0, 1, 1, "Input Gain");
        configParam(OUTPUT_GAIN, 0, 1, 1, "Output Gain");
        configParam(MODULATOR_GAIN, 0, 1, 1, "Modulator Gain");
    }

    virtual std::string getName() override { return SurgeFXName<effectNum>::getName(); }

    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS);

        fxstorage = &(storage->getPatch().fx[0]);
        fxstorage->type.val.i = effectNum;

        surge_effect.reset(spawn_effect(effectNum, storage.get(), &(storage->getPatch().fx[0]),
                                        storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();
        surge_effect->init_default_values();

        // Do default values
        reorderSurgeParams();

        for (auto i = 0; i < BLOCK_SIZE; ++i)
        {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
            modulatorL[i] = 0.0f;
            modulatorR[i] = 0.0f;
            processedL[i] = 0.0f;
            processedR[i] = 0.0f;
        }

        // You would think this is what you do yeah? But return_level is special
        // and is uninitalized so we have to do a mild hack. This will tell the
        // copyrange to ignore this one.
        fxstorage->return_level.id = -1;
        setupStorageRanges(&(fxstorage->type), &(fxstorage->p[n_fx_params - 1]));
    }

    void reorderSurgeParams()
    {
        if (surge_effect.get())
        {
            std::vector<std::pair<int, int>> orderTrack;
            for (auto i = 0; i < n_fx_params; ++i)
            {
                if (fxstorage->p[i].posy_offset)
                {
                    orderTrack.push_back(
                        std::pair<int, int>(i, i * 2 + fxstorage->p[i].posy_offset));
                }
                else
                {
                    orderTrack.push_back(std::pair<int, int>(i, 10000));
                }
            }
            std::sort(orderTrack.begin(), orderTrack.end(),
                      [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                          return a.second < b.second;
                      });
            int idx = FX_PARAM_0;
            for (auto a : orderTrack)
            {
                Parameter *p = &(fxstorage->p[a.first]);
                pb[idx] = std::shared_ptr<SurgeRackParamBinding>(
                    new SurgeRackParamBinding(p, idx, idx + FX_PARAM_INPUT_0 - FX_PARAM_0));
                pb[idx]->setTemposync(PARAM_TEMPOSYNC_0 + idx - FX_PARAM_0, true);
                pb[idx]->setActivate(PARAM_ACTIVATE_0 + idx - FX_PARAM_0);
                pb[idx]->setExtend(PARAM_EXTEND_0 + idx - FX_PARAM_0);
                idx++;
            }
        }

        // I hate having to use this API so much...
        for (auto i = 0; i < n_fx_params; ++i)
        {
            int fpos = pb[i]->p->posy + 10 * pb[i]->p->posy_offset;
            for (auto j = 0; j < n_fx_params; ++j)
            {
                if (surge_effect->group_label(j) &&
                    162 + 8 + 10 * surge_effect->group_label_ypos(j) <
                        fpos // constants for SurgeGUIEditor. Sigh.
                )
                {
                    groupCache[i].reset(surge_effect->group_label(j));
                }
            }
        }
    }

    bool canTempoSync(int i) { return pb[i]->p->can_temposync(); }

    float bufferL alignas(16)[BLOCK_SIZE], bufferR alignas(16)[BLOCK_SIZE];
    float modulatorL alignas(16)[BLOCK_SIZE], modulatorR alignas(16)[BLOCK_SIZE];
    float processedL alignas(16)[BLOCK_SIZE], processedR alignas(16)[BLOCK_SIZE];
    int bufferPos = BLOCK_SIZE - 1;

    virtual void moduleSpecificSampleRateChange() override
    {
        surge_effect.reset(spawn_effect(effectNum, storage.get(), &(storage->getPatch().fx[0]),
                                        storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();
        surge_effect->init_default_values();
        for (auto binding : pb)
            if (binding)
                binding->forceRefresh = true;
    }

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);

        float inl = inpG * inputs[INPUT_L_OR_MONO].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
        float inr = inpG * inputs[INPUT_R].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;

        if (inputConnected(INPUT_L_OR_MONO) && !inputConnected(INPUT_R))
        {
            bufferL[bufferPos] = inl;
            bufferR[bufferPos] = inl;
        }
        else
        {
            bufferL[bufferPos] = inl;
            bufferR[bufferPos] = inr;
        }

        if (SurgeFXTraits<effectNum>::hasModulatorSignal)
        {
            float mg = getParam(MODULATOR_GAIN);
            float ml = mg * inputs[MODULATOR_L_OR_MONO].getVoltageSum();
            float mr = mg * inputs[MODULATOR_R].getVoltageSum();
            if (inputConnected(MODULATOR_L_OR_MONO) && !inputConnected(MODULATOR_R))
            {
                modulatorL[bufferPos] = ml;
                modulatorR[bufferPos] = ml;
            }
            else
            {
                modulatorL[bufferPos] = ml;
                modulatorR[bufferPos] = mr;
            }
        }

        bufferPos++;
        if (bufferPos >= BLOCK_SIZE)
        {
            std::memcpy(processedL, bufferL, BLOCK_SIZE * sizeof(float));
            std::memcpy(processedR, bufferR, BLOCK_SIZE * sizeof(float));

            if (SurgeFXTraits<effectNum>::hasModulatorSignal)
            {
                std::memcpy(storage->audio_in_nonOS[0], modulatorL, BLOCK_SIZE * sizeof(float));
                std::memcpy(storage->audio_in_nonOS[1], modulatorR, BLOCK_SIZE * sizeof(float));
            }

            if (inputConnected(CLOCK_CV_INPUT))
            {
                updateBPMFromClockCV(inputs[CLOCK_CV_INPUT].getVoltage(), args.sampleTime,
                                     args.sampleRate);
            }
            else
            {
                updateBPMFromClockCV(1, args.sampleTime, args.sampleRate);
            }

            for (auto binding : pb)
                if (binding)
                    binding->update(pc, this);
            pc.update(this);

            copyGlobaldataSubset(storage_id_start, storage_id_end);
            surge_effect->process_ringout(processedL, processedR, true);

            bufferPos = 0;
        }

        float outl = outG * processedL[bufferPos] * SURGE_TO_RACK_OSC_MUL;
        float outr = outG * processedR[bufferPos] * SURGE_TO_RACK_OSC_MUL;

        if (outputConnected(OUTPUT_L_OR_MONO) && !outputConnected(OUTPUT_R))
        {
            outputs[OUTPUT_L_OR_MONO].setVoltage(0.5 * (outl + outr));
        }
        else
        {
            outputs[OUTPUT_L_OR_MONO].setVoltage(outl);
            outputs[OUTPUT_R].setVoltage(outr);
        }
    }

    std::unique_ptr<Effect> surge_effect;
    FxStorage *fxstorage;
};
