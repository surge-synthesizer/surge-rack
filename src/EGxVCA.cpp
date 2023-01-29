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

#include "EGxVCA.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "SurgeXT.h"

namespace sst::surgext_rack::egxvca::ui
{
struct EGxVCAWidget : public widgets::XTModuleWidget
{
    typedef sst::surgext_rack::egxvca::EGxVCA M;
    EGxVCAWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_mod_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_mod_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    widgets::CurveSwitch *aShape{nullptr}, *dShape{nullptr}, *rShape{nullptr};
    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;

        menu->addChild(new rack::ui::MenuSeparator);
        /*
         * Clock entries
         */
        addClockMenu<EGxVCA>(menu);
        menu->addChild(new rack::ui::MenuSeparator);

        addSelectionMenu(menu, module->paramQuantities[M::ATTACK_FROM],
                         {{"Attack from Zero", 0}, {"Attack from Current (Legato)", 1}});
    }

    widgets::DirtyHelper<EGxVCA, false> modeDirty, analogDigitalDirty;
    void step() override
    {
        if ((modeDirty.dirty() || analogDigitalDirty.dirty()) && aShape && dShape && rShape)
        {
            auto type = modeDirty.lastValue;
            auto isDig = analogDigitalDirty.lastValue == 0;

            if (!isDig)
            {
                aShape->visible = false;
                dShape->visible = false;
                rShape->visible = false;
            }
            else
            {
                aShape->visible = true;
                rShape->visible = true;
                if (type == 0)
                {
                    dShape->visible = true;
                    rShape->drawDirection = widgets::CurveSwitch::FULL_RELEASE;
                }
                else
                {
                    dShape->visible = false;
                    rShape->drawDirection = widgets::CurveSwitch::FULL_RELEASE;
                }
            }
        }

        XTModuleWidget::step();
    }

    widgets::BufferedDrawFunctionWidget *divWidget{nullptr};
    void drawDividingLines(NVGcontext *vg)
    {
        if (!divWidget)
            return;

        auto col = style()->getColor(style::XTStyle::Colors::PLOT_MARKS);
        auto sw = 0.75;
        auto sz = divWidget->box.size;

        nvgBeginPath(vg);
        nvgStrokeColor(vg, col);
        nvgStrokeWidth(vg, sw);
        nvgMoveTo(vg, 2, rack::mm2px(5));
        nvgLineTo(vg, sz.x - 2, rack::mm2px(5));
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, col);
        nvgStrokeWidth(vg, sw);
        nvgMoveTo(vg, sz.x * 0.65 + rack::mm2px(1), rack::mm2px(5));
        nvgLineTo(vg, sz.x * 0.65 + rack::mm2px(1), sz.y - rack::mm2px(1));
        nvgStroke(vg);
    }
};

