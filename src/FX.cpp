/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
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

#include "FX.h"
#include "FXConfig.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "SurgeXT.h"

namespace sst::surgext_rack::fx::ui
{
template <int fxType> struct FXWidget : public widgets::XTModuleWidget
{
    typedef FX<fxType> M;
    FXWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, n_fx_params> overlays;
    std::array<widgets::ModulatableKnob *, n_fx_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
        auto xtm = static_cast<FX<fxType> *>(module);

        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(
            rack::createMenuItem("Re-Initialize Effect", "", [xtm] { xtm->reinitialize(); }));

        if constexpr (FXConfig<fxType>::allowsPolyphony())
        {
            menu->addChild(new rack::ui::MenuSeparator);
            bool t = xtm->polyphonicMode;
            menu->addChild(rack::createMenuItem("Monophonic Stereo Processing", CHECKMARK(!t),
                                                [xtm] { xtm->polyphonicMode = false; }));

            menu->addChild(rack::createMenuItem("Polyphonic Stereo Processing", CHECKMARK(t),
                                                [xtm] { xtm->polyphonicMode = true; }));
        }

        if (FXConfig<fxType>::usesClock())
        {
            addClockMenu<FX<fxType>>(menu);
        }
    }
};

template <int fxType> struct FXPresetSelector : widgets::PresetJogSelector
{
    FX<fxType> *module{nullptr};
    Surge::Storage::FxUserPreset::Preset *currentPreset{nullptr};
    static FXPresetSelector *create(FX<fxType> *module)
    {
        auto res = new FXPresetSelector<fxType>();
        res->box.pos.x = rack::mm2px(widgets::LCDBackground::contentPosX_MM);
        res->box.pos.y = rack::mm2px(widgets::LCDBackground::contentPosY_MM);
        res->box.size.x =
            rack::RACK_GRID_WIDTH * 12 - 2 * rack::mm2px(widgets::LCDBackground::contentPosX_MM);
        res->box.size.y = rack::mm2px(4.5);

        res->module = module;
        res->setup();

        if (module && res->module->loadedPreset >= 0)
        {
            res->currentPreset = &res->module->presets[res->module->loadedPreset];
            res->id = res->module->loadedPreset;
        }

        return res;
    }

    int id{0};
    void setValue(int i)
    {
        id = i;
        if (!module)
            return;
        if (module->presets.empty())
            return;

        module->loadPreset(id);
        currentPreset = &module->presets[id];
        forceDirty = true;
    }
    void onPresetJog(int dir /* +/- 1 */) override
    {
        if (!module)
            return;
        if (module->presets.empty())
            return;

        id += dir;
        if (id < 0)
            id = module->presets.size() - 1;
        if (id >= (int)module->presets.size())
            id = 0;
        forceDirty = true;
        setValue(id);
    };
    void onShowMenu() override
    {
        if (!module)
            return;
        auto menu = rack::createMenu();
        auto psn = std::string(fx_type_names[fxType]) + " Presets";
        menu->addChild(rack::createMenuLabel(psn));

        int idx{0};
        for (const auto &p : module->presets)
        {
            menu->addChild(rack::createMenuItem(p.name, "", [this, idx]() { setValue(idx); }));
            idx++;
        }
    }
    std::string getPresetName() override
    {
        if (!module)
            return "";
        if (module->presets.empty())
            return "";
        if (module->maxPresets == 0)
            return "";
        if (module->maxPresets <= id || id < 0)
            return "Software Error";

        auto res = module->presets[id].name;
        if (module->presetIsDirty)
            res += "*";
        return res;
    }

    int checkPresetEvery{0};
    bool forceDirty{true};
    bool isDirty() override
    {
        if (module && !module->presets.empty() && currentPreset && checkPresetEvery == 0 &&
            !module->presetIsDirty)
        {
            for (int i = 0; i < n_fx_params; ++i)
            {
                if (module->fxstorage->p[i].ctrltype == ct_none)
                    continue;

                auto rabs = fabs(module->paramQuantities[FX<fxType>::FX_PARAM_0 + i]->getValue() -
                                 module->value01for(i, currentPreset->p[i]));

                if (rabs > 1e-5)
                {
                    if (!module->presetIsDirty)
                    {
                        module->presetIsDirty = true;
                        forceDirty = true;
                    }
                }
            }

            if (FXConfig<fxType>::isDirtyPresetVsSpecificParams(module, *currentPreset))
            {
                module->presetIsDirty = true;
                forceDirty = true;
            }
        }
        checkPresetEvery++;
        if (checkPresetEvery >= 8)
            checkPresetEvery = 0;

        if (module && !module->presets.empty())
        {
            if (module->loadedPreset >= 0 && module->loadedPreset != id)
            {
                id = module->loadedPreset;
                return true;
            }
        }

        if (forceDirty)
        {
            forceDirty = false;
            return true;
        }

        return false;
    }

    bool hasPresets() override
    {
        if (module)
            return !module->presets.empty();
        return module != nullptr;
    }
};

