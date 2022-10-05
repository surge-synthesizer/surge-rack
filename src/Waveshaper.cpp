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

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }
};

struct WaveshaperPlotWidget : public rack::widget::TransparentWidget, style::StyleParticipant
{
    typename WaveshaperWidget::M *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwPlot{nullptr};

    std::vector<std::pair<float, float>> inputSignal;
    std::vector<std::pair<float, float>> outputSignal;
    void setup(typename WaveshaperWidget::M *m)
    {
        module = m;
        if (module)
        {
            // storage = module->storage.get();
        }
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto *vg) { drawPlotBackground(vg); });
        addChild(bdw);

        bdwPlot = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto *vg) { drawPlot(vg); });
        addChild(bdwPlot);

        auto fac = 2.0;
        auto inputRes = (int)box.size.x * fac;
        auto dx = 1.0 / inputRes;
        for (int i = 0; i < inputRes; ++i)
        {
            auto x = dx * i;
            auto y = std::sin(x * 4.0 * M_PI);
            inputSignal.emplace_back(x * box.size.x, y);
        }
    }

    void step() override
    {
        if (!module)
            return;

        /*
         * if wavetable changed and draw wavetable bdw->dirty = true
         */

        if (isDirty())
        {
            recalcPath();
            bdwPlot->dirty = true;
        }

        rack::widget::Widget::step();
    }

    virtual void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwPlot->dirty = true;
    }

    static WaveshaperPlotWidget *create(rack::Vec pos, rack::Vec size,
                                        typename WaveshaperWidget::M *module)
    {
        auto *res = rack::createWidget<WaveshaperPlotWidget>(pos);

        res->box.pos = pos;
        res->box.size = size;

        res->setup(module);

        return res;
    }

    bool firstDirty{false};
    int dirtyCount{0};
    int sumDeact{-1};
    int sumAbs{-1};
    uint32_t wtloadCompare{842932918};

    bool isDirty()
    {
        if (!firstDirty)
        {
            firstDirty = true;
            return true;
        }

        bool dval{false};
        if (module)
        {
            auto wstype = (sst::waveshapers::WaveshaperType)
                std::round(module->paramQuantities[Waveshaper::WSHP_TYPE]->getValue());

            auto ddb = module->modulationAssistant.values[Waveshaper::DRIVE][0];
            auto bias = module->modulationAssistant.values[Waveshaper::BIAS][0];

            dval = wstype != lastType ||
                ddb != lastDrive ||
                bias != lastBias;
        }
        return dval;
    }

    sst::waveshapers::WaveshaperType lastType{waveshapers::WaveshaperType::wst_none};
    float lastDrive{-100}, lastBias{-100};

    void recalcPath() {
        if (!module)
            return;

        outputSignal.clear();
        auto wstype = (sst::waveshapers::WaveshaperType)
            std::round(module->paramQuantities[Waveshaper::WSHP_TYPE]->getValue());
        sst::waveshapers::QuadWaveshaperState wss;
        float R[4];

        initializeWaveshaperRegister(wstype, R);

        for (int i = 0; i < sst::waveshapers::n_waveshaper_registers; ++i)
        {
            wss.R[i] = _mm_set1_ps(R[i]);
        }

        wss.init = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()); // better way?

        auto ddb = module->modulationAssistant.values[Waveshaper::DRIVE][0];
        auto bias = module->modulationAssistant.values[Waveshaper::BIAS][0];

        auto wsop = sst::waveshapers::GetQuadWaveshaper(wstype);
        auto damp = pow(10, 0.05 * ddb);
        auto d1 = _mm_set1_ps(damp);

        lastType = wstype;
        lastBias = bias;
        lastDrive = ddb;

        for (const auto &[x,y] : inputSignal)
        {
            auto ivs = _mm_set1_ps(y + bias);
            auto ov1 = ivs;

            if (wsop)
            {
                ov1 = wsop(&wss, ivs, d1);
            }

            float r alignas(16)[8];
            _mm_store_ps(r, ov1);

            outputSignal.emplace_back(x, r[0]);
        }
    }

    const float xs3d{rack::mm2px(5)};
    const float ys3d{rack::mm2px(3.5)};

    float y1toypx(float y1)
    {
        auto y2 = (1 - y1) * 0.5;
        auto ypx = y2 * box.size.y;
        return ypx;
    }

    void drawPlotBackground(NVGcontext *vg)
    {
        // This will go in layer 0
        int nSteps = 9;
        int mid = (nSteps - 1) / 2;
        float dy = box.size.y * 1.f / (nSteps - 1);

        float nX = std::ceil(box.size.x / dy);
        float dx = box.size.x / nX;

        auto markCol = style()->getColor(style::XTStyle::PLOT_MARKS);
        for (int yd = 0; yd < nSteps; yd++)
        {
            if (yd == mid)
                continue;
            float y = yd * dy;
            float x = 0;

            while (x <= box.size.x)
            {
                nvgBeginPath(vg);
                nvgFillColor(vg, markCol);
                nvgEllipse(vg, x, y, 0.5, 0.5);
                nvgFill(vg);
                x += dx;
            }
        }
        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, box.size.y * 0.5);
        nvgLineTo(vg, box.size.x, box.size.y * 0.5);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, box.size.y);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, 0);
        nvgLineTo(vg, box.size.x, 0);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        auto nc = markCol;
        nc.r *= 1.2;
        nc.g *= 1.2;
        nc.b *= 1.2;
        nvgStrokeColor(vg, nc);
        bool first{true};
        for (const auto &[x, yv] : inputSignal)
        {
            auto y = y1toypx(yv);
            if (first)
            {
                nvgMoveTo(vg, x, y);
            }
            else
            {
                nvgLineTo(vg, x, y);
            }
            first = false;
        }
        nvgStrokeWidth(vg, 1.0);
        nvgStroke(vg);
    }

    void drawPlot(NVGcontext *vg)
    {
        if (!module)
        {
            // Draw the module name here for preview goodness
            nvgBeginPath(vg);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, 30);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, "WaveShaper", nullptr);
            return;
        }

        nvgBeginPath(vg);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        bool first{true};
        for (const auto &[x, yv] : outputSignal)
        {
            auto y = y1toypx(yv);
            if (first)
            {
                nvgMoveTo(vg, x, y);
            }
            else
            {
                nvgLineTo(vg, x, y);
            }
            first = false;
        }
        nvgStrokeWidth(vg, 1.5);
        nvgStroke(vg);
    }
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

    plotStartY += fivemm;
    plotH -= fivemm;

    auto fpw = WaveshaperPlotWidget::create(rack::Vec(plotStartX, plotStartY),
                                            rack::Vec(plotW, plotH + underPlotH), module);
    addChild(fpw);

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

        if (row == 1)
        {
            auto ulx = uxp + (col == 2 ? -1 : 1) * 5.5;
            auto uly = uyp - 5.5;

            auto light = rack::createParamCentered<widgets::ActivateKnobSwitch>(rack::mm2px(rack::Vec(ulx, uly)),
                                                                                module,
                                                                                col == 2 ?
                                    Waveshaper::LOCUT_ENABLED : Waveshaper::HICUT_ENABLED);
            addChild(light);

        }

        idx++;
    }

    int col{0};
    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        addChild(makeModLabel(i));
    }

    col = 0;

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

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::waveshaper::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeWaveshaper =
    rack::createModel<sst::surgext_rack::waveshaper::ui::WaveshaperWidget::M,
                      sst::surgext_rack::waveshaper::ui::WaveshaperWidget>("SurgeXTWaveshaper");
