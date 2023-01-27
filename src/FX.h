/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_FXHPP
#define SURGE_XT_RACK_FXHPP

#include "SurgeXT.h"
#include "dsp/Effect.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"
#include "FxPresetAndClipboardManager.h"

#include "LayoutEngine.h"

namespace sst::surgext_rack::fx
{
template <int fxType> struct FX;

template <int fxType> struct FXConfig
{
    typedef sst::surgext_rack::layout::LayoutItem LayoutItem;
    typedef std::vector<LayoutItem> layout_t;
    static layout_t getLayout() { return {LayoutItem::createPresetLCDArea()}; }

    static constexpr int extraInputs() { return 0; }
    static constexpr int extraSchmidtTriggers() { return 1; }
    static void configExtraInputs(FX<fxType> *M) {}
    static void processExtraInputs(FX<fxType> *M) {}

    static constexpr int extraOutputs() { return 0; }
    static void configExtraOutputs(FX<fxType> *M) {}
    static void populateExtraOutputs(FX<fxType> *M, int chan, Effect *fx) {}

    static constexpr int specificParamCount() { return 0; }
    static void configSpecificParams(FX<fxType> *M) {}
    static void processSpecificParams(FX<fxType> *M) {}
    static void loadPresetOntoSpecificParams(FX<fxType> *M,
                                             const Surge::Storage::FxUserPreset::Preset &)
    {
    }
    static bool isDirtyPresetVsSpecificParams(FX<fxType> *M,
                                              const Surge::Storage::FxUserPreset::Preset &)
    {
        return false;
    }

    static constexpr int panelWidthInScrews() { return 12; }
    static constexpr bool usesSideband() { return false; }
    static constexpr bool usesClock() { return false; }
    static constexpr bool usesPresets() { return true; }
    static constexpr int numParams() { return n_fx_params; }
    static constexpr bool allowsPolyphony() { return true; }

    static constexpr float rescaleInputFactor() { return 1.0; }
    static constexpr bool softclipOutput() { return false; }
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
        EXTRA_OUTPUT_0,
        NUM_OUTPUTS = EXTRA_OUTPUT_0 + FXConfig<fxType>::extraOutputs()
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    modules::MonophonicModulationAssistant<FX<fxType>, FXConfig<fxType>::numParams(), FX_PARAM_0,
                                           n_mod_inputs, MOD_INPUT_0>
        modAssist;

    modules::ModulationAssistant<FX<fxType>, FXConfig<fxType>::numParams(), FX_PARAM_0,
                                 n_mod_inputs, MOD_INPUT_0>
        polyModAssist;

    FX() : XTModule()
    {
        std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
        setupSurge();
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        int lastParam{0};
        for (int i = 0; i < n_fx_params; ++i)
        {
            if (fxstorage->p[i].ctrltype != ct_none)
                lastParam = i;
            configParam<modules::SurgeParameterParamQuantity>(FX_PARAM_0 + i, 0, 1,
                                                              fxstorage->p[i].get_value_f01());
        }

        if (lastParam != FXConfig<fxType>::numParams() - 1)
        {
            std::cout << "WARNING: " << fx_type_names[fxType] << " last non-param is "
                      << lastParam + 1 << " not " << FXConfig<fxType>::numParams() << std::endl;
        }

        for (int i = 0; i < n_fx_params * n_mod_inputs; ++i)
        {
            std::string name{"Mod"};
            name += std::to_string((i - FX_MOD_PARAM_0) % 4 + 1);

            configParamNoRand<modules::SurgeParameterModulationQuantity>(FX_MOD_PARAM_0 + i, -1, 1,
                                                                         0, name)
                ->baseName = name;
        }

        FXConfig<fxType>::configSpecificParams(this);

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        configInput(INPUT_CLOCK, "Clock/Tempo CV");

        configInput(SIDEBAND_L, "Left Sideband");
        configInput(SIDEBAND_R, "Right Sideband");

        for (int m = 0; m < n_mod_inputs; ++m)
        {
            auto s = std::string("Modulation Signal ") + std::to_string(m + 1);
            configInput(MOD_INPUT_0 + m, s);
        }
        FXConfig<fxType>::configExtraInputs(this);
        configOutput(OUTPUT_L, "Left (or Mono merged)");
        configOutput(OUTPUT_R, "Right");
        FXConfig<fxType>::configExtraOutputs(this);

        modAssist.initialize(this);
        polyModAssist.initialize(this);
        if (maxPresets > 0)
            loadPreset(0, false, true);

        configBypass(INPUT_L, OUTPUT_L);
        configBypass(INPUT_R, OUTPUT_R);
        snapCalculatedNames();
    }

    void moduleSpecificSampleRateChange() override
    {
        clockProc.setSampleRate(APP->engine->getSampleRate());
    }
    typedef modules::ClockProcessor<FX<fxType>> clockProcessor_t;
    clockProcessor_t clockProc;

    // If you need em you have a scnmidt trigger for extra inputs
    // add one since 0 length arrays are gross and its just memory smidges
    // this could obviously be better with complicated specializations and enable ifs
    rack::dsp::SchmittTrigger extraInputTriggers[FXConfig<fxType>::extraSchmidtTriggers()];

    float modScales[n_fx_params];
    std::atomic<int> loadedPreset{-1}, maxPresets{0};
    std::atomic<bool> presetIsDirty{false};
    std::vector<Surge::Storage::FxUserPreset::Preset> presets;

    std::atomic<bool> polyphonicMode{false};

    void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS, false, true);

