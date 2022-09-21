#ifndef SURGE_XT_RACK_FXHPP
#define SURGE_XT_RACK_FXHPP

#include "SurgeXT.hpp"
#include "dsp/Effect.h"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"

namespace sst::surgext_rack::fx
{
template <int fxType> struct FX;

template <int fxType> struct FXConfig
{
    struct LayoutItem
    {
        // order matters a bit on this enum. knobs contiguous pls
        enum Type {
            KNOB9,
            KNOB12,
            KNOB16,
            PORT,
            GROUP_LABEL,
            LCD_BG,
            LCD_MENU_ITEM,
            POWER_LIGHT,
            EXTEND_LIGHT,
            ERROR
        } type{ERROR};
        std::string label{"ERR"};
        int parId{-1};
        float xcmm{-1}, ycmm{-1};
        float spanmm{0}; // for group label only

        static LayoutItem createLCDArea(float ht)
        {
            auto res = LayoutItem();
            res.type = LCD_BG;
            res.ycmm = ht;
            return res;
        }
        static LayoutItem createGrouplabel(const std::string &label, float xcmm, float ycmm, float span)
        {
            auto res = LayoutItem();
            res.label = label;
            res.type = GROUP_LABEL;
            res.xcmm = xcmm;
            res.ycmm = ycmm;
            res.spanmm = span;
            return res;
        }
    };
    typedef std::vector<LayoutItem> layout_t;
    static layout_t getLayout() { return {}; }


    static constexpr int extraInputs() { return 0; }
    static void configExtraInputs(FX<fxType> *M) {}
    static void processExtraInputs(FX<fxType> *M) {}

    static constexpr int specificParamCount() { return 0; }
    static void configSpecificParams(FX<fxType> *M) {}
    static void processSpecificParams(FX<fxType> *M) {}

    static constexpr int panelWidthInScrews() { return 12; }
    static constexpr bool usesSideband() { return false; }
    static constexpr bool usesClock() { return false; }
};

template <int fxType> struct FX : modules::XTModule
{
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        FX_PARAM_0 = 0,
        FX_MOD_PARAM_0 = FX_PARAM_0 + n_fx_params,
        FX_SPECIFIC_PARAM_0 = FX_MOD_PARAM_0 + n_fx_params * n_mod_inputs,
        NUM_PARAMS = FX_SPECIFIC_PARAM_0 + FXConfig<fxType>::specificParamCount()
    };

    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        SIDEBAND_L,
        SIDEBAND_R,
        INPUT_CLOCK,
        MOD_INPUT_0,
        INPUT_SPECIFIC_0 = MOD_INPUT_0 + n_mod_inputs,
        NUM_INPUTS = INPUT_SPECIFIC_0 + FXConfig<fxType>::extraInputs()
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

    modules::MonophonicModulationAssistant<FX<fxType>, n_fx_params, FX_PARAM_0, n_mod_inputs, MOD_INPUT_0>
        modAssist;

    FX() : XTModule()
    {
        setupSurge();
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < n_fx_params; ++i)
        {
            configParam<modules::SurgeParameterParamQuantity>(FX_PARAM_0 + i, 0, 1,
                                                              fxstorage->p[i].get_value_f01());
        }

        for (int i = 0; i < n_fx_params * n_mod_inputs; ++i)
        {
            configParam(FX_MOD_PARAM_0 + i, -1, 1, 0);
        }

        FXConfig<fxType>::configSpecificParams(this);

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        configInput(INPUT_CLOCK, "Clock/Tempo CV");
        for (int m = 0; m < n_mod_inputs; ++m)
        {
            auto s = std::string("Modulation Signal ") + std::to_string(m + 1);
            configInput(MOD_INPUT_0 + m, s);
        }
        FXConfig<fxType>::configExtraInputs(this);
        configOutput(OUTPUT_L, "Left (or Mono merged)");
        configOutput(OUTPUT_R, "Right");

