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

    /*
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
            auto par = M::FX_PARAM_0 + lay.parId;
            if (diff == 0)
                knob = rack::createParamCentered<widgets::Knob9>(pos, module, par);
            if (diff == 1)
                knob = rack::createParamCentered<widgets::Knob12>(pos, module, par);
            if (diff == 2)
                knob = rack::createParamCentered<widgets::Knob16>(pos, module, par);
            if (knob)
            {
                addChild(knob);
                auto boxbl = rack::Vec(rack::mm2px(lay.xcmm - columnWidth_MM * 0.5),
                                       knob->box.pos.y + knob->box.size.y);
                auto lab = widgets::Label::createWithBaselineBox(
                    boxbl, rack::mm2px(rack::Vec(columnWidth_MM, 4)), lay.label);
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
            auto boxbl = rack::Vec(rack::mm2px(lay.xcmm - columnWidth_MM * 0.5),
                                   port->box.pos.y + port->box.size.y +
                                       rack::mm2px(2 * verticalPortOffset_MM));
            auto lab = widgets::Label::createWithBaselineBox(
                boxbl, rack::mm2px(rack::Vec(columnWidth_MM, 4)), lay.label);
            addChild(lab);
        }
        break;
        case FXConfig<fxType>::LayoutItem::LCD_BG:
        {
            auto bg = widgets::LCDBackground::createWithHeight(lay.ycmm);
            if (!module)
                bg->noModuleText = panelLabel;
            addChild(bg);
        }
        default:
            break;
        }
    }

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
}
} // namespace sst::surgext_rack::fx::ui

namespace fxui = sst::surgext_rack::fx::ui;

#define FXMODEL(type, nm)                                                                          \
    rack::Model *modelFX##nm = rack::createModel<fxui::FXWidget<type>::M, fxui::FXWidget<type>>(   \
        std::string("SurgeXTFX") + #nm);

FXMODEL(fxt_reverb, Reverb);
FXMODEL(fxt_reverb2, Reverb2);
FXMODEL(fxt_freqshift, FrequencyShifter);
FXMODEL(fxt_flanger, Flanger);
FXMODEL(fxt_delay, Delay);
FXMODEL(fxt_spring_reverb, SpringReverb);