struct EnvCurveWidget : rack::Widget, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwCurve{nullptr};
    EGxVCA *module{nullptr};
    std::unordered_map<int, widgets::DirtyHelper<EGxVCA>> dirtyChecks;

    EnvCurveWidget(const rack::Vec &pos, const rack::Vec &size, EGxVCA *md)
    {
        box.pos = pos;
        box.size = size;
        module = md;

        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                      [this](auto vg) { drawBg(vg); });
        addChild(bdw);

        bdwCurve = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), size, [this](auto vg) { drawCurve(vg); });
        addChild(bdwCurve);

        auto su = [this](auto b, auto m) {
            auto ac = widgets::DirtyHelper<EGxVCA>();
            ac.module = module;
            ac.par = b;
            ac.isModulated = m;
            dirtyChecks[b] = ac;
        };
        su(EGxVCA::EG_A, true);
        su(EGxVCA::EG_D, true);
        su(EGxVCA::EG_S, true);
        su(EGxVCA::EG_R, true);
        su(EGxVCA::A_SHAPE, false);
        su(EGxVCA::D_SHAPE, false);
        su(EGxVCA::R_SHAPE, false);
        su(EGxVCA::ANALOG_OR_DIGITAL, false);
        su(EGxVCA::ADSR_OR_DAHD, false);
    }

    void drawBg(NVGcontext *vg)
    {
        /*
        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGB(255,0,0));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgStroke(vg);
         */
    }
    void drawCurve(NVGcontext *vg)
    {
        if (!module)
            return;

        auto a = dirtyChecks[EGxVCA::EG_A].lastValue;
        auto d = dirtyChecks[EGxVCA::EG_D].lastValue;
        auto s = dirtyChecks[EGxVCA::EG_S].lastValue;
        auto r = dirtyChecks[EGxVCA::EG_R].lastValue;

        auto as = dirtyChecks[EGxVCA::A_SHAPE].lastValue;
        auto ds = dirtyChecks[EGxVCA::D_SHAPE].lastValue;
        auto rs = dirtyChecks[EGxVCA::R_SHAPE].lastValue;

        auto isDig = dirtyChecks[EGxVCA::ANALOG_OR_DIGITAL].lastValue < 0.5;
        auto shp = dirtyChecks[EGxVCA::ADSR_OR_DAHD].lastValue;

        auto mx = modules::CTEnvTimeParamQuantity::etMax;
        auto mn = modules::CTEnvTimeParamQuantity::etMin;
        auto sc = mx - mn;
        auto gt = 0.f, endt = 0.f;
        if (shp < 0.5)
        {
            // adsr
            endt = pow(2, a * sc + mn) + pow(2, d * sc + mn) + pow(2, r * sc + mn);
            auto dGate = 0.33 * endt;
            endt += dGate;
            endt = std::max(0.25f, endt);
            gt = endt - pow(2, r * sc + mn);
        }
        else
        {
            endt = pow(2, a * sc + mn) + pow(2, d * sc + mn) + pow(2, s * sc + mn) +
                   pow(2, r * sc + mn);
        }

        auto smp = endt * module->storage->samplerate;
        auto runs = smp * BLOCK_SIZE_INV;
        auto smpEvery = std::max((int)std::floor(runs / (box.size.x * 4)), 1);
        auto gtSmp = gt * module->storage->samplerate * BLOCK_SIZE_INV;

        auto env = dsp::envelopes::ADSRDAHDEnvelope(module->storage.get());
        env.attackFrom((dsp::envelopes::ADSRDAHDEnvelope::Mode)shp, 0, a, as, isDig);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, box.size.y - 2); // that's the 0,0 point

        for (int i = 0; i < runs; ++i)
        {
            env.process(a, d, s, r, as, ds, rs, i < gtSmp);
            if ((i % smpEvery) == 0)
            {
                auto v = env.output;
                nvgLineTo(vg, box.size.x * i / runs, (1.0 - v) * (box.size.y - 4) + 2);
            }
            env.current = BLOCK_SIZE;
        }
        nvgLineTo(vg, box.size.x, box.size.y - 2);

        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        nvgStrokeWidth(vg, 1.25);
        nvgStroke(vg);

        auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
        auto gcp = col;
        gcp.a = 0.5;
        auto gcn = col;
        gcn.a = 0.0;
        nvgFillPaint(vg, nvgLinearGradient(vg, 0, 0, 0, box.size.y * 0.9, gcp, gcn));
        nvgFill(vg);
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwCurve->dirty = true;
    }

    void step() override
    {
        if (module)
            for (auto &[i, dc] : dirtyChecks)
                if (dc.dirty())
                {
                    bdwCurve->dirty = true;
                    bdw->dirty = true;
                }
        Widget::step();
    }
};

struct ResponseMeterWidget : rack::Widget, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwCurve{nullptr};
    EGxVCA *module{nullptr};
    ResponseMeterWidget(const rack::Vec &pos, const rack::Vec &size, EGxVCA *md)
    {
        box.pos = pos;
        box.size = size;
        module = md;

        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                      [this](auto vg) { drawBg(vg); });
        addChild(bdw);

        bdwCurve = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), size, [this](auto vg) { drawCurve(vg); });
        addChild(bdwCurve);
        for (int i = 0; i < MAX_POLY; ++i)
            meterValues[i] = 0;
    }

    void drawBg(NVGcontext *vg)
    {
        /*
        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGB(255,0,0));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgStroke(vg);
         */
    }
    void drawCurve(NVGcontext *vg)
    {
        auto dx = box.size.x / nChan;
        auto sy = box.size.y - 2;
        for (int i = 0; i < nChan; ++i)
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            auto v = meterValues[i];

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            auto gcp = col;
            gcp.a = 0.5;
            auto gcn = col;
            gcn.a = 0.0;
            nvgFillPaint(vg, nvgLinearGradient(vg, 0, (1 - v) * sy, 0, sy, gcp, gcn));

            nvgRect(vg, dx * i, (1 - v) * sy, dx, v * sy);
            nvgFill(vg);
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);

            nvgBeginPath(vg);
            nvgMoveTo(vg, dx * i, (1 - v) * sy);
            nvgLineTo(vg, dx * (i + 1), (1 - v) * sy);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 1.25);
            nvgStroke(vg);
        }
    }

    float meterValues[MAX_POLY];
    float nChan{1};

    void step() override
    {
        if (module)
        {
            bool dirty{false};
            dirty = (nChan != module->nChan);
            nChan = module->nChan;
            for (int i = 0; i < MAX_POLY; ++i)
            {
                dirty = dirty || (module->meterLevels[i] != meterValues[i]);
                meterValues[i] = module->meterLevels[i];
            }
            if (dirty)
            {
                bdw->dirty = true;
                bdwCurve->dirty = true;
            }
        }
        rack::Widget::step();
    }

    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwCurve->dirty = true;
    }
};

