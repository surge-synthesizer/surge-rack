#include "FX.hpp"
#include "FXConfig.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "SurgeXT.hpp"

namespace sst::surgext_rack::fx::ui
{
template <int fxType>
struct FXWidget : public widgets::XTModuleWidget, widgets::StandardWidthWithModulationConstants
{
    typedef FX<fxType> M;
    FXWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, n_fx_params> overlays;
    std::array<widgets::KnobN *, n_fx_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    /*
remove this
         * @baconpaul some figures that might help you with FX layout (more to come later)

AS you know, on the VCO's there was a 16mm vertical offset between knob rows.

On the FX, where there is no group title on a row, the offset will be the same. But if there is a
group title on a row, then the offset to the next row of regular sized knobs will be 20mm instead

We will work up from the bottom, so all the 2 lower rows of jacks/mods and the first row of knobs
will be in exactly the same place as on the VCOs, and we work the spacing up from there. In other
words, having group titles on a row adds 4mm to the vertical offset spiritlevel — Yesterday at 5:09
PM So: 16mm up to the next reg. knob row if current knob row has no group titles 20mm up to next
reg. knob row if current knob row has group titles Add an extra 1.5mm if next row up is medium knobs
Add an extra 3mm if next row up is large knobs

This should work for all FX - hope that makes sense!
     */
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
        module->loadPreset(id);
        currentPreset = &module->presets[id];
        forceDirty = true;
    }
    void onPresetJog(int dir /* +/- 1 */) override
    {
        if (!module)
            return;

        id += dir;
        if (id < 0)
            id = module->presets.size() - 1;
        if (id >= module->presets.size())
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
        if (module && currentPreset && checkPresetEvery == 0 && !module->presetIsDirty)
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

        if (module)
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
};

template <int fxType> FXWidget<fxType>::FXWidget(FXWidget<fxType>::M *module)
{
    setModule(module);

    for (auto &uk : underKnobs)
        uk = nullptr;

    for (auto &ob : overlays)
        for (auto &o : ob)
            o = nullptr;

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * FXConfig<fxType>::panelWidthInScrews(),
                         rack::app::RACK_GRID_HEIGHT);

    auto panelLabel = std::string(fx_type_names[fxType]);
    for (auto &q : panelLabel)
        q = std::toupper(q);

    auto bg = new widgets::Background(box.size, panelLabel, "fx", "BlankNoDisplay");
    addChild(bg);

