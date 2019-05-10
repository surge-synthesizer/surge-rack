#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "dsp/effect/Effect.h"
#include "rack.hpp"
#include <cstring>

#define NUM_FX_PARAMS 12
struct SurgeFX : virtual SurgeModuleCommon {
    enum ParamIds {
        FX_TYPE = 0,
        FX_PARAM_0,
        FX_EXTEND_0 = FX_PARAM_0 + NUM_FX_PARAMS,
        INPUT_GAIN = FX_EXTEND_0 + NUM_FX_PARAMS,
        OUTPUT_GAIN,
        FX_PARAM_GAIN_0,
        NUM_PARAMS = FX_PARAM_GAIN_0 + NUM_FX_PARAMS
    };
    enum InputIds {
        INPUT_R_OR_MONO,
        INPUT_L,

        FX_PARAM_INPUT_0,

        NUM_INPUTS = FX_PARAM_INPUT_0 + NUM_FX_PARAMS
    };
    enum OutputIds { OUTPUT_R_OR_MONO, OUTPUT_L, NUM_OUTPUTS };
    enum LightIds { NUM_LIGHTS };

    ParamCache pc;
    
    StringCache paramDisplayCache[NUM_FX_PARAMS];

    std::string effectNameCache = "";
    bool effectNameCacheDirty = true;
    std::string labelCache[NUM_FX_PARAMS];
    bool labelCacheDirty[NUM_FX_PARAMS];
    std::string sublabelCache[NUM_FX_PARAMS];
    bool sublabelCacheDirty[NUM_FX_PARAMS];
    std::string groupCache[NUM_FX_PARAMS];
    bool groupCacheDirty[NUM_FX_PARAMS];

#if RACK_V1
    SurgeFX() : SurgeModuleCommon() {
        setupSurge();

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FX_TYPE, 0, 9, 1);
        for (int i = 0; i < 12; ++i) {
            configParam(FX_PARAM_0 + i, 0, 1, fxstorage->p[i].get_value_f01());
            configParam(FX_PARAM_GAIN_0 + i, 0, 1, 0.2);
        }
        configParam(INPUT_GAIN, 0, 1, 1);
        configParam(OUTPUT_GAIN, 0, 1, 1);
    }
#else
    SurgeFX()
        : SurgeModuleCommon(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setupSurge();
    }