EGxVCAWidget::EGxVCAWidget(sst::surgext_rack::egxvca::ui::EGxVCAWidget::M *m)
{
    setModule(m);
    typedef layout::LayoutEngine<EGxVCAWidget, EGxVCAWidget::M::LEVEL, EGxVCAWidget::M::CLOCK_IN>
        engine_t;
    engine_t::initializeModulationToBlank(this);

    modeDirty.module = m;
    modeDirty.par = M::ADSR_OR_DAHD;

    analogDigitalDirty.module = m;
    analogDigitalDirty.par = M::ANALOG_OR_DIGITAL;

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "EG x VCA", "fx", "BlankNoDisplay");
    addChild(bg);

    auto col = std::vector<float>();
    for (int i = 0; i < 4; ++i)
        col.push_back(layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::columnWidth_MM * i);

    const auto col0 = layout::LayoutConstants::firstColumnCenter_MM;
    const auto col1 =
        layout::LayoutConstants::firstColumnCenter_MM + layout::LayoutConstants::columnWidth_MM;
    const auto col2 =
        layout::LayoutConstants::firstColumnCenter_MM + 2 * layout::LayoutConstants::columnWidth_MM;
    const auto col3 =
        layout::LayoutConstants::firstColumnCenter_MM + 3 * layout::LayoutConstants::columnWidth_MM;

    const auto dSlider = layout::LayoutConstants::columnWidth_MM * 0.5f;

    const auto sliderStart = col0 - layout::LayoutConstants::columnWidth_MM * 0.75f;
    const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1];
    const auto row2 = layout::LayoutConstants::vcoRowCenters_MM[0];
    const auto row3 = layout::LayoutConstants::vcoRowCenters_MM[0] - (row1 - row2);
    const auto rowS = (row2 + row3) * 0.5f;

    typedef layout::LayoutItem li_t;
    // fixme use the enums
    // clang-format off
    std::vector<li_t> layout = {
        {li_t::KNOB12, "LEVEL", M::LEVEL, col3, rowS},
        {li_t::KNOB9, "PAN", M::PAN, col2, row3},
        {li_t::KNOB9, "RESP", M::RESPONSE, col2, row2},

        {li_t::PORT, "GATE", M::GATE_IN, col0, row1},
        {li_t::PORT, "CLOCK", M::CLOCK_IN, col1, row1},
        {li_t::OUT_PORT, "EOC", M::EOC_OUT, col2, row1},
        {li_t::OUT_PORT, "ENV", M::ENV_OUT, col3, row1},

        li_t::createLCDArea(row3 - rack::mm2px(2.5))
    };
    // clang-format on

    for (auto &lay : layout)
    {
        if (lay.parId == M::GATE_IN && lay.type == li_t::PORT)
        {
            lay.dynamicLabel = true;
            lay.dynLabelFn = [](modules::XTModule *m) -> std::string {
                auto mode = 0;
                if (m)
                    mode = std::round(m->paramQuantities[EGxVCA::ADSR_OR_DAHD]->getValue());
                if (mode == 0)
                    return "GATE";
                return "TRIG";
            };
        }
        engine_t::layoutItem(this, lay, "EGxVCA");
    }

    std::vector<li_t> sliderLayout = {
        {li_t::VSLIDER_25, "A", M::EG_A, sliderStart + 1.f * dSlider, rowS},
        {li_t::VSLIDER_25, "D", M::EG_D, sliderStart + 2.f * dSlider, rowS},
        {li_t::VSLIDER_25, "S", M::EG_S, sliderStart + 3.f * dSlider, rowS},
        {li_t::VSLIDER_25, "R", M::EG_R, sliderStart + 4.f * dSlider, rowS},
    };

    int i{0};
    for (auto &lay : sliderLayout)
    {
        lay.dynamicLabel = true;
        lay.dynLabelFn = [i](modules::XTModule *m) -> std::string {
            auto mode = 0;
            if (m)
                mode = std::round(m->paramQuantities[EGxVCA::ADSR_OR_DAHD]->getValue());
            if (mode == 0)
            {
                switch (i)
                {
                case 0:
                    return "A";
                case 1:
                    return "D";
                case 2:
                    return "S";
                case 3:
                    return "R";
                }
            }
            else
            {
                switch (i)
                {
                case 0:
                    return "D";
                case 1:
                    return "A";
                case 2:
                    return "H";
                case 3:
                    return "D";
                }
            }
            return {"ERR"};
        };
        i++;
        engine_t::layoutItem(this, lay, "EGxVCA");
    }

    rack::Rect lcdB;
    auto lc = getFirstDescendantOfType<widgets::LCDBackground>();
    if (lc)
        lcdB = lc->box;

    {
        divWidget = new widgets::BufferedDrawFunctionWidget(
            lc->box.pos, lc->box.size, [this](auto vg) { drawDividingLines(vg); });
        addChild(divWidget);
    }
    {
        auto ads = rack::Vec(lcdB.size.x * 0.25, rack::mm2px(5.0));
        auto adp = rack::Vec(lcdB.pos.x + lcdB.size.x - ads.x - rack::mm2px(1.5),
                             lcdB.pos.y - rack::mm2px(0.5));
        auto andig = widgets::PlotAreaToggleClick::create(adp, ads, module, M::ANALOG_OR_DIGITAL);
        addChild(andig);
    }
    {
        auto ads = rack::Vec(lcdB.size.x * 0.25, rack::mm2px(5.0));
        auto adp = rack::Vec(lcdB.pos.x + rack::mm2px(1.5), lcdB.pos.y - rack::mm2px(0.5));
        auto mode = widgets::PlotAreaToggleClick::create(adp, ads, module, M::ADSR_OR_DAHD);
        mode->align = widgets::PlotAreaToggleClick::LEFT;
        addChild(mode);
    }

    {
        auto cs = rack::mm2px(4.5);
        auto ws = rack::mm2px(3.0);
        auto off = rack::mm2px(0.5);
        auto x0 = lcdB.pos.x + lcdB.size.x * 0.5 - 1.5 * cs + off;
        auto y0 = lcdB.pos.y + off + rack::mm2px(0.5);

        auto A = rack::createParam<widgets::CurveSwitch>(rack::Vec(x0, y0), module, M::A_SHAPE);
        A->box.size = rack::Vec(ws, ws);
        A->drawDirection = widgets::CurveSwitch::ATTACK;
        addChild(A);
        aShape = A;

        auto D =
            rack::createParam<widgets::CurveSwitch>(rack::Vec(x0 + cs, y0), module, M::D_SHAPE);
        D->box.size = rack::Vec(ws, ws);
        D->drawDirection = widgets::CurveSwitch::FULL_RELEASE;
        addChild(D);
        dShape = D;

        auto R =
            rack::createParam<widgets::CurveSwitch>(rack::Vec(x0 + 2 * cs, y0), module, M::R_SHAPE);
        R->box.size = rack::Vec(ws, ws);
        R->drawDirection = widgets::CurveSwitch::FULL_RELEASE;
        addChild(R);
        rShape = R;
    }

    auto envc = new EnvCurveWidget(
        rack::Vec(lcdB.pos.x + rack::mm2px(1), lcdB.pos.y + rack::mm2px(6)),
        rack::Vec(lcdB.size.x * .65 - rack::mm2px(1), lcdB.size.y - rack::mm2px(7.5)), m);
    addChild(envc);

    auto respc = new ResponseMeterWidget(
        rack::Vec(lcdB.pos.x + rack::mm2px(2) + lcdB.size.x * 0.65, lcdB.pos.y + rack::mm2px(6)),
        rack::Vec(lcdB.size.x * .35 - rack::mm2px(4), lcdB.size.y - rack::mm2px(7.5)), m);
    addChild(respc);

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0);
    engine_t::createLeftRightInputLabels(this);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::egxvca::ui

rack::Model *modelEGxVCA =
    rack::createModel<sst::surgext_rack::egxvca::ui::EGxVCAWidget::M,
                      sst::surgext_rack::egxvca::ui::EGxVCAWidget>("SurgeXTEGxVCA");