template <int fxType> FXWidget<fxType>::FXWidget(FXWidget<fxType>::M *module)
{
    setModule(module);
    typedef layout::LayoutEngine<FXWidget<fxType>, M::FX_PARAM_0, M::INPUT_CLOCK> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * FXConfig<fxType>::panelWidthInScrews(),
                         rack::app::RACK_GRID_HEIGHT);

    auto panelLabel = std::string(fx_type_names[fxType]);
    for (auto &q : panelLabel)
        q = std::toupper(q);

    auto bg = new widgets::Background(box.size, panelLabel, "fx", "BlankNoDisplay");
    addChild(bg);

    for (const auto &lay : FXConfig<fxType>::getLayout())
    {
        engine_t::layoutItem(this, lay, panelLabel);
    }

    // Sometimes you just don't need to generalize
    if constexpr (fxType == fxt_reverb)
    {
        auto xc = box.size.x / 2;
        auto yc = rack::mm2px(fx::FXLayoutHelper::rowStart_MM - fx::FXLayoutHelper::labeledGap_MM -
                              fx::FXLayoutHelper::knobGap16_MM);
        auto fl =
            widgets::ThereAreFourLights<>::createCentered({xc, yc}, module, M::FX_PARAM_0 + 1);
        addParam(fl);

        auto halfSize = (14 - 9) * 0.5f;
        auto boxx0 = xc - rack::mm2px(layout::LayoutConstants::columnWidth_MM * 0.5 + halfSize);
        auto boxy0 = yc + rack::mm2px(8.573 + halfSize - 5);

        auto p0 = rack::Vec(boxx0, boxy0);
        auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::columnWidth_MM + halfSize * 2, 5));
        auto lab = widgets::Label::createWithBaselineBox(p0, s0, "SHAPE");
        addChild(lab);
    }

    auto wts = FXPresetSelector<fxType>::create(module);
    addChild(wts);

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0);

    engine_t::createLeftRightInputLabels(this);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::fx::ui

namespace fxui = sst::surgext_rack::fx::ui;

#define FXMODEL(type, nm)                                                                          \
    rack::Model *modelFX##nm = rack::createModel<fxui::FXWidget<type>::M, fxui::FXWidget<type>>(   \
        std::string("SurgeXTFX") + #nm);

FXMODEL(fxt_reverb, Reverb);
FXMODEL(fxt_phaser, Phaser);
FXMODEL(fxt_rotaryspeaker, RotarySpeaker);
FXMODEL(fxt_distortion, Distortion);
// skip EQ. better covered elsewhere
FXMODEL(fxt_freqshift, FrequencyShifter);
// skip Conditioner. It's not that good! And there's other compressors
FXMODEL(fxt_chorus4, Chorus);
FXMODEL(fxt_vocoder, Vocoder);
FXMODEL(fxt_reverb2, Reverb2);
FXMODEL(fxt_flanger, Flanger);
FXMODEL(fxt_ringmod, RingMod);
// skip Airwindows. Maybe BP will do a mega-airwindows collection
FXMODEL(fxt_neuron, Neuron);
FXMODEL(fxt_resonator, Resonator);
FXMODEL(fxt_chow, Chow);
FXMODEL(fxt_exciter, Exciter);
FXMODEL(fxt_ensemble, Ensemble);
FXMODEL(fxt_combulator, Combulator);
FXMODEL(fxt_nimbus, Nimbus);
// skip Nimbus - there's loads of clouds out there
// ski Tape - chow has that covered
// skip Waveshaper - write a poly version instead
// skip MSTool - covered by vcv core
FXMODEL(fxt_spring_reverb, SpringReverb);
FXMODEL(fxt_treemonster, TreeMonster);
FXMODEL(fxt_bonsai, Bonsai);