#endif

    virtual std::string getName() override { return "FX"; }

    void setupSurge() {
        pc.resize(NUM_PARAMS);
        
        setupSurgeCommon();

        fxstorage = &(storage->getPatch().fx[0]);
        fxstorage->type.val.i = 1;

        surge_effect.reset(spawn_effect(1, storage.get(),
                                        &(storage->getPatch().fx[0]),
                                        storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();

        effectNameCacheDirty = true;
        effectNameCache = surge_effect->get_effectname();

        fxstorage->type.val.i = 1;

        // Because I know this
        fxstorage->p[0].set_value_f01(0.6);
        fxstorage->p[1].set_value_f01(0.54);
        fxstorage->p[2].set_value_f01(0.9);
        fxstorage->p[3].set_value_f01(0.5);
        fxstorage->p[4].set_value_f01(0);
        fxstorage->p[5].set_value_f01(1);
        fxstorage->p[6].set_value_f01(0.5);
        fxstorage->p[7].set_value_f01(0.0);
        fxstorage->p[8].set_value_f01(0.0);
        fxstorage->p[9].set_value_f01(0.0);
        fxstorage->p[10].set_value_f01(1.0);
        fxstorage->p[11].set_value_f01(0.0);

        reorderSurgeParams();

        for (auto i = 0; i < BLOCK_SIZE; ++i) {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
            processedL[i] = 0.0f;
            processedR[i] = 0.0f;
        }

        for (auto i = 0; i < BLOCK_SIZE; ++i) {
            bufferL[i] = 0.0f;
            bufferR[i] = 0.0f;
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
                labelCache[idx] = fxstorage->p[a.first].get_name();
                ;
                labelCacheDirty[idx] = true;
            }
        }

        // I hate having to use this API so much...
        for (auto i = 0; i < n_fx_params; ++i) {
            int fpos = fxstorage->p[orderToParam[i]].posy +
                       10 * fxstorage->p[orderToParam[i]].posy_offset;
            sublabelCacheDirty[i] = true;
            for (auto j = 0; j < n_fx_params; ++j) {
                if (surge_effect->group_label(j) &&
                    162 + 8 + 10 * surge_effect->group_label_ypos(j) <
                        fpos // constants for SurgeGUIEditor. Sigh.
                ) {
                    sublabelCache[i] = surge_effect->group_label(j);
                }
            }
        }

        for (auto i = 0; i < NUM_FX_PARAMS; ++i) {
            paramDisplayCache[i].reset("");
        }
    }

    float bufferL alignas(16)[BLOCK_SIZE], bufferR alignas(16)[BLOCK_SIZE];
    float processedL alignas(16)[BLOCK_SIZE], processedR
        alignas(16)[BLOCK_SIZE];
    int bufferPos = BLOCK_SIZE - 1;

    inline int getTypeParam() { return floor(getParam(FX_TYPE)); }

#if RACK_V1
    void process(const typename rack::Module::ProcessArgs &args) override
#else
    void step() override
#endif
    {
        float inpG = getParam(INPUT_GAIN);
        float outG = getParam(OUTPUT_GAIN);

        
        bufferR[bufferPos] = inpG * getInput(INPUT_R_OR_MONO) / 5.0;
        bufferL[bufferPos] = inpG * getInput(INPUT_L) /
                             5.0; // Surge works on a +/- 1; rack works on +/- 5
        // FIXME - deal with MONO when L not hooked up

        bufferPos++;
        if (bufferPos >= BLOCK_SIZE) {
            std::memcpy(processedL, bufferL, BLOCK_SIZE * sizeof(float));
            std::memcpy(processedR, bufferR, BLOCK_SIZE * sizeof(float));

            int tp = getTypeParam();
            if (tp != fxstorage->type.val.i &&
                tp != 0) // FIXME: Deal with the 0 case
            {
                fxstorage->type.val.i = tp;
                surge_effect.reset(spawn_effect(tp, storage.get(),
                                                &(storage->getPatch().fx[0]),
                                                storage->getPatch().globaldata));
                surge_effect->init();
                surge_effect->init_ctrltypes();
                reorderSurgeParams();
                effectNameCacheDirty = true;
                effectNameCache = surge_effect->get_effectname();
            }
            
            for (int i = 0; i < n_fx_params; ++i) {
                if(pc.changed(FX_PARAM_0 + i, this))
                {
                    fxstorage->p[orderToParam[i]].set_value_f01(
                        getParam(FX_PARAM_0 + i));
                    char txt[256];
                    fxstorage->p[orderToParam[i]].get_display(txt, false, 0);
                    
                    paramDisplayCache[i].reset(txt);
                }
            }
            pc.update(this);

            for (int i = 0; i < n_fx_params; ++i) {
                fxstorage->p[orderToParam[i]].set_value_f01(
                    getParam(FX_PARAM_0 + i) + (getInput(FX_PARAM_INPUT_0 + i)) /
                    10.0 *
                    getParam(FX_PARAM_GAIN_0 + i));
            }

            copyGlobaldataSubset(storage_id_start, storage_id_end);
            surge_effect->process_ringout(processedL, processedR, true);

            bufferPos = 0;
        }

        setOutput(OUTPUT_R_OR_MONO, outG * processedR[bufferPos] * 5.0);
        setOutput(OUTPUT_L, outG * processedL[bufferPos] * 5.0);
    }

    std::string getLabel(int gi) { return labelCache[gi]; }
    bool getLabelDirty(int gi) {
        bool res = labelCacheDirty[gi];
        labelCacheDirty[gi] = false;
        return res;
    }
    std::string getSubLabel(int gi) { return sublabelCache[gi]; }

    bool getSubLabelDirty(int gi) {
        bool res = sublabelCacheDirty[gi];
        sublabelCacheDirty[gi] = false;
        return res;
    }

    bool getStringDirty(int gi) {
        return true; // fixme of course
    }

    std::string getEffectNameString() { return effectNameCache; }

    bool getEffectNameStringDirty() {
        auto res = effectNameCacheDirty;
        effectNameCacheDirty = false;
        return res;
    }

    std::unique_ptr<Effect> surge_effect;
    FxStorage *fxstorage;
    std::vector<int> orderToParam;
};
