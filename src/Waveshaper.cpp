//
// Created by Paul Walker on 8/29/22.
//

#include "Waveshaper.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "dsp/DSPExternalAdapterUtils.h"

namespace sst::surgext_rack::waveshaper::ui
{
struct WaveshaperWidget : widgets::XTModuleWidget, widgets::VCOVCFConstants
{
    typedef waveshaper::Waveshaper M;
    WaveshaperWidget(M *module);

    void moduleBackground(NVGcontext *vg) {}

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, 5> overlays;
    std::array<widgets::KnobN *, Waveshaper::n_wshp_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

struct WaveshaperSelector : widgets::ParamJogSelector
{
    WaveShaperSelectorMapper wsm;
    std::vector<int> wsmOrdering;

    WaveshaperSelector() { wsmOrdering = wsm.totalIndexOrdering(); }
    static WaveshaperSelector *create(const rack::Vec &pos, const rack::Vec &size,
                                      Waveshaper *module, int paramId)
    {
        auto res = new WaveshaperSelector();
        res->box.pos = pos;
        res->box.size = size;
        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();
        res->setup();

        return res;
    }

    void onPresetJog(int dir) override
    {
        if (!getParamQuantity())
            return;

        int type = (int)std::round(getParamQuantity()->getValue());
        auto di = wsm.remapStreamedIndexToDisplayIndex(type);
        di += dir;
        if (di >= (int)wsmOrdering.size())
            di = 0;
        if (di < 0)
            di = (int)wsmOrdering.size() - 1;
        setType(wsmOrdering[di]);
    }

    rack::ui::Menu *menuForGroup(rack::ui::Menu *menu, const std::string &group)
    {
        for (const auto &[id, gn] : wsm.mapping)
        {
            if (gn == group)
            {
                menu->addChild(rack::createMenuItem(sst::waveshapers::wst_names[id], "",
                                                    [this, cid = id]() { setType(cid); }));
            }
        }
        return menu;
    }

    void onShowMenu() override
    {
        if (!module)
            return;
        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel("Waveshaper Types"));

        std::string currentGroup{"-not-a-ws-group-"};

        for (const auto &[id, gn] : wsm.mapping)
        {
            if (gn == "")
            {
                menu->addChild(rack::createMenuItem(sst::filters::filter_type_names[id], "",
                                                    [this, cid = id]() { setType(cid); }));
            }
            else if (gn != currentGroup)
            {
                menu->addChild(rack::createSubmenuItem(
                    gn, "", [this, cgn = gn](auto *x) { menuForGroup(x, cgn); }));
                currentGroup = gn;
            }
        }
    }
    bool forceDirty{false};
    void setType(int id)
    {
        forceDirty = true;
        if (!module)
            return;
        if (!getParamQuantity())
            return;
        getParamQuantity()->setValue(id);
    }

    bool isDirty() override
    {
        if (forceDirty)
        {
            forceDirty = false;
            return true;
        }
        return false;
    }
    std::string getPresetName() override
    {
        if (!getParamQuantity())
            return "Waveshaper";
        int type = (int)std::round(getParamQuantity()->getValue());
        return sst::waveshapers::wst_names[type];
    }
};

WaveshaperWidget::WaveshaperWidget(WaveshaperWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "WAVESHAPER", "vco", "BlankVCO");
    addChild(bg);

    int idx = 0;

    auto fivemm = rack::mm2px(5);
    auto halfmm = rack::mm2px(0.5);
    auto wts = WaveshaperSelector::create(rack::Vec(plotStartX, plotStartY),
                                          rack::Vec(plotW, fivemm - halfmm), module,
                                          Waveshaper::WSHP_TYPE);
    addChild(wts);
#if 0
    plotStartY += fivemm;
    plotH -= fivemm;

    auto fpw = FilterPlotWidget::create(rack::Vec(plotStartX, plotStartY), rack::Vec(plotW, plotH),
                                        module);
    addChild(fpw);

    auto subType = VCFSubtypeSelector::create(rack::Vec(plotStartX, underPlotStartY),
                                              rack::Vec(plotW, underPlotH), module, M::VCF_SUBTYPE);
    addChild(subType);
#endif

    for (const auto &[row, col, pid, label] :
         {std::make_tuple(0, 0, M::DRIVE, "DRIVE"), std::make_tuple(0, 2, M::BIAS, "BIAS"),
          std::make_tuple(0, 3, M::OUT_GAIN, "GAIN"), std::make_tuple(1, 2, M::LOCUT, "LOCUT"),
          std::make_tuple(1, 3, M::HICUT, "HICUT")})
    {
        auto uxp = columnCenters_MM[col];
        auto uyp = rowCenters_MM[row];

        widgets::KnobN *baseKnob{nullptr};
        if (row == 0 && col == 0)
        {
            uxp = (columnCenters_MM[0] + columnCenters_MM[1]) * 0.5f;
            uyp = (rowCenters_MM[0] + rowCenters_MM[1]) * 0.5f;

            auto boxx0 = uxp - columnWidth_MM;
            auto boxy0 = uyp + 8;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(columnWidth_MM * 2, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, label);
            addChild(lab);
            baseKnob = rack::createParamCentered<widgets::Knob16>(rack::mm2px(rack::Vec(uxp, uyp)),
                                                                  module, pid);
        }
        else
        {
            addChild(makeLabel(row, col, label));
            baseKnob = rack::createParamCentered<widgets::Knob9>(rack::mm2px(rack::Vec(uxp, uyp)),
                                                                 module, pid);
        }
        addParam(baseKnob);
        underKnobs[idx] = baseKnob;
        for (int m = 0; m < M::n_mod_inputs; ++m)
        {
            auto radius = rack::mm2px(baseKnob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
            int id = M::modulatorIndexFor(pid, m);
            auto *k = widgets::ModRingKnob::createCentered(rack::mm2px(rack::Vec(uxp, uyp)), radius,
                                                           module, id);
            overlays[idx][m] = k;
            k->setVisible(false);
            k->underlyerParamWidget = baseKnob;
            baseKnob->modRings.insert(k);
            addChild(k);
        }

        idx++;
    }

    int row{0}, col{0};
    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        addChild(makeModLabel(i));
    }

    col = 0;
    row = 3;

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto uxp = columnCenters_MM[i];
        auto uyp = rowCenters_MM[2];

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

        uyp = rowCenters_MM[3];

        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module,
                                                          M::WSHP_MOD_INPUT + i));
    }

    col = 0;
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
        addChild(makeIORowLabel(col, s, col < 2));
        col++;
    }

    coupleToGlobalStyle(true, module);
}
} // namespace sst::surgext_rack::waveshaper::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeWaveshaper =
    rack::createModel<sst::surgext_rack::waveshaper::ui::WaveshaperWidget::M,
                      sst::surgext_rack::waveshaper::ui::WaveshaperWidget>("SurgeXTWaveshaper");
