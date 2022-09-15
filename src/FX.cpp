#include "FX.hpp"
#include "FXConfig.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "SurgeXT.hpp"

namespace sst::surgext_rack::fx::ui
{
template <int fxType> struct FXWidget : public widgets::XTModuleWidget, widgets::VCOVCFConstants
{
    typedef FX<fxType> M;
    FXWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, n_fx_params> overlays;
    std::array<widgets::KnobN *, n_fx_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

template <int fxType> FXWidget<fxType>::FXWidget(FXWidget<fxType>::M *module)
{
    setModule(module);
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * FXConfig<fxType>::panelWidthInScrews(),
                         rack::app::RACK_GRID_HEIGHT);

    auto panelLabel = std::string(fx_type_names[fxType]);
    for (auto &q : panelLabel)
        q = std::toupper(q);

    auto bg = new widgets::Background(box.size, panelLabel, "fx", "BlankNoDisplay");
    addChild(bg);

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto uxp = columnCenters_MM[i];
        auto uyp = rowCenters_MM[2];

        auto *k =
            rack::createWidgetCentered<widgets::ModToggleButton>(rack::mm2px(rack::Vec(uxp, uyp)));
        toggles[i] = k;
        k->onToggle = [this, toggleIdx = i](bool isOn) {
            std::cout << __FILE__ << ":" << __LINE__ << " SET UP THIS TOGGLE WITH THE IFDEF"
                      << std::endl;
#if 0
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
#endif
        };

        addChild(k);
        uyp = rowCenters_MM[3];
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module,
                                                          M::MOD_INPUT_0 + i));
    }

    int col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = rowCenters_MM[4];
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = rowCenters_MM[4];
        auto xp = columnCenters_MM[col];
        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    col = 0;
    for (const std::string &s : {"LEFT", "RIGHT", "LEFT", "RIGHT"})
    {
        addChild(makeLabel(
            3, col, s, (col < 2 ? style::XTStyle::TEXT_LABEL : style::XTStyle::TEXT_LABEL_OUTPUT)));
        col++;
    }
}
} // namespace sst::surgext_rack::fx::ui

namespace fxui = sst::surgext_rack::fx::ui;

#define FXMODEL(type, nm)                                                                          \
    rack::Model *modelFX##nm = rack::createModel<fxui::FXWidget<type>::M, fxui::FXWidget<type>>(   \
        std::string("SurgeXTFX") + #nm);

FXMODEL(fxt_reverb2, Reverb2);
FXMODEL(fxt_freqshift, FrequencyShifter);
FXMODEL(fxt_flanger, Flanger);
