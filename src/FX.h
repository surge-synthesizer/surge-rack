/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_SRC_FX_H
#define SURGE_XT_RACK_SRC_FX_H

#include <cmath>
#include "SurgeXT.h"
#include "dsp/Effect.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>

#include "DebugHelpers.h"
#include "FxPresetAndClipboardManager.h"

#include "LayoutEngine.h"
#include "sst/rackhelpers/neighbor_connectable.h"
#include "sst/filters/HalfRateFilter.h"

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
    static void processExtraInputs(FX<fxType> *M, int channel) {}

    static constexpr int extraOutputs() { return 0; }
    static void configExtraOutputs(FX<fxType> *M) {}
    static void populateExtraOutputs(FX<fxType> *M, int chan, Effect *fx) {}

    static constexpr int specificParamCount() { return 0; }
    static void configSpecificParams(FX<fxType> *M) {}
    static void processSpecificParams(FX<fxType> *M) {}
    static void adjustParamsBasedOnState(FX<fxType> *M) {}
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
    static constexpr bool usesSidebandOversampled() { return false; }
    static constexpr bool usesClock() { return false; }
    static constexpr bool usesPresets() { return true; }
    static constexpr int numParams() { return n_fx_params; }
    static constexpr bool allowsPolyphony() { return true; }

    static constexpr float rescaleInputFactor() { return 1.0; }
    static constexpr bool softclipOutput() { return false; }
    static constexpr bool nanCheckOutput() { return false; }

    static void addFXSpecificMenuItems(FX<fxType> *M, rack::ui::Menu *) {}
};

template <int fxType>
struct FX : modules::XTModule, sst::rackhelpers::module_connector::NeighborConnectable_V1
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

    FX() : XTModule(), halfbandIN(6, true)
    {
        std::lock_guard<std::mutex> lgxt(xtSurgeCreateMutex);
        setupSurge();
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (auto &t : extraInputTriggers)
            t.state = false;

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

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        if constexpr (FXConfig<fxType>::usesSideband() ||
                      FXConfig<fxType>::usesSidebandOversampled())
        {
            return {{std::make_pair("Input", std::make_pair(INPUT_L, INPUT_R)),
                     std::make_pair("SideBand", std::make_pair(SIDEBAND_L, SIDEBAND_R))}};
        }
        else
        {
            return {{std::make_pair("Input", std::make_pair(INPUT_L, INPUT_R))}};
        }
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{std::make_pair("Output", std::make_pair(OUTPUT_L, OUTPUT_R))}};
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

    sst::filters::HalfRate::HalfRateFilter halfbandIN;
    std::atomic<bool> sidebandAttached{false};

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
    uint32_t lastNanCheck{0};
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
                float ml = inputs[SIDEBAND_L].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
                modulatorL[0][bufferPos] = ml;
                modulatorR[0][bufferPos] = ml;
            }
            else
            {
                modulatorL[0][bufferPos] =
                    inputs[SIDEBAND_L].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
                modulatorR[0][bufferPos] =
                    inputs[SIDEBAND_R].getVoltageSum() * RACK_TO_SURGE_OSC_MUL;
            }
            bool wasSB = sidebandAttached;
            sidebandAttached = inputs[SIDEBAND_L].isConnected() || inputs[SIDEBAND_R].isConnected();
            if (FXConfig<fxType>::usesSidebandOversampled())
            {
                if (sidebandAttached && !wasSB)
                {
                    halfbandIN.reset();
                }
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
                if (FXConfig<fxType>::usesSidebandOversampled())
                {
                    halfbandIN.process_block_U2(modulatorL[0], modulatorR[0], storage->audio_in[0],
                                                storage->audio_in[1], BLOCK_SIZE_OS);
                }
            }
            if constexpr (FXConfig<fxType>::specificParamCount() > 0)
            {
                FXConfig<fxType>::processSpecificParams(this);
            }

            for (int i = 0; i < FXConfig<fxType>::numParams(); ++i)
            {
                fxstorage->p[i].set_value_f01(modAssist.basevalues[i]);
            }

            FXConfig<fxType>::processExtraInputs(this, 0);
            FXConfig<fxType>::adjustParamsBasedOnState(this);

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

            if constexpr (FXConfig<fxType>::nanCheckOutput())
            {
                if (lastNanCheck == 0)
                {
                    bool isNumber{true};
                    for (int ns = 0; ns < BLOCK_SIZE; ++ns)
                    {
                        isNumber = isNumber && std::isfinite(processedL[0][ns]);
                        isNumber = isNumber && std::isfinite(processedR[0][ns]);
                    }

                    if (!isNumber)
                    {
                        reinitialize();
                    }
                }
                lastNanCheck = (lastNanCheck + 1) % 32;
            }

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

    void reinitialize(int c = -1)
    {
        if (c == -1)
        {
            // Re-initialize everything
            surge_effect->init();
            halfbandIN.reset();
            for (const auto &s : surge_effect_poly)
                if (s)
                {
                    s->init();
                }

            // We are just starting over so clear all the buffers
            bufferPos = 0;

            memset(processedL, 0, sizeof(float) * MAX_POLY * BLOCK_SIZE);
            memset(processedR, 0, sizeof(float) * MAX_POLY * BLOCK_SIZE);
            memset(bufferL, 0, sizeof(float) * MAX_POLY * BLOCK_SIZE);
            memset(bufferR, 0, sizeof(float) * MAX_POLY * BLOCK_SIZE);
        }
        else
        {
            // poly nan case
            surge_effect_poly[c]->init();

            // Other buffers are fine. Just clear mine. And don't change
            // pos since the zeros wont hurt me.
            memset(processedL[c], 0, sizeof(float) * BLOCK_SIZE);
            memset(processedR[c], 0, sizeof(float) * BLOCK_SIZE);
            memset(bufferL[c], 0, sizeof(float) * BLOCK_SIZE);
            memset(bufferR[c], 0, sizeof(float) * BLOCK_SIZE);
        }
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

            for (int c = 0; c < chan; ++c)
            {
                FXConfig<fxType>::processExtraInputs(this, c);

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

            if constexpr (FXConfig<fxType>::nanCheckOutput())
            {
                if (lastNanCheck == 0)
                {
                    for (int c = 0; c < chan; ++c)
                    {

                        bool isNumber{true};
                        for (int ns = 0; ns < BLOCK_SIZE; ++ns)
                        {
                            isNumber = isNumber && std::isfinite(processedL[c][ns]);
                            isNumber = isNumber && std::isfinite(processedR[c][ns]);
                        }

                        if (!isNumber)
                        {
                            reinitialize(c);
                        }
                    }
                }
                lastNanCheck = (lastNanCheck + 1) % 32;
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

        // A little bit of defensive code I added in 2.2 in case we change int bounds in the
        // future. I don't read this yet but I do write it
        auto *paramNatural = json_array();
        for (int i = 0; i < n_fx_params; ++i)
        {
            const auto &p = fxstorage->p[i];
            auto *parJ = json_object();

            json_object_set(parJ, "index", json_integer(i));
            json_object_set(parJ, "valtype", json_integer(p.valtype));
            switch (p.valtype)
            {
            case vt_float:
                json_object_set(parJ, "val_f", json_real(p.val.f));
                break;
            case vt_int:
                json_object_set(parJ, "val_i", json_integer(p.val.i));
                break;
            case vt_bool:
                json_object_set(parJ, "val_b", json_boolean(p.val.b));
                break;
            }
            json_array_append_new(paramNatural, parJ);
        }

        json_object_set_new(fx, "paramNatural", paramNatural);

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