        fxstorage = &(storage->getPatch().fx[0]);
        fxstorage->type.val.i = fxType;

        setupStorageRanges(&(fxstorage->type), &(fxstorage->p[n_fx_params - 1]));
        copyGlobaldataSubset(storage_id_start, storage_id_end);

        surge_effect.reset(
            spawn_effect(fxType, storage.get(), fxstorage, storage->getPatch().globaldata));
        surge_effect->init();
        surge_effect->init_ctrltypes();
        surge_effect->init_default_values();

        // This is a micro-hack to stop ranges blowing up
        fxstorage->return_level.id = -1;

        for (int i = 0; i < n_fx_params; ++i)
        {
            modScales[i] = fxstorage->p[i].val_max.f - fxstorage->p[i].val_min.f;
        }

        memset(processedL, 0, sizeof(float) * MAX_POLY * BLOCK_SIZE);
        memset(processedR, 0, sizeof(float) * MAX_POLY * BLOCK_SIZE);

        if (FXConfig<fxType>::usesPresets())
        {
            auto sect = storage->getSnapshotSection("fx");
            if (sect)
            {
                auto type = sect->FirstChildElement();
                while (type)
                {
                    int i;

                    if (type->Value() && strcmp(type->Value(), "type") == 0 &&
                        type->QueryIntAttribute("i", &i) == TIXML_SUCCESS && i == fxType)
                    {
                        auto kid = type->FirstChildElement();
                        while (kid)
                        {
                            if (strcmp(kid->Value(), "snapshot") == 0)
                            {
                                auto p = Surge::Storage::FxUserPreset::Preset();
                                p.type = fxType;
                                for (int q = 0; q < n_fx_params; ++q)
                                {
                                    // Set up with default values remember q index
                                    if (fxstorage->p[q].valtype == vt_float)
                                    {
                                        p.p[q] = fxstorage->p[q].val.f;
                                    }
                                    if (fxstorage->p[q].valtype == vt_int)
                                    {
                                        p.p[q] = fxstorage->p[q].val.i;
                                    }
                                    if (fxstorage->p[q].valtype == vt_bool)
                                    {
                                        p.p[q] = fxstorage->p[q].val.b;
                                    }
                                }
                                storage->fxUserPreset->readFromXMLSnapshot(p, kid);
                                p.isFactory = true;
                                presets.push_back(p);
                            }
                            kid = kid->NextSiblingElement();
                        }
                    }
                    type = type->NextSiblingElement();
                }
            }
            auto xtrapresets = storage->fxUserPreset->getPresetsForSingleType(fxType);
            for (auto p : xtrapresets)
                presets.push_back(p);
            maxPresets = presets.size();
        }
    }

    Parameter *surgeDisplayParameterForParamId(int paramId) override
    {
        if (paramId < FX_PARAM_0 || paramId >= FX_PARAM_0 + n_fx_params)
            return nullptr;

        return &fxstorage->p[paramId - FX_PARAM_0];
    }

    static int paramModulatedBy(int modIndex)
    {
        int offset = modIndex - FX_MOD_PARAM_0;
        if (offset >= n_mod_inputs * (n_fx_params + 1) || offset < 0)
            return -1;
        return offset / n_mod_inputs;
    }

    Parameter *surgeDisplayParameterForModulatorParamId(int modParamId) override
    {
        auto paramId = paramModulatedBy(modParamId);
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
        if (polyphonicMode)
            return polyModAssist.modvalues[idx][0];
        else
            return modAssist.modvalues[idx];
    }

    bool isBipolar(int paramId) override
    {
        if (paramId >= FX_PARAM_0 && paramId <= FX_PARAM_0 + n_fx_params)
        {
            return fxstorage->p[paramId - FX_PARAM_0].is_bipolar();
        }
        return false;
    }

    float value01for(int i, float f)
    {
        const auto &p = fxstorage->p[i];
        if (p.ctrltype == ct_none)
            return 0;

        if (p.valtype == vt_float)
        {
            return (f - p.val_min.f) / (p.val_max.f - p.val_min.f);
        }
        if (p.valtype == vt_int)
        {
            return Parameter::intScaledToFloat(f, p.val_max.i, p.val_min.i);
        }
        if (p.valtype == vt_bool)
        {
            return f > 0.5 ? 1 : 0;
        }
        return 0;
    }

    struct PresetChangeAction : rack::history::ModuleAction
    {
        json_t *moduleJ{nullptr};
        int presetId{-1};

        ~PresetChangeAction()
        {
            if (moduleJ)
                json_decref(moduleJ);
        }

        void stash(FX<fxType> *fx, int ps)
        {
            assert(fx);
            moduleId = fx->id;
            presetId = ps;
            moduleJ = APP->engine->moduleToJson(fx);
        }

        void undo()
        {
            auto *module = APP->engine->getModule(moduleId);
            if (module)
            {
                module->fromJson(moduleJ);
            }
        }

        void redo()
        {
            auto *module = APP->engine->getModule(moduleId);
            auto *fx = dynamic_cast<FX<fxType> *>(module);
            if (fx)
                fx->loadPreset(presetId);
        }
    };

    void loadPreset(int which, bool recordHistory = true, bool resetDefaults = false)
    {
        if (recordHistory)
        {
            auto h = new PresetChangeAction();
            h->stash(this, which);
            APP->history->push(h);
        }

        const auto &ps = presets[which];

        for (int i = 0; i < n_fx_params; ++i)
        {
            paramQuantities[FX_PARAM_0 + i]->setValue(value01for(i, ps.p[i]));
            if (resetDefaults)
            {
                paramQuantities[FX_PARAM_0 + i]->defaultValue =
                    paramQuantities[FX_PARAM_0 + i]->getValue();
            }
        }

        FXConfig<fxType>::loadPresetOntoSpecificParams(this, ps);

        loadedPreset = (int)which;
        presetIsDirty = false;
    }

    std::string getName() override { return std::string("FX<") + fx_type_names[fxType] + ">"; }

    int bufferPos{0};
    float bufferL alignas(16)[MAX_POLY][BLOCK_SIZE], bufferR alignas(16)[MAX_POLY][BLOCK_SIZE];
    float modulatorL alignas(16)[MAX_POLY][BLOCK_SIZE], modulatorR
        alignas(16)[MAX_POLY][BLOCK_SIZE];
    float processedL alignas(16)[MAX_POLY][BLOCK_SIZE], processedR
        alignas(16)[MAX_POLY][BLOCK_SIZE];

    float extraOutputs alignas(
        16)[std::max(1, FXConfig<fxType>::extraOutputs())][MAX_POLY][BLOCK_SIZE];

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        // auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

        if constexpr (FXConfig<fxType>::usesClock())
        {
            if (inputs[INPUT_CLOCK].isConnected())
                clockProc.process(this, INPUT_CLOCK);
            else
                clockProc.disconnect(this);
        }

        if (polyphonicMode)
        {
            processPoly(args);
        }
        else
        {
            processMono(args);
        }
    }
    void processMono(const typename rack::Module::ProcessArgs &args)
    {
        static constexpr float scaleFac{FXConfig<fxType>::rescaleInputFactor()},
            unscaleFac{1.0f / scaleFac};
        float inl = inputs[INPUT_L].getVoltageSum() * RACK_TO_SURGE_OSC_MUL * scaleFac;
        float inr = inputs[INPUT_R].getVoltageSum() * RACK_TO_SURGE_OSC_MUL * scaleFac;

        outputs[OUTPUT_L].setChannels(1);
        outputs[OUTPUT_R].setChannels(1);

        for (int i = 0; i < FXConfig<fxType>::extraOutputs(); ++i)
            outputs[EXTRA_OUTPUT_0 + i].setChannels(1);

        if (inputs[INPUT_L].isConnected() && !inputs[INPUT_R].isConnected())
        {
            bufferL[0][bufferPos] = inl;
            bufferR[0][bufferPos] = inl;
        }
        else
        {
            bufferL[0][bufferPos] = inl;
            bufferR[0][bufferPos] = inr;
        }

        if constexpr (FXConfig<fxType>::usesSideband())
        {
            if (inputs[SIDEBAND_L].isConnected() && !inputs[SIDEBAND_R].isConnected())
            {
                float ml = inputs[SIDEBAND_L].getVoltageSum();
                modulatorL[0][bufferPos] = ml;
                modulatorR[0][bufferPos] = ml;
            }
            else
            {
                modulatorL[0][bufferPos] = inputs[SIDEBAND_L].getVoltageSum();
                modulatorR[0][bufferPos] = inputs[SIDEBAND_R].getVoltageSum();
            }
        }
        bufferPos++;

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

            for (int i = 0; i < FXConfig<fxType>::numParams(); ++i)
            {
                fxstorage->p[i].set_value_f01(modAssist.basevalues[i]);
            }

            FXConfig<fxType>::processExtraInputs(this);

            copyGlobaldataSubset(storage_id_start, storage_id_end);

            auto *oap = &fxstorage->p[0];
            auto *eap = &fxstorage->p[FXConfig<fxType>::numParams() - 1];
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

            surge_effect->process_ringout(processedL[0], processedR[0], true);

            FXConfig<fxType>::populateExtraOutputs(this, 0, surge_effect.get());

            bufferPos = 0;
        }

        float outl = processedL[0][bufferPos] * unscaleFac;
        float outr = processedR[0][bufferPos] * unscaleFac;

        if constexpr (FXConfig<fxType>::softclipOutput())
        {
            // FIXME we can do this simd-wise of course
            outl = std::clamp(outl, -1.5f, 1.5f);
            outr = std::clamp(outr, -1.5f, 1.5f);
            outl = outl - 4.0 / 27.0 * outl * outl * outl;
            outr = outr - 4.0 / 27.0 * outr * outr * outr;
        }
        outl *= SURGE_TO_RACK_OSC_MUL;
        outr *= SURGE_TO_RACK_OSC_MUL;
        if (outputs[OUTPUT_L].isConnected() && !outputs[OUTPUT_R].isConnected())
        {
            outputs[OUTPUT_L].setVoltage(0.5 * (outl + outr));
        }
        else
        {
            outputs[OUTPUT_L].setVoltage(outl);
            outputs[OUTPUT_R].setVoltage(outr);
        }

        for (int i = 0; i < FXConfig<fxType>::extraOutputs(); ++i)
        {
            outputs[EXTRA_OUTPUT_0 + i].setVoltage(extraOutputs[i][0][bufferPos]);
        }
    }

    int lastNChan{-1};

    void reinitialize()
    {
        surge_effect->init();
        for (const auto &s : surge_effect_poly)
            if (s)
                s->init();
    }

    void guaranteePolyFX(int chan)
    {
        for (int i = 0; i < chan; ++i)
        {
            if (!surge_effect_poly[i])
            {
                surge_effect_poly[i].reset(
                    spawn_effect(fxType, storage.get(), fxstorage, storage->getPatch().globaldata));
                surge_effect_poly[i]->init();
            }
        }
    }

    void processPoly(const typename rack::Module::ProcessArgs &args)
    {
        static constexpr float scaleFac{FXConfig<fxType>::rescaleInputFactor()},
            unscaleFac{1.0f / scaleFac};

        auto chan = std::max({1, inputs[INPUT_L].getChannels(), inputs[INPUT_R].getChannels()});

        if (chan != lastNChan)
        {
            lastNChan = chan;
            guaranteePolyFX(chan);
            reinitialize();
        }

        outputs[OUTPUT_L].setChannels(chan);
        outputs[OUTPUT_R].setChannels(chan);

        for (int i = 0; i < FXConfig<fxType>::extraOutputs(); ++i)
            outputs[EXTRA_OUTPUT_0 + i].setChannels(chan);

        for (int c = 0; c < chan; ++c)
        {
            float inl = inputs[INPUT_L].getVoltage(c) * RACK_TO_SURGE_OSC_MUL * scaleFac;
            float inr = inputs[INPUT_R].getVoltage(c) * RACK_TO_SURGE_OSC_MUL * scaleFac;

            if (inputs[INPUT_L].isConnected() && !inputs[INPUT_R].isConnected())
            {
                bufferL[c][bufferPos] = inl;
                bufferR[c][bufferPos] = inl;
            }
            else
            {
                bufferL[c][bufferPos] = inl;
                bufferR[c][bufferPos] = inr;
            }
        }

        // FIXME make this poly
        if constexpr (FXConfig<fxType>::usesSideband())
        {
            if (inputs[SIDEBAND_L].isConnected() && !inputs[SIDEBAND_R].isConnected())
            {
                float ml = inputs[SIDEBAND_L].getVoltageSum();
                modulatorL[0][bufferPos] = ml;
                modulatorR[0][bufferPos] = ml;
            }
            else
            {
                modulatorL[0][bufferPos] = inputs[SIDEBAND_L].getVoltageSum();
                modulatorR[0][bufferPos] = inputs[SIDEBAND_R].getVoltageSum();
            }
        }
        bufferPos++;

        if (bufferPos >= BLOCK_SIZE)
        {
            polyModAssist.setupMatrix(this);
            polyModAssist.updateValues(this);

            if constexpr (FXConfig<fxType>::specificParamCount() > 0)
            {
                FXConfig<fxType>::processSpecificParams(this);
            }

            for (int i = 0; i < FXConfig<fxType>::numParams(); ++i)
            {
                fxstorage->p[i].set_value_f01(polyModAssist.basevalues[i]);
            }

            FXConfig<fxType>::processExtraInputs(this);

            for (int c = 0; c < chan; ++c)
            {
                std::memcpy(processedL[c], bufferL[c], BLOCK_SIZE * sizeof(float));
                std::memcpy(processedR[c], bufferR[c], BLOCK_SIZE * sizeof(float));

                if constexpr (FXConfig<fxType>::usesSideband())
                {
                    std::memcpy(storage->audio_in_nonOS[0], modulatorL, BLOCK_SIZE * sizeof(float));
                    std::memcpy(storage->audio_in_nonOS[1], modulatorR, BLOCK_SIZE * sizeof(float));
                }

                copyGlobaldataSubset(storage_id_start, storage_id_end);

                auto *oap = &fxstorage->p[0];
                auto *eap = &fxstorage->p[FXConfig<fxType>::numParams() - 1];
                auto &pt = storage->getPatch().globaldata;
                int idx = 0;
                while (oap <= eap)
                {
                    if (oap->valtype == vt_float)
                    {
                        pt[oap->id].f += polyModAssist.modvalues[idx][c] * modScales[idx];
                    }
                    idx++;
                    oap++;
                }

                surge_effect_poly[c]->process_ringout(processedL[c], processedR[c], true);

                FXConfig<fxType>::populateExtraOutputs(this, c, surge_effect_poly[c].get());
            }
            bufferPos = 0;
        }

        bool mono = outputs[OUTPUT_L].isConnected() && !outputs[OUTPUT_R].isConnected();
        for (int c = 0; c < chan; ++c)
        {
            float outl = processedL[c][bufferPos] * unscaleFac;
            float outr = processedR[c][bufferPos] * unscaleFac;

            if constexpr (FXConfig<fxType>::softclipOutput())
            {
                // FIXME we can do this simd-wise of course
                outl = std::clamp(outl, -1.5f, 1.5f);
                outr = std::clamp(outr, -1.5f, 1.5f);
                outl = outl - 4.0 / 27.0 * outl * outl * outl;
                outr = outr - 4.0 / 27.0 * outr * outr * outr;
            }
            outl *= SURGE_TO_RACK_OSC_MUL;
            outr *= SURGE_TO_RACK_OSC_MUL;

            if (mono)
            {
                outputs[OUTPUT_L].setVoltage(0.5 * (outl + outr), c);
            }
            else
            {
                outputs[OUTPUT_L].setVoltage(outl, c);
                outputs[OUTPUT_R].setVoltage(outr, c);
            }

            for (int i = 0; i < FXConfig<fxType>::extraOutputs(); ++i)
            {
                outputs[EXTRA_OUTPUT_0 + i].setVoltage(extraOutputs[i][c][bufferPos], c);
            }
        }
    }

    int polyChannelCount()
    {
        if (polyphonicMode)
            return std::max(inputs[INPUT_L].getChannels(), 1);
        else
            return 1; // these arent' polyphonic fx
    }

    void activateTempoSync()
    {
        auto p = &fxstorage->p[0];
        auto pe = &fxstorage->p[n_fx_params - 1];
        while (p <= pe)
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
        while (p <= pe)
        {
            if (p->can_temposync())
                p->temposync = false;
            ++p;
        }
    }

    json_t *makeModuleSpecificJson() override
    {
        auto fx = json_object();
        if (FXConfig<fxType>::usesPresets())
        {
            if (loadedPreset >= 0)
            {
                json_object_set_new(fx, "loadedPreset", json_integer(loadedPreset));
                json_object_set_new(fx, "presetName",
                                    json_string(presets[loadedPreset].name.c_str()));
                json_object_set_new(fx, "presetIsDirty", json_boolean(presetIsDirty));
            }
        }
        if (FXConfig<fxType>::usesClock())
        {
            clockProc.toJson(fx);
        }

        if (FXConfig<fxType>::allowsPolyphony())
        {
            json_object_set_new(fx, "polyphonicMode", json_boolean(polyphonicMode));
        }
        return fx;
    }

    void readModuleSpecificJson(json_t *modJ) override
    {
        if (FXConfig<fxType>::usesPresets())
        {
            auto lp = json_object_get(modJ, "loadedPreset");
            auto pn = json_object_get(modJ, "presetName");
            auto pd = json_object_get(modJ, "presetIsDirty");
            if (lp && pn && pd)
            {
                auto lpc = json_integer_value(lp);
                auto pnc = std::string(json_string_value(pn));
                auto pdc = json_boolean_value(pd);
                if (lpc >= 0 && lpc < (int)presets.size() && presets[lpc].name == pnc)
                {
                    loadedPreset = lpc;
                    presetIsDirty = pdc;
                }
            }
        }
        if (FXConfig<fxType>::usesClock())
        {
            clockProc.fromJson(modJ);
        }

        if (FXConfig<fxType>::allowsPolyphony())
        {
            auto pm = json_object_get(modJ, "polyphonicMode");
            if (pm)
            {
                auto pmv = json_boolean_value(pm);
                polyphonicMode = pmv;
            }
        }
    }

    std::unique_ptr<Effect> surge_effect;
    std::array<std::unique_ptr<Effect>, MAX_POLY> surge_effect_poly;
    FxStorage *fxstorage{nullptr};
};
} // namespace sst::surgext_rack::fx
#endif