    for (const auto &lay : FXConfig<fxType>::getLayout())
    {
        switch (lay.type)
        {
        case FXConfig<fxType>::LayoutItem::KNOB9:
        case FXConfig<fxType>::LayoutItem::KNOB12:
        case FXConfig<fxType>::LayoutItem::KNOB16:
        {
            widgets::KnobN *knob{nullptr};
            int diff = lay.type - FXConfig<fxType>::LayoutItem::KNOB9;
            auto pos = rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm));
            float halfSize{0}; // diff in radius from 4.5
            auto par = M::FX_PARAM_0 + lay.parId;
            if (diff == 0)
            {
                knob = rack::createParamCentered<widgets::Knob9>(pos, module, par);
                halfSize = 0;
            }
            if (diff == 1)
            {
                knob = rack::createParamCentered<widgets::Knob12>(pos, module, par);
                halfSize = 1.5;
            }
            if (diff == 2)
            {
                knob = rack::createParamCentered<widgets::Knob16>(pos, module, par);
                halfSize = 3.5;
            }
            if (knob)
            {
                addChild(knob);

                auto boxx0 = lay.xcmm - columnWidth_MM * 0.5;
                auto boxy0 = lay.ycmm + 8.573 + halfSize - 5;

                auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
                auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));
                auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);
                addChild(lab);

                underKnobs[lay.parId] = knob;

                for (int m = 0; m < M::n_mod_inputs; ++m)
                {
                    auto radius = rack::mm2px(knob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
                    int id = M::modulatorIndexFor(lay.parId + M::FX_PARAM_0, m);
                    auto *k = widgets::ModRingKnob::createCentered(pos, radius, module, id);
                    overlays[lay.parId][m] = k;
                    k->setVisible(false);
                    k->underlyerParamWidget = knob;
                    knob->modRings.insert(k);
                    addChild(k);
                }
            }
        }
        break;
        case FXConfig<fxType>::LayoutItem::PORT:
        {
            auto port = rack::createInputCentered<widgets::Port>(
                rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm + verticalPortOffset_MM)), module,
                lay.parId);
            addChild(port);

            auto boxx0 = lay.xcmm - columnWidth_MM * 0.5;
            auto boxy0 = lay.ycmm + 8.573 - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));
            auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);

            addChild(lab);
        }
        break;
        case FXConfig<fxType>::LayoutItem::POWER_LIGHT:
        case FXConfig<fxType>::LayoutItem::EXTEND_LIGHT:
        {
            auto x = rack::mm2px(lay.xcmm + lay.spanmm * 5.5);
            auto y = rack::mm2px(lay.ycmm - 5.5);

            auto light = rack::createParamCentered<widgets::ActivateKnobSwitch>(rack::Vec(x, y),
                                                                                module, lay.parId);

            if (lay.type == FXConfig<fxType>::LayoutItem::EXTEND_LIGHT)
            {
                light->type = widgets::ActivateKnobSwitch::EXTENDED;
            }
            addChild(light);
        }
        break;
        case FXConfig<fxType>::LayoutItem::GROUP_LABEL:
        {
            auto gl = widgets::GroupLabel::createAboveCenterWithColSpan(
                lay.label, rack::Vec(lay.xcmm, lay.ycmm), lay.spanmm);
            if (lay.extras.find("SHORTLEFT") != lay.extras.end())
                gl->shortLeft = true;
            if (lay.extras.find("SHORTRIGHT") != lay.extras.end())
                gl->shortRight = true;
            addChild(gl);
        }
        break;
        case FXConfig<fxType>::LayoutItem::LCD_BG:
        {
            auto bg = widgets::LCDBackground::createWithHeight(lay.ycmm);
            if (!module)
                bg->noModuleText = panelLabel;
            addChild(bg);
        }
        break;
        case FXConfig<fxType>::LayoutItem::LCD_MENU_ITEM:
        {
            auto xpos = rack::mm2px(widgets::LCDBackground::contentPosX_MM);
            auto width = box.size.x - 2 * rack::mm2px(widgets::LCDBackground::contentPosX_MM);
            auto height = rack::mm2px(5);
            auto ypos = rack::mm2px(lay.ycmm - widgets::LCDBackground::padY_MM) - height;
            auto wid = widgets::PlotAreaMenuItem::create(
                rack::Vec(xpos, ypos), rack::Vec(width, height), module, lay.parId);
            wid->upcaseDisplay = false;
            wid->transformLabel = [n = lay.label](auto s) {
                if (n.empty())
                    return s;
                return n + ": " + s;
            };
            wid->onShowMenu = [this, wid, lay]() {
                if (!this->module)
                    return;

                auto *fxm = static_cast<FX<fxType> *>(this->module);

                auto pq = wid->getParamQuantity();
                if (!pq)
                    return;

                auto &surgePar = fxm->fxstorage->p[lay.parId - FX<fxType>::FX_PARAM_0];
                if (!(surgePar.valtype == vt_int))
                    return;

                auto men = rack::createMenu();
                men->addChild(rack::createMenuLabel(pq->getLabel()));

                for (int i = surgePar.val_min.i; i <= surgePar.val_max.i; i++)
                {
                    char txt[256];
                    auto fv =
                        Parameter::intScaledToFloat(i, surgePar.val_max.i, surgePar.val_min.i);
                    surgePar.get_display(txt, true, fv);
                    men->addChild(rack::createMenuItem(txt, "", [pq, fv]() { pq->setValue(fv); }));
                }
            };
            addParam(wid);
        }
        break;
        default:
            break;
        }
    }

    auto wts = FXPresetSelector<fxType>::create(module);
    addChild(wts);
    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        addChild(makeModLabel(i));
    }

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto uxp = columnCenters_MM[i];
        auto uyp = modulationRowCenters_MM[0];

        auto *k =
            rack::createWidgetCentered<widgets::ModToggleButton>(rack::mm2px(rack::Vec(uxp, uyp)));
        toggles[i] = k;
        k->onToggle = [this, toggleIdx = i](bool isOn) {
            for (const auto &t : toggles)
                if (t)
                    t->setState(false);
            for (const auto &ob : overlays)
                for (const auto &o : ob)
                    if (o)
                        o->setVisible(false);
            if (isOn)
            {
                toggles[toggleIdx]->setState(true);
                for (const auto &ob : overlays)
                    if (ob[toggleIdx])
                    {
                        ob[toggleIdx]->setVisible(true);
                        ob[toggleIdx]->bdw->dirty = true;
                    }
                for (const auto &uk : underKnobs)
                    if (uk)
                        uk->setIsModEditing(true);
            }
            else
            {
                for (const auto &uk : underKnobs)
                    if (uk)
                        uk->setIsModEditing(false);
            }
        };

        addChild(k);
        uyp = modulationRowCenters_MM[1];
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module,
                                                          M::MOD_INPUT_0 + i));
    }

    int col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    col = 0;
    for (const std::string &s : {"LEFT", "RIGHT", "LEFT", "RIGHT"})
    {
        addChild(makeIORowLabel(col, s, col < 2));
        col++;
    }

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
// skip Nimbus - there's loads of clouds out there
// ski Tape - chow has that covered
// skip Treemonster - write a poly version instead
// skip Waveshaper - write a poly version instead
// skip MSTool - covered by vcv core
FXMODEL(fxt_spring_reverb, SpringReverb);
