#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/effect/Effect.h"
#include "rack.hpp"
#include <cstring>

#define NUM_FX_PARAMS 12

template<int effectNum>
struct SurgeFXName {
    static std::string getName() { return "GENERIC"; }
};

template<> struct SurgeFXName<fxt_delay> { static std::string getName() { return "DELAY"; } };
template<> struct SurgeFXName<fxt_eq> { static std::string getName() { return "EQ"; } };
template<> struct SurgeFXName<fxt_phaser> { static std::string getName() { return "PHASER"; } };
template<> struct SurgeFXName<fxt_rotaryspeaker> { static std::string getName() { return "ROTARY"; } };
template<> struct SurgeFXName<fxt_distortion> { static std::string getName() { return "DISTORT"; } };
template<> struct SurgeFXName<fxt_reverb> { static std::string getName() { return "REVERB"; } };
template<> struct SurgeFXName<fxt_reverb2> { static std::string getName() { return "REVERB2"; } };
template<> struct SurgeFXName<fxt_freqshift> { static std::string getName() { return "FREQSHIFT"; } };
template<> struct SurgeFXName<fxt_chorus4> { static std::string getName() { return "CHORUS"; } };
template<> struct SurgeFXName<fxt_conditioner> { static std::string getName() { return "CONDITION"; } };
template<> struct SurgeFXName<fxt_vocoder> { static std::string getName() { return "VOC"; } };

template<int effectNum>
struct SurgeFXTraits
{
    static bool constexpr hasModulatorSignal = false;
};

template<> struct SurgeFXTraits<fxt_vocoder> { static bool constexpr hasModulatorSignal = true; };

template<int effectNum>
struct SurgeFX : virtual SurgeModuleCommon {
    enum ParamIds {
        FX_PARAM_0 = 0,
        INPUT_GAIN = FX_PARAM_0 + NUM_FX_PARAMS,
        OUTPUT_GAIN,
        PARAM_TEMPOSYNC_0,
        MODULATOR_GAIN = PARAM_TEMPOSYNC_0 + NUM_FX_PARAMS,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_L_OR_MONO,
        INPUT_R,

        FX_PARAM_INPUT_0,

        CLOCK_CV_INPUT = FX_PARAM_INPUT_0 + NUM_FX_PARAMS,

        MODULATOR_L_OR_MONO,
        MODULATOR_R,
        
        NUM_INPUTS
    };
    enum OutputIds { OUTPUT_L_OR_MONO, OUTPUT_R, NUM_OUTPUTS };
    enum LightIds {
        CAN_TEMPOSYNC_0,
        NUM_LIGHTS = CAN_TEMPOSYNC_0 + NUM_FX_PARAMS
    };

    ParamCache pc;
    
    StringCache paramDisplayCache[NUM_FX_PARAMS];
    StringCache labelCache[NUM_FX_PARAMS];
    StringCache groupCache[NUM_FX_PARAMS];

