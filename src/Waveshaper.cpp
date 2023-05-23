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

#include "Waveshaper.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "dsp/DSPExternalAdapterUtils.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::waveshaper::ui
{
struct WaveshaperWidget : widgets::XTModuleWidget
{
    typedef waveshaper::Waveshaper M;
    WaveshaperWidget(M *module);

    void moduleBackground(NVGcontext *vg) {}

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, 5> overlays;
    std::array<widgets::ModulatableKnob *, Waveshaper::n_wshp_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (module)
        {
            auto m = static_cast<M *>(module);
            menu->addChild(new rack::ui::MenuSeparator);

            menu->addChild(rack::createMenuItem("Apply DC Blocker", CHECKMARK(m->doDCBlock),
                                                [m]() { m->doDCBlock = !m->doDCBlock; }));

            menu->addChild(rack::createMenuItem(
                "Show Transform and Response", CHECKMARK(style()->getWaveshaperShowsBothCurves()),
                [this]() {
                    style()->setWaveshaperShowsBothCurves(!style()->getWaveshaperShowsBothCurves());
                }));
        }
    }
};

struct WaveshaperPlotWidget : public rack::widget::TransparentWidget, style::StyleParticipant
{
    typename WaveshaperWidget::M *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwPlot{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwResponse{nullptr};

    std::vector<std::pair<float, float>> inputSignal;
    std::vector<std::pair<float, float>> outputSignal;
    std::vector<std::pair<float, float>> responseSignal;
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

        bdwResponse = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(box.size.x * 0.666 - rack::mm2px(0), rack::mm2px(0)),
            rack::Vec(box.size.x * 0.333, box.size.y - rack::mm2px(0)),
            [this](auto *vg) { drawResponse(vg); });
        addChild(bdwResponse);
        calculateInputSignal();
    }
    void calculateInputSignal()
    {
        inputSignal.clear();
        auto fac = 2.0;
        auto inputRes = (int)box.size.x * fac;
        auto dx = 1.0 / inputRes;
        auto cmul = module ? (style()->getWaveshaperShowsBothCurves() ? 6.0 : 4.0) : 4.0;
        for (int i = 0; i < inputRes; ++i)
        {
            auto x = dx * i;
            auto y = std::sin(x * cmul * M_PI);
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
            bdwResponse->dirty = true;
        }

        rack::widget::Widget::step();
    }

    virtual void onStyleChanged() override
    {
        // If transform display has changed bounds adjust
        calculateInputSignal();
        recalcPath();
        bdw->dirty = true;
        bdwPlot->dirty = true;
        bdwResponse->dirty = true;
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
            auto wstype = (sst::waveshapers::WaveshaperType)std::round(
                module->paramQuantities[Waveshaper::WSHP_TYPE]->getValue());

            float ddb{0.f}, bias{0.f};
            if (style::XTStyle::getShowModulationAnimationOnDisplay())
            {
                ddb = module->modulationAssistant.values[Waveshaper::DRIVE][0];
                bias = module->modulationAssistant.values[Waveshaper::BIAS][0];
            }
            else
            {
                ddb = module->modulationAssistant.basevalues[Waveshaper::DRIVE];
                bias = module->modulationAssistant.basevalues[Waveshaper::BIAS];
            }

            dval = wstype != lastType || ddb != lastDrive || bias != lastBias;
        }
        return dval;
    }

    sst::waveshapers::WaveshaperType lastType{waveshapers::WaveshaperType::wst_none};
    float lastDrive{-100}, lastBias{-100};

    void recalcPath()
    {
        if (!module)
            return;

        responseSignal.clear();
        outputSignal.clear();
        auto wstype = (sst::waveshapers::WaveshaperType)std::round(
            module->paramQuantities[Waveshaper::WSHP_TYPE]->getValue());
        sst::waveshapers::QuadWaveshaperState wss;
        float R[4];

        {
            initializeWaveshaperRegister(wstype, R);

            for (int i = 0; i < sst::waveshapers::n_waveshaper_registers; ++i)
            {
                wss.R[i] = _mm_set1_ps(R[i]);
            }

            wss.init = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()); // better way?

            float ddb{0.f}, bias{0.f};
            if (style::XTStyle::getShowModulationAnimationOnDisplay())
            {
                ddb = module->modulationAssistant.values[Waveshaper::DRIVE][0];
                bias = module->modulationAssistant.values[Waveshaper::BIAS][0];
            }
            else
            {
                ddb = module->modulationAssistant.basevalues[Waveshaper::DRIVE];
                bias = module->modulationAssistant.basevalues[Waveshaper::BIAS];
            }

            auto wsop = sst::waveshapers::GetQuadWaveshaper(wstype);
            auto damp = pow(10, 0.05 * ddb);
            auto d1 = _mm_set1_ps(damp);

            lastType = wstype;
            lastBias = bias;
            lastDrive = ddb;

            for (const auto &[x, y] : inputSignal)
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

        {
            initializeWaveshaperRegister(wstype, R);

            for (int i = 0; i < sst::waveshapers::n_waveshaper_registers; ++i)
            {
                wss.R[i] = _mm_set1_ps(R[i]);
            }

            wss.init = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()); // better way?

            auto wsop = sst::waveshapers::GetQuadWaveshaper(wstype);

            for (float x = -2.0; x < 2.0; x += 2.0 * 0.01)
            {
                auto ivs = _mm_set1_ps(x);
                auto d1 = _mm_set1_ps(1.f);
                auto ov1 = ivs;

                if (wsop)
                {
                    ov1 = wsop(&wss, ivs, d1);
                }

                float r alignas(16)[8];
                _mm_store_ps(r, ov1);

                responseSignal.emplace_back(x, r[0]);
            }
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

    void drawResponse(NVGcontext *vg)
    {
        if (!module)
            return;

        if (!style()->getWaveshaperShowsBothCurves())
            return;

        auto bx = bdwResponse->box;
        nvgBeginPath(vg);
        auto markCol = style()->getColor(style::XTStyle::PLOT_MARKS);
        auto bCol = style()->getColor(style::XTStyle::LED_PANEL);
        // bCol.a = 0.95;
        nvgStrokeColor(vg, markCol);
        nvgFillColor(vg, bCol);
        nvgRect(vg, 0, 0, bx.getWidth(), bx.getHeight());
        nvgStrokeWidth(vg, 1);
        nvgFill(vg);
        nvgStroke(vg);

        auto xs = 2.0f, ys = 3.8f;

        nvgBeginPath(vg);
        bool start = true;
        for (float x = -xs; x <= xs; x += xs * 0.01)
        {
            auto px = (x + xs) / (2 * xs) * bx.getWidth();

            auto ly = -x;
            auto py = (ly + ys) / (2 * ys) * bx.getHeight();
            if (start)
                nvgMoveTo(vg, px, py);
            else
                nvgLineTo(vg, px, py);
            start = false;
        }
        nvgStroke(vg);

        auto crvCol = style()->getColor(style::XTStyle::PLOT_CURVE);
        nvgBeginPath(vg);
        nvgStrokeColor(vg, crvCol);
        start = true;
        for (auto &[x, y] : responseSignal)
        {
            auto px = (x + xs) / (2 * xs) * bx.getWidth();

            auto ly = -y;
            auto py = (ly + ys) / (2 * ys) * bx.getHeight();
            if (start)
                nvgMoveTo(vg, px, py);
            else
                nvgLineTo(vg, px, py);
            start = false;
        }
        nvgStroke(vg);
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
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5 + 2, "Wave", nullptr);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5 + 3, "Shaper", nullptr);
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
        int type = (int)std::round(getParamQuantity()->getValue());

        for (const auto &[id, gn] : wsm.mapping)
        {
            if (gn == group)
            {
                menu->addChild(rack::createMenuItem(sst::waveshapers::wst_names[id],
                                                    CHECKMARK(id == type),
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

        int type = (int)std::round(getParamQuantity()->getValue());

        for (const auto &[id, gn] : wsm.mapping)
        {
            if (gn == "")
            {
                menu->addChild(rack::createMenuItem(sst::filters::filter_type_names[id],
                                                    CHECKMARK(id == type),
                                                    [this, cid = id]() { setType(cid); }));
            }
            else if (gn != currentGroup)
            {
                bool check{false};
                for (const auto &[ird, gnr] : wsm.mapping)
                {
                    if (gnr == gn && ird == type)
                    {
                        check = true;
                        ;
                    }
                }
                menu->addChild(rack::createSubmenuItem(
                    gn, CHECKMARK(check), [this, cgn = gn](auto *x) { menuForGroup(x, cgn); }));
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

        auto pq = getParamQuantity();
        auto *h = new rack::history::ParamChange;
        h->name = std::string("change waveshaper type");
        h->moduleId = pq->module->id;
        h->paramId = pq->paramId;
        h->oldValue = pq->getValue();
        h->newValue = id;
        APP->history->push(h);
        pq->setValue(id);
    }

    int lastShape{-1};
    bool isDirty() override
    {
        if (forceDirty)
        {
            forceDirty = false;
            return true;
        }
        if (getParamQuantity())
        {
            auto s = (int)std::round(getParamQuantity()->getValue());
            if (s != lastShape)
            {
                lastShape = s;
                return true;
            }
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

    typedef layout::LayoutEngine<WaveshaperWidget, M::DRIVE> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "WAVESHAPER", "vco", "BlankVCO");
    addChild(bg);

    int idx = 0;

    float plotStartX = rack::mm2px(layout::LayoutConstants::VCOplotCX_MM -
                                   layout::LayoutConstants::VCOplotW_MM * 0.5);
    float plotStartY = rack::mm2px(layout::LayoutConstants::VCOplotCY_MM -
                                   layout::LayoutConstants::VCOplotH_MM * 0.5);
    float plotW = rack::mm2px(layout::LayoutConstants::VCOplotW_MM);
    float plotH = rack::mm2px(layout::LayoutConstants::VCOplotH_MM -
                              layout::LayoutConstants::VCOplotControlsH_MM);
    float underPlotStartY = plotStartY + plotH;
    float underPlotH = rack::mm2px(layout::LayoutConstants::VCOplotControlsH_MM);

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

    typedef layout::LayoutItem lay_t;
    for (const auto &lay : {
             lay_t::createVCFWSBigKnob(M::DRIVE, "DRIVE"),
             lay_t::createVCOKnob(M::BIAS, "BIAS", 0, 2),
             lay_t::createVCOKnob(M::OUT_GAIN, "GAIN", 0, 3),
             lay_t::createVCOKnob(M::LOCUT, "", 1, 2),
             lay_t::createVCOKnob(M::HICUT, "", 1, 3),
             lay_t::createVCOSpanLabel("LO - CUT - HI", 1, 2, 2),
             lay_t::createVCOLight(layout::LayoutItem::POWER_LIGHT, M::LOCUT_ENABLED, 1, 2),
             lay_t::createVCOLight(layout::LayoutItem::POWER_LIGHT, M::HICUT_ENABLED, 1, 3),
         })
    {
        engine_t::layoutItem(this, lay, "VCF");
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::WSHP_MOD_INPUT);
    engine_t::createLeftRightInputLabels(this);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::waveshaper::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeWaveshaper =
    rack::createModel<sst::surgext_rack::waveshaper::ui::WaveshaperWidget::M,
                      sst::surgext_rack::waveshaper::ui::WaveshaperWidget>("SurgeXTWaveshaper");