        modAssist.initialize(this);
    }

    void moduleSpecificSampleRateChange() override {
        clockProc.setSampleRate(APP->engine->getSampleRate());
    }
    modules::ClockProcessor<FX<fxType>> clockProc;

    float modScales[n_fx_params];
    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false);

        fxstorage = &(storage->getPatch().fx[0]);
        fxstorage->type.val.i = fxType;

        surge_effect.reset(spawn_effect(fxType, storage.get(), fxstorage,
                                        storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();
        surge_effect->init_default_values();

        // This is a micro-hack to stop ranges blowing up
        fxstorage->return_level.id = -1;
        setupStorageRanges(&(fxstorage->type), &(fxstorage->p[n_fx_params - 1]));

        for (int i=0; i<n_fx_params; ++i)
        {
            modScales[i] = fxstorage->p[i].val_max.f - fxstorage->p[i].val_min.f;
        }

        std::fill(processedL, processedL + BLOCK_SIZE, 0);
        std::fill(processedR, processedR + BLOCK_SIZE, 0);
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        if (paramId < FX_PARAM_0 || paramId >= FX_PARAM_0 + n_fx_params)
            return nullptr;

        return &fxstorage->p[paramId - FX_PARAM_0];
    }


    static int modulatorIndexFor(int baseParam, int modulator)
    {
        int offset = baseParam - FX_PARAM_0;
        return FX_MOD_PARAM_0 + offset * n_mod_inputs + modulator;
    }

    float modulationDisplayValue(int paramId) override
    {
        int idx = paramId - FX_PARAM_0;
        if (idx < 0 || idx >= n_fx_params)
            return 0;
        return modAssist.modvalues[idx];
    }

    bool isBipolar(int paramId) override {
        if (paramId >= FX_PARAM_0 && paramId <= FX_PARAM_0 + n_fx_params)
        {
            return fxstorage->p[paramId-FX_PARAM_0].is_bipolar();
        }
        return false;
    }

    std::string getName() override { return std::string("FX<") + fx_type_names[fxType] + ">"; }

    int bufferPos{0};
    float bufferL alignas(16)[BLOCK_SIZE], bufferR alignas(16)[BLOCK_SIZE];
    float modulatorL alignas(16)[BLOCK_SIZE], modulatorR alignas(16)[BLOCK_SIZE];
    float processedL alignas(16)[BLOCK_SIZE], processedR alignas(16)[BLOCK_SIZE];

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        if constexpr (FXConfig<fxType>::usesClock())
        {
            if (inputs[INPUT_CLOCK].isConnected())
                clockProc.process(this, INPUT_CLOCK);
            else
                clockProc.disconnect(this);
        }

        float inl = inputs[INPUT_L].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
        float inr = inputs[INPUT_R].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;

        if (inputs[INPUT_L].isConnected() && !inputs[INPUT_R].isConnected())
        {
            bufferL[bufferPos] = inl;
            bufferR[bufferPos] = inl;
        }
        else
        {
            bufferL[bufferPos] = inl;
            bufferR[bufferPos] = inr;
        }
        bufferPos++;

        if constexpr (FXConfig<fxType>::usesSideband())
        {
            if (inputConnected(SIDEBAND_L) && !inputConnected(SIDEBAND_R))
            {
                float ml = inputs[SIDEBAND_L].getVoltageSum();
                modulatorL[bufferPos] = ml;
                modulatorR[bufferPos] = ml;
            }
            else
            {
                modulatorL[bufferPos] = inputs[SIDEBAND_L].getVoltageSum();
                modulatorR[bufferPos] = inputs[SIDEBAND_R].getVoltageSum();
            }
        }

        if (bufferPos >= BLOCK_SIZE)
        {
            modAssist.setupMatrix(this);
            modAssist.updateValues(this);

            std::memcpy(processedL, bufferL, BLOCK_SIZE * sizeof(float));
            std::memcpy(processedR, bufferR, BLOCK_SIZE * sizeof(float));

            if constexpr (FXConfig<fxType>::usesSideband())
            {
                std::memcpy(storage->audio_in_nonOS[0], modulatorL, BLOCK_SIZE * sizeof(float));
                std::memcpy(storage->audio_in_nonOS[1], modulatorR, BLOCK_SIZE * sizeof(float));
            }

            if constexpr (FXConfig<fxType>::specificParamCount() > 0)
            {
                FXConfig<fxType>::processSpecificParams(this);
            }

            for (int i = 0; i < n_fx_params; ++i)
            {
                fxstorage->p[i].set_value_f01(modAssist.basevalues[i]);
            }

            FXConfig<fxType>::processExtraInputs(this);

            copyGlobaldataSubset(storage_id_start, storage_id_end);

            auto *oap = &fxstorage->p[0];
            auto *eap = &fxstorage->p[n_fx_params-1];
            auto &pt = storage->getPatch().globaldata;
            int idx = 0;
            while (oap <= eap)
            {
                if (oap->valtype == vt_float)
                {
                    pt[oap->id].f += modAssist.modvalues[idx] * modScales[idx];

                }
                idx++;
                oap++;
            }

            surge_effect->process_ringout(processedL, processedR, true);

            bufferPos = 0;
        }

        float outl = processedL[bufferPos] * SURGE_TO_RACK_OSC_MUL;
        float outr = processedR[bufferPos] * SURGE_TO_RACK_OSC_MUL;

        if (outputs[OUTPUT_L].isConnected() && !outputs[OUTPUT_R].isConnected())
        {
            outputs[OUTPUT_L].setVoltage(0.5 * (outl + outr));
        }
        else
        {
            outputs[OUTPUT_L].setVoltage(outl);
            outputs[OUTPUT_R].setVoltage(outr);
        }
    }

    int polyChannelCount()
    {
        return 1; // these arent' polyphonic fx
    }

    void activateTempoSync()
    {
        auto p = &fxstorage->p[0];
        auto pe = &fxstorage->p[n_fx_params - 1];
        while ( p <= pe)
        {
            if (p->can_temposync())
                p->temposync = true;
            ++p;
        }
    }
    void deactivateTempoSync()
    {
        auto p = &fxstorage->p[0];
        auto pe = &fxstorage->p[n_fx_params - 1];
        while ( p <= pe)
        {
            if (p->can_temposync())
                p->temposync = false;
            ++p;
        }
    }

    std::unique_ptr<Effect> surge_effect;
    FxStorage *fxstorage{nullptr};
};
} // namespace sst::surgext_rack::fx
#endif