    SurgeFX() : SurgeModuleCommon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = 0; i < 12; ++i) {
            configParam(FX_PARAM_0 + i, 0, 1, 0 );
            configParam(PARAM_TEMPOSYNC_0 + i, 0, 1, 0 );
        }
        configParam(INPUT_GAIN, 0, 1, 1);
        configParam(OUTPUT_GAIN, 0, 1, 1);
        configParam(MODULATOR_GAIN, 0, 1, 1 );
        
        setupSurge();
    }

    virtual std::string getName() override { return SurgeFXName<effectNum>::getName(); }

    void setupSurge() {
        pc.resize(NUM_PARAMS);
        
        setupSurgeCommon();

        fxstorage = &(storage->getPatch().fx[0]);
        fxstorage->type.val.i = effectNum;

        surge_effect.reset(spawn_effect(effectNum, storage.get(),
                                        &(storage->getPatch().fx[0]),
                                        storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();
        surge_effect->init_default_values();

        // Do default values
        reorderSurgeParams();

        if( ! firstRespawnIsFromJSON )
        {
            // Set up the parametres based on the thingy
            for( int i=0; i<n_fx_params; ++i )
            {
                int o = orderToParam[ i ];
                setParam(FX_PARAM_0 + i, fxstorage->p[o].get_value_f01() );
            }
        }
        
        
        for (auto i = 0; i < BLOCK_SIZE; ++i) {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
            modulatorL[i] = 0.0f;
            modulatorR[i] = 0.0f;
            processedL[i] = 0.0f;
            processedR[i] = 0.0f;
        }


        setupStorageRanges((Parameter *)fxstorage, &(fxstorage->p[n_fx_params-1]));
    }

    void reorderSurgeParams() {
        if (surge_effect.get()) {
            std::vector<std::pair<int, int>> orderTrack;
            for (auto i = 0; i < n_fx_params; ++i) {
                if (fxstorage->p[i].posy_offset) {
                    orderTrack.push_back(std::pair<int, int>(
                        i, i * 2 + fxstorage->p[i].posy_offset));
                } else {
                    orderTrack.push_back(std::pair<int, int>(i, 10000));
                }
            }
            std::sort(
                orderTrack.begin(), orderTrack.end(),
                [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                    return a.second < b.second;
                });
            orderToParam.clear();
            for (auto a : orderTrack) {
                orderToParam.push_back(a.first);
                int idx = orderToParam.size() - 1;
                labelCache[idx].reset(fxstorage->p[a.first].get_name());
            }
        }

        // I hate having to use this API so much...
        for (auto i = 0; i < n_fx_params; ++i) {
            int fpos = fxstorage->p[orderToParam[i]].posy +
                       10 * fxstorage->p[orderToParam[i]].posy_offset;
            for (auto j = 0; j < n_fx_params; ++j) {
                if (surge_effect->group_label(j) &&
                    162 + 8 + 10 * surge_effect->group_label_ypos(j) <
                        fpos // constants for SurgeGUIEditor. Sigh.
                ) {
                    groupCache[i].reset( surge_effect->group_label(j) );
                }
            }
        }

        for (auto i = 0; i < NUM_FX_PARAMS; ++i) {
            paramDisplayCache[i].reset("");
        }
    }

    bool canTempoSync(int i) {
        return fxstorage->p[orderToParam[i]].can_temposync();
    }
    
    float bufferL alignas(16)[BLOCK_SIZE], bufferR alignas(16)[BLOCK_SIZE];
    float modulatorL alignas(16)[BLOCK_SIZE], modulatorR alignas(16)[BLOCK_SIZE];
    float processedL alignas(16)[BLOCK_SIZE], processedR
        alignas(16)[BLOCK_SIZE];
    int bufferPos = BLOCK_SIZE - 1;

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);


        float inl = inpG * getInput(INPUT_L_OR_MONO) * RACK_TO_SURGE_OSC_MUL;
        float inr = inpG * getInput(INPUT_R) * RACK_TO_SURGE_OSC_MUL;

        if( inputConnected(INPUT_L_OR_MONO) && ! inputConnected(INPUT_R) )
        {
            bufferL[bufferPos] = inl;
            bufferR[bufferPos] = inl;
        }
        else
        {
            bufferL[bufferPos] = inl;
            bufferR[bufferPos] = inr;
        }

        if( SurgeFXTraits<effectNum>::hasModulatorSignal )
        {
            float mg = getParam(MODULATOR_GAIN);
            float ml = mg * getInput(MODULATOR_L_OR_MONO);
            float mr = mg * getInput(MODULATOR_R);
            if( inputConnected(MODULATOR_L_OR_MONO) && ! inputConnected(MODULATOR_R) )
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
        if (bufferPos >= BLOCK_SIZE) {
            std::memcpy(processedL, bufferL, BLOCK_SIZE * sizeof(float));
            std::memcpy(processedR, bufferR, BLOCK_SIZE * sizeof(float));

            if( SurgeFXTraits<effectNum>::hasModulatorSignal )
            {
                std::memcpy( storage->audio_in_nonOS[ 0 ], modulatorL, BLOCK_SIZE * sizeof(float) );
                std::memcpy( storage->audio_in_nonOS[ 1 ], modulatorR, BLOCK_SIZE * sizeof(float) );
            }
            
            
            bool newBPM = false;
            if( inputConnected(CLOCK_CV_INPUT) )
            {
                newBPM = updateBPMFromClockCV(getInput(CLOCK_CV_INPUT), args.sampleTime, args.sampleRate );
            }
            else
            {
                // FIXME - only once please
                newBPM = updateBPMFromClockCV(1, args.sampleTime, args.sampleRate );
            }
            
            for (int i = 0; i < n_fx_params; ++i) {
                bool changed = newBPM;
                if(pc.changed(FX_PARAM_0 + i, this))
                {
                    changed = true;
                }
                if( pc.changed(PARAM_TEMPOSYNC_0 + i, this) )
                {
                    changed = true;
                    fxstorage->p[orderToParam[i]].temposync =
                        getParam(PARAM_TEMPOSYNC_0 + i ) > 0.5;
                }
                if( changed )
                {
                    fxstorage->p[orderToParam[i]].set_value_f01(
                        getParam(FX_PARAM_0 + i));
                    char txt[256];
                    fxstorage->p[orderToParam[i]].get_display(txt, false, 0);
                    if( getParam(PARAM_TEMPOSYNC_0 + i) > 0.5 )
                    {
		        char ntxt[256];
                        snprintf( ntxt, 256, "%s @ %5.1lf bpm", txt, lastBPM );
			strcpy(txt, ntxt);
                    }
                    paramDisplayCache[i].reset(txt);
                }
            }
            pc.update(this);

            for (int i = 0; i < n_fx_params; ++i) {
                fxstorage->p[orderToParam[i]].set_value_f01(
                    getParam(FX_PARAM_0 + i) + (getInput(FX_PARAM_INPUT_0 + i)) * RACK_TO_SURGE_CV_MUL );
            }

            copyGlobaldataSubset(storage_id_start, storage_id_end);
            surge_effect->process_ringout(processedL, processedR, true);

            bufferPos = 0;
        }

        float outl = outG * processedL[bufferPos] * SURGE_TO_RACK_OSC_MUL;
        float outr = outG * processedR[bufferPos] * SURGE_TO_RACK_OSC_MUL;

        if( outputConnected(OUTPUT_L_OR_MONO) && ! outputConnected(OUTPUT_R) )
        {
            setOutput(OUTPUT_L_OR_MONO, 0.5 * ( outl + outr ) );
        }
        else
        {
            setOutput(OUTPUT_L_OR_MONO, outl );
            setOutput(OUTPUT_R, outr );
        }
    }

    std::unique_ptr<Effect> surge_effect;
    FxStorage *fxstorage;
    std::vector<int> orderToParam;
};
