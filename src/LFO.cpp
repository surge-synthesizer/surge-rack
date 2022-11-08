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

#include "LFO.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::lfo::ui
{
struct LFOWidget : widgets::XTModuleWidget
{
    typedef lfo::LFO M;
    LFOWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_lfo_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_lfo_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    int priorShape{-1};
    rack::Widget *wavePlot{nullptr}, *stepEditor{nullptr};
    rack::Widget *stepStart{nullptr}, *stepEnd{nullptr};
    void step() override;

    void polyMenu(rack::Menu *p, M *m)
    {
        if (!m || !m->paramQuantities[M::NO_TRIG_POLY])
            return;
        p->addChild(rack::createMenuLabel("Polyphony"));
        p->addChild(new rack::ui::MenuSeparator);

        auto tt = (LFO::TrigBroadcastMode)std::round(
            m->paramQuantities[M::BROADCAST_TRIG_TO_POLY]->getValue());

        p->addChild(rack::createMenuItem(
            "Trigger Sets Polyphony If Connected", CHECKMARK(tt == M::FOLLOW_TRIG_POLY), [m]() {
                m->paramQuantities[M::BROADCAST_TRIG_TO_POLY]->setValue(M::FOLLOW_TRIG_POLY);
            }));

        p->addChild(rack::createMenuItem(
            "No Trig or Trig Chan 1 Triggers All", CHECKMARK(tt == M::TAKE_CHANNEL_0),
            [m]() { m->paramQuantities[M::BROADCAST_TRIG_TO_POLY]->setValue(M::TAKE_CHANNEL_0); }));
        p->addChild(new rack::ui::MenuSeparator);
        int cp = (int)std::round(m->paramQuantities[M::NO_TRIG_POLY]->getValue());
        for (int i = 1; i <= 16; ++i)
        {
            p->addChild(rack::createMenuItem(std::to_string(i), CHECKMARK(i == cp), [m, i]() {
                m->paramQuantities[M::NO_TRIG_POLY]->setValue(i);
            }));
        }
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
        auto m = static_cast<M *>(module);
        {
            auto shp = m->lfostorage->shape.val.i;
            auto mx = lt_num_deforms[shp];
            if (mx > 0)
            {
                menu->addChild(new rack::MenuSeparator);
                auto dv = (int)std::round(m->paramQuantities[LFO::DEFORM_TYPE]->getValue());
                for (int i = 0; i < mx; ++i)
                {
                    auto nm = "Deform Type " + std::to_string(i + 1);

                    menu->addChild(rack::createMenuItem(nm, CHECKMARK(i == dv), [m, i] {
                        m->paramQuantities[LFO::DEFORM_TYPE]->setValue(i);
                    }));
                }
            }
        }

        menu->addChild(new rack::MenuSeparator);
        menu->addChild(
            rack::createSubmenuItem("Polyphony", "", [this, m](auto *x) { polyMenu(x, m); }));

        menu->addChild(
            rack::createMenuItem("Envelope Triggers from Zero", CHECKMARK(m->retriggerFromZero),
                                 [m]() { m->retriggerFromZero = !m->retriggerFromZero; }));

        menu->addChild(rack::createMenuItem(
            "Random Phase on Attack", CHECKMARK(m->params[LFO::RANDOM_PHASE].getValue() > 0.5),
            [m]() {
                auto v = m->paramQuantities[LFO::RANDOM_PHASE]->getValue() > 0.5;
                m->paramQuantities[LFO::RANDOM_PHASE]->setValue(v ? 0 : 1);
            }));
        menu->addChild(rack::createMenuItem(
            "Scale LFO and EG Outputs by Amp",
            CHECKMARK(m->params[LFO::SCALE_RAW_OUTPUTS].getValue() > 0.5), [m]() {
                auto v = m->paramQuantities[LFO::SCALE_RAW_OUTPUTS]->getValue() > 0.5;
                m->paramQuantities[LFO::SCALE_RAW_OUTPUTS]->setValue(v ? 0 : 1);
            }));
        menu->addChild(rack::createMenuItem(
            "Set EG to Zero when No Trigger Connected",
            CHECKMARK(m->params[LFO::UNTRIGGERED_ENV_NONZERO].getValue() < 0.5), [m]() {
                auto v = m->paramQuantities[LFO::UNTRIGGERED_ENV_NONZERO]->getValue() > 0.5;
                m->paramQuantities[LFO::UNTRIGGERED_ENV_NONZERO]->setValue(v ? 0 : 1);
            }));
        menu->addChild(new rack::MenuSeparator);
        typedef modules::ClockProcessor<LFO> cp_t;

        auto wts = (int)std::round(m->paramQuantities[LFO::WHICH_TEMPOSYNC]->getValue());
        auto wtR = (bool)(wts & 1);
        auto wtE = (bool)(wts & 2);
        menu->addChild(rack::createMenuItem("Temposync LFO Rate", CHECKMARK(wtR),
                                            [m, wtE, wtR]() { m->setWhichTemposyc(!wtR, wtE); }));
        menu->addChild(rack::createMenuItem("Temposync Env Rates", CHECKMARK(wtE),
                                            [m, wtE, wtR]() { m->setWhichTemposyc(wtR, !wtE); }));
        menu->addChild(new rack::MenuSeparator);

        auto t = m->clockProc.clockStyle;
        menu->addChild(
            rack::createMenuItem("Clock in QuarterNotes", CHECKMARK(t == cp_t::QUARTER_NOTE),
                                 [m]() { m->clockProc.clockStyle = cp_t::QUARTER_NOTE; }));

        menu->addChild(rack::createMenuItem("Clock in BPM CV", CHECKMARK(t == cp_t::BPM_VOCT),
                                            [m]() { m->clockProc.clockStyle = cp_t::BPM_VOCT; }));
    }

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }
};

struct LFOStepWidget : rack::Widget, style::StyleParticipant
{
    struct StepSlider : rack::app::SliderKnob, style::StyleParticipant
    {
        widgets::BufferedDrawFunctionWidget *bdw{nullptr};
        widgets::BufferedDrawFunctionWidget *bdwLight{nullptr};

        static StepSlider *create(const rack::Vec &pos, const rack::Vec &sz,
                                  modules::XTModule *module, int paramId)
        {
            auto res = new StepSlider();

            res->box.pos = pos;
            res->box.size = sz;

            res->module = module;
            res->paramId = paramId;
            res->initParamQuantity();

            res->setup();

            return res;
        }

        void setup()
        {
            bdw = new widgets::BufferedDrawFunctionWidget(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawSlider(vg); });
            bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawLight(vg); });

            addChild(bdw);
            addChild(bdwLight);
        }

        bool isInLoop()
        {
            auto step = paramId - LFO::STEP_SEQUENCER_STEP_0;
            return step >= sp && step < ep;
        }
        bool isInOrBeforeLoop()
        {
            auto step = paramId - LFO::STEP_SEQUENCER_STEP_0;
            return step < ep;
        }

        static constexpr float cramY = 0.98, padY = (1.f - cramY) * 0.5f;
        void drawSlider(NVGcontext *vg)
        {
            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, box.size.x, box.size.y);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);

            auto pq = getParamQuantity();
            if (!pq)
                return;
            auto v = (-pq->getValue()) * 0.5 + 0.5;
            if (uni)
                v = (1.0 - std::clamp(pq->getValue(), 0.f, 1.f));

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            if (!isInLoop())
                col = style()->getColor(style::XTStyle::PLOT_MARKS);
            auto gcp = col;
            gcp.a = 0.0;
            auto gcn = col;
            gcn.a = 0.9;

            auto s = box.size.y * 0.5;
            if (uni)
            {
                s = box.size.y;
            }
            auto e = cramY * box.size.y * v + padY;
            if (s > e)
            {
                std::swap(gcp, gcn);
                std::swap(s, e);
            }
            nvgBeginPath(vg);
            nvgRect(vg, 1, s, box.size.x - 2, e - s);
            nvgFillPaint(vg, nvgLinearGradient(vg, 0, s, 0, e, gcp, gcn));
            nvgFill(vg);

            if (!uni)
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, 0, box.size.y * 0.5);
                nvgLineTo(vg, box.size.x, box.size.y * 0.5);
                nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
                nvgStrokeWidth(vg, 0.75);
                nvgStroke(vg);
            }
        }

        void drawLight(NVGcontext *vg)
        {
            auto pq = getParamQuantity();
            if (!pq)
                return;
            auto v = (-pq->getValue()) * 0.5 + 0.5;
            if (uni)
                v = (1.0 - std::clamp(pq->getValue(), 0.f, 1.f));

            auto e = cramY * box.size.y * v + padY;

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            if (!isInOrBeforeLoop())
                col = style()->getColor(style::XTStyle::PLOT_MARKS);

            nvgBeginPath(vg);
            nvgMoveTo(vg, 1, e);
            nvgLineTo(vg, box.size.x - 1, e);
            nvgStrokeWidth(vg, 1.5);
            nvgStrokeColor(vg, col);
            nvgStroke(vg);
        }

        int sp{-1}, ep{-1};
        bool uni{false};
        void step() override
        {
            if (module)
            {
                auto ss =
                    (int)std::round(module->paramQuantities[LFO::STEP_SEQUENCER_START]->getValue());
                auto se =
                    (int)std::round(module->paramQuantities[LFO::STEP_SEQUENCER_END]->getValue());
                auto su = (bool)(module->paramQuantities[LFO::UNIPOLAR]->getValue() > 0.5);
                if (sp != ss || ep != se || uni != su)
                {
                    bdwLight->dirty = true;
                    bdw->dirty = true;
                }
                sp = ss;
                ep = se;
                uni = su;
            }
            rack::app::SliderKnob::step();
        }

        inline void onChange(const rack::widget::Widget::ChangeEvent &e) override
        {
            bdw->dirty = true;
            bdwLight->dirty = true;
        }

        void onStyleChanged() override
        {
            bdw->dirty = true;
            bdwLight->dirty = true;
        }

        rack::Vec buttonPos;
        void onButton(const ButtonEvent &e) override
        {
            if (e.action == GLFW_PRESS)
                buttonPos = e.pos;

            if (e.action == GLFW_RELEASE)
            {
                auto diff = buttonPos - e.pos;
                if (diff.norm() < 0.05)
                {
                    // auto e = cramY * box.size.y * v + padY;
                    // e - padY = cY * bsy * v
                    // v = ( e- padY)/(cY * bsy);
                    auto val = (buttonPos.y - padY) / (cramY * box.size.y);
                    if (uni)
                        val = 1 - val;
                    else
                        val = (1 - val) * 2 - 1;
                    if (getParamQuantity())
                        getParamQuantity()->setValue(val);
                }
                buttonPos = rack::Vec(-1, -1);
            }

            SliderKnob::onButton(e);
        }
    };

    struct TriggerSwitch : rack::app::Switch, style::StyleParticipant
    {
        widgets::BufferedDrawFunctionWidget *bdw{nullptr};
        widgets::BufferedDrawFunctionWidget *bdwLight{nullptr};

        static TriggerSwitch *create(const rack::Vec &pos, const rack::Vec &sz,
                                     modules::XTModule *module, int paramId)
        {
            auto res = new TriggerSwitch();

            res->box.pos = pos;
            res->box.size = sz;

            res->module = module;
            res->paramId = paramId;
            res->initParamQuantity();

            res->setup();

            return res;
        }

        void setup()
        {
            bdw = new widgets::BufferedDrawFunctionWidget(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawSlider(vg); });
            bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawLight(vg); });

            addChild(bdw);
            addChild(bdwLight);
        }

        void drawSlider(NVGcontext *vg)
        {
            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, box.size.x, box.size.y);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);

            nvgBeginPath(vg);
            nvgFontFaceId(vg, style()->fontId(vg));
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgText(vg, 0.5, box.size.y * 0.5, "A", nullptr);

            nvgBeginPath(vg);
            nvgFontFaceId(vg, style()->fontId(vg));
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgText(vg, box.size.x - 0.5, box.size.y * 0.5, "B", nullptr);
        }

        void drawLight(NVGcontext *vg)
        {
            auto pq = getParamQuantity();
            if (!pq)
                return;
            auto sv = (int)std::round(pq->getValue());

            if (sv & 1)
            {
                nvgBeginPath(vg);
                nvgFontFaceId(vg, style()->fontId(vg));
                nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
                nvgText(vg, 0.5, box.size.y * 0.5, "A", nullptr);
            }
            if (sv & 2)
            {
                nvgBeginPath(vg);
                nvgFontFaceId(vg, style()->fontId(vg));
                nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
                nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
                nvgText(vg, box.size.x - 0.5, box.size.y * 0.5, "B", nullptr);
            }
        }

        inline void onChange(const rack::widget::Widget::ChangeEvent &e) override
        {
            bdw->dirty = true;
            bdwLight->dirty = true;
        }

        void onStyleChanged() override
        {
            bdw->dirty = true;
            bdwLight->dirty = true;
        }
    };

    struct JogPatternButton : rack::Widget, style::StyleParticipant
    {
        widgets::BufferedDrawFunctionWidget *bdw{nullptr};
        LFO *module{nullptr};

        static JogPatternButton *create(const rack::Vec &pos, const rack::Vec &sz, LFO *module)
        {
            auto res = new JogPatternButton();

            res->box.pos = pos;
            res->box.size = sz;

            res->module = module;

            res->setup();

            return res;
        }

        void setup()
        {
            bdw = new widgets::BufferedDrawFunctionWidget(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawButton(vg); });
            addChild(bdw);
        }
        void drawButton(NVGcontext *vg)
        {
            // > in top half < in bottom
            auto tq = box.size.y * 0.25;
            auto bq = box.size.y * 0.75;
            auto ts = box.size.x;

            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, tq - ts * 0.5);
            nvgLineTo(vg, box.size.x, tq);
            nvgLineTo(vg, 0, tq + ts * 0.5);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgMoveTo(vg, box.size.x, bq - ts * 0.5);
            nvgLineTo(vg, 0, bq);
            nvgLineTo(vg, box.size.x, bq + ts * 0.5);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgFill(vg);
        }
        void onStyleChanged() override { bdw->dirty = true; }
        void onButton(const ButtonEvent &e) override
        {
            if (module && e.action == GLFW_PRESS)
            {
                int dir = 1;
                if (e.pos.y > box.size.y * 0.5)
                {
                    dir = -1;
                }
                float tvals[LFO::n_steps];
                float gvals[LFO::n_steps];
                for (int i = 0; i < LFO::n_steps; ++i)
                {
                    tvals[i] = module->paramQuantities[LFO::STEP_SEQUENCER_STEP_0 + i]->getValue();
                    gvals[i] =
                        module->paramQuantities[LFO::STEP_SEQUENCER_TRIGGER_0 + i]->getValue();
                }
                for (int i = 0; i < LFO::n_steps; ++i)
                {
                    auto next = (i + dir) & (LFO::n_steps - 1);
                    module->paramQuantities[LFO::STEP_SEQUENCER_STEP_0 + next]->setValue(tvals[i]);
                    module->paramQuantities[LFO::STEP_SEQUENCER_TRIGGER_0 + next]->setValue(
                        gvals[i]);
                }

                e.consume(this);
                return;
            }
            Widget::onButton(e);
        }
    };

    widgets::BufferedDrawFunctionWidget *bdw{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwLight{nullptr};
    LFO *module{nullptr};
    void setup()
    {
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { this->drawEditorBG(vg); });
        addChild(bdw);
        bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto vg) { this->drawEditorLights(vg); });
        addChild(bdwLight);

        float x0 = 15, pad = rack::mm2px(2);

        auto ys = 25;
        float ytrig0 = rack::mm2px(1), y0 = rack::mm2px(4), ypad = rack::mm2px(1);
        auto dx = (box.size.x - x0 - pad) / LFO::n_steps;

        auto jog = JogPatternButton::create(
            rack::Vec(2, y0 + ytrig0), rack::Vec(x0 - 4, box.size.y - y0 - ypad - ytrig0), module);
        addChild(jog);

        for (int i = 0; i < LFO::n_steps; ++i)
        {
            auto pos = rack::Vec(i * dx + x0, y0 + ytrig0);
            auto sz = rack::Vec(dx, box.size.y - y0 - ypad - ytrig0);
            auto ks = StepSlider::create(pos, sz, module, LFO::STEP_SEQUENCER_STEP_0 + i);
            addChild(ks);

            auto spos = rack::Vec(i * dx + x0, ytrig0);
            auto ssz = rack::Vec(dx, y0);
            auto sks = TriggerSwitch::create(spos, ssz, module, LFO::STEP_SEQUENCER_TRIGGER_0 + i);
            addChild(sks);
        }
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
    }
    void drawEditorBG(NVGcontext *vg) {}
    void drawEditorLights(NVGcontext *vg) {}
};

struct LFOTypeWidget : rack::app::ParamWidget, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};

    static constexpr int nTypes{8};
    std::array<std::shared_ptr<rack::Svg>, nTypes> svgs;

    static LFOTypeWidget *create(float widthInScrews, LFO *module, int param)
    {
        auto res = new LFOTypeWidget();
        res->box.pos.x = rack::mm2px(widgets::LCDBackground::contentPosX_MM);
        res->box.pos.y = widgets::LCDBackground::posy + rack::mm2px(1);
        res->box.size.x = rack::app::RACK_GRID_WIDTH * widthInScrews -
                          rack::mm2px(2 * widgets::LCDBackground::contentPosX_MM);
        res->box.size.y = rack::mm2px(6);
        res->module = module;
        res->paramId = param;
        res->initParamQuantity();

        res->setup();

        return res;
    }

    void setup()
    {
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { this->drawWidget(vg); });
        addChild(bdw);
        reloadGlyphs();
    }

    int sel{0};
    void step() override
    {
        int nsel = sel;
        if (module)
        {
            auto lf = static_cast<LFO *>(module);
            nsel = lf->lfostorage->shape.val.i;
        }
        if (nsel != sel)
            bdw->dirty = true;
        sel = nsel;
    }

    void drawWidget(NVGcontext *vg)
    {
        auto dSpan = box.size.x / nTypes;
        auto col = style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT);
        for (int i = 0; i < nTypes; ++i)
        {
            auto x = dSpan * i;
            auto y = 0;
            auto w = dSpan;
            auto h = box.size.y;

            auto pt = col;
            if (i == sel)
            {
                pt = style()->getColor(style::XTStyle::LED_PANEL);
                nvgBeginPath(vg);
                nvgRect(vg, x, y, w, h);
                nvgFillColor(vg, col);
                nvgFill(vg);
            }

            if (i != 0)
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, x, 0);
                nvgLineTo(vg, x, box.size.y);
                nvgStrokeWidth(vg, 0.5);
                nvgStrokeColor(vg, col);
                nvgStroke(vg);
            }

            auto hn = svgs[i]->handle;
            for (auto s = hn->shapes; s; s = s->next)
            {
                s->fill.color = (255 << 24) + (((int)(pt.b * 255)) << 16) +
                                (((int)(pt.g * 255)) << 8) + (int)(pt.r * 255);
            }

            float centerFudge = 1;
            nvgSave(vg);
            nvgTranslate(vg, x + centerFudge, y);
            rack::svgDraw(vg, svgs[i]->handle);
            nvgRestore(vg);
        }
        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, box.size.y);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgStrokeWidth(vg, 1.0);
        nvgStrokeColor(vg, col);
        nvgStroke(vg);
    }

    void onButton(const ButtonEvent &e) override
    {
        if (!module || !module->paramQuantities[LFO::SHAPE])
            return;

        if (e.action == GLFW_PRESS)
        {
            auto dSpan = box.size.x / (nTypes);
            float which = std::floor(e.pos.x / dSpan);

            auto pq = module->paramQuantities[LFO::SHAPE];
            auto val = which / (n_lfo_types - 1);
            auto *h = new rack::history::ParamChange;
            h->name = std::string("change lfo shape");
            h->moduleId = pq->module->id;
            h->paramId = pq->paramId;
            h->oldValue = pq->getValue();
            h->newValue = val;
            APP->history->push(h);

            pq->setValue(val);
            bdw->dirty = true;
            e.consume(this);
        }
    }

    void reloadGlyphs()
    {
        for (int i = 0; i < nTypes; ++i)
        {
            auto svg = rack::Svg::load(rack::asset::plugin(
                pluginInstance, std::string("res/xt/glyphs/lt_") + std::to_string(i) + ".svg"));
            svgs[i] = svg;
        }
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        reloadGlyphs();
    }
    void onChange(const ChangeEvent &e) override { bdw->dirty = true; }
};

struct LFOWaveform : rack::Widget, style::StyleParticipant
{
    LFO *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwBG{nullptr}, *bdwLight{nullptr};
    void setup()
    {
        bdwBG = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                        [this](auto vg) { this->drawBG(vg); });
        addChild(bdwBG);
        bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto vg) { this->drawLight(vg); });
        addChild(bdwLight);
        memset(tp, 0, n_scene_params * sizeof(pdata));
    }

    void onStyleChanged() override
    {
        bdwLight->dirty = true;
        bdwBG->dirty = true;
    }

    void drawBG(NVGcontext *vg) {}

    void drawLight(NVGcontext *vg)
    {
        if (!module)
            return;
        drawWaveform(vg);
    }

    pdata tp[n_scene_params];
    int priorDef{-1};
    void step() override
    {
        if (!module)
            return;

        auto lfodata = module->lfostorageDisplay;
        auto storage = module->storage.get();

        bool changed = false;
        changed = changed || (tp[lfodata->delay.param_id_in_scene].i != lfodata->delay.val.i);
        changed = changed || (tp[lfodata->attack.param_id_in_scene].i != lfodata->attack.val.i);
        changed = changed || (tp[lfodata->hold.param_id_in_scene].i != lfodata->hold.val.i);
        changed = changed || (tp[lfodata->decay.param_id_in_scene].i != lfodata->decay.val.i);
        changed = changed || (tp[lfodata->sustain.param_id_in_scene].i != lfodata->sustain.val.i);
        changed = changed || (tp[lfodata->release.param_id_in_scene].i != lfodata->release.val.i);

        changed =
            changed || (tp[lfodata->magnitude.param_id_in_scene].i != lfodata->magnitude.val.i);
        changed = changed || (tp[lfodata->rate.param_id_in_scene].i != lfodata->rate.val.i);
        changed = changed || (tp[lfodata->shape.param_id_in_scene].i != lfodata->shape.val.i);
        changed =
            changed || (tp[lfodata->start_phase.param_id_in_scene].i != lfodata->start_phase.val.i);
        changed = changed || (tp[lfodata->deform.param_id_in_scene].i != lfodata->deform.val.i);
        changed = changed || (tp[lfodata->unipolar.param_id_in_scene].i != lfodata->unipolar.val.i);

        changed = changed || (priorDef != lfodata->deform.deform_type);
        priorDef = lfodata->deform.deform_type;
        if (changed)
        {
            bdwLight->dirty = true;
            bdwBG->dirty = true;

            auto isUni = module->params[LFO::UNIPOLAR].getValue() > 0.5;
            float minStepSlider{-1};
            if (isUni)
            {
                minStepSlider = 0;
            }

            for (int i = 0; i < 16; ++i)
                module->paramQuantities[LFO::STEP_SEQUENCER_STEP_0 + i]->minValue = minStepSlider;
        }
    }
    void drawWaveform(NVGcontext *vg)
    {
        auto lfodata = module->lfostorageDisplay;
        auto storage = module->storage.get();

        tp[lfodata->delay.param_id_in_scene].i = lfodata->delay.val.i;
        tp[lfodata->attack.param_id_in_scene].i = lfodata->attack.val.i;
        tp[lfodata->hold.param_id_in_scene].i = lfodata->hold.val.i;
        tp[lfodata->decay.param_id_in_scene].i = lfodata->decay.val.i;
        tp[lfodata->sustain.param_id_in_scene].i = lfodata->sustain.val.i;
        tp[lfodata->release.param_id_in_scene].i = lfodata->release.val.i;

        tp[lfodata->magnitude.param_id_in_scene].i = lfodata->magnitude.val.i;
        tp[lfodata->rate.param_id_in_scene].i = lfodata->rate.val.i;
        tp[lfodata->shape.param_id_in_scene].i = lfodata->shape.val.i;
        tp[lfodata->unipolar.param_id_in_scene].i = lfodata->unipolar.val.i;
        tp[lfodata->start_phase.param_id_in_scene].i = lfodata->start_phase.val.i;
        tp[lfodata->deform.param_id_in_scene].i = lfodata->deform.val.i;
        tp[lfodata->trigmode.param_id_in_scene].i = lm_keytrigger;

        float susTime = 0.5;
        bool msegRelease = false;
        float msegReleaseAt = 0;
        float lfoEnvelopeDAHDTime =
            pow(2.0f, lfodata->delay.val.f) + pow(2.0f, lfodata->attack.val.f) +
            pow(2.0f, lfodata->hold.val.f) + pow(2.0f, lfodata->decay.val.f);

        float totalEnvTime =
            lfoEnvelopeDAHDTime + std::min(pow(2.0f, lfodata->release.val.f), 4.f) +
            0.5; // susTime; this is now 0.5 to keep the envelope fixed in gate mode

        float rateInHz = pow(2.0, (double)lfodata->rate.val.f);
        if (lfodata->rate.temposync)
            rateInHz *= storage->temposyncratio;

        bool isuni = lfodata->unipolar.val.b;

        /*
         * What we want is no more than 50 wavelengths. So
         *
         * totalEnvTime * rateInHz < 50
         *
         * totalEnvTime < 50 / rateInHz
         *
         * so
         */
        totalEnvTime = std::min(totalEnvTime, 50.f / rateInHz);

        std::unique_ptr<LFOModulationSource> tlfo = std::make_unique<LFOModulationSource>();
        tlfo->assign(storage, lfodata, tp, nullptr, module->surge_ss.get(), module->surge_ms.get(),
                     module->surge_fs.get(), true);
        tlfo->attack();

        bool hasFullWave = false, waveIsAmpWave = false;

        bool drawEnvelope = !lfodata->delay.deactivated;

        int minSamples = (1 << 0) * (int)(box.size.x);
        int totalSamples =
            std::max((int)minSamples, (int)(totalEnvTime * storage->samplerate / BLOCK_SIZE));
        float drawnTime = totalSamples * storage->samplerate_inv * BLOCK_SIZE;

        // OK so let's assume we want about 1000 pixels worth tops in
        int averagingWindow = (int)(totalSamples / 1000.0) + 1;

        float valScale = 100.0;
        int susCountdown = -1;

        float priorval = 0.f, priorwval = 0.f;

        std::string invalidMessage;

        std::vector<std::pair<float, float>> pathV, eupathV, edpathV;

        float scaleComp = 0.9;
        float scaleOff = (1.0 - scaleComp) * 0.5;

        for (int i = 0; i < totalSamples; i += averagingWindow)
        {
            float val = 0;
            float wval = 0;
            float eval = 0;
            float minval = 1000000, minwval = 1000000;
            float maxval = -1000000, maxwval = -1000000;
            float firstval;
            float lastval;

            for (int s = 0; s < averagingWindow; s++)
            {
                tlfo->process_block();

                if (susCountdown < 0 && tlfo->env_state == lfoeg_stuck)
                {
                    susCountdown = susTime * storage->samplerate / BLOCK_SIZE;
                }
                else if (susCountdown == 0 && tlfo->env_state == lfoeg_stuck)
                {
                    tlfo->release();
                }
                else if (susCountdown > 0)
                {
                    susCountdown--;
                }

                val += tlfo->get_output(0);

                if (s == 0)
                {
                    firstval = tlfo->get_output(0);
                }

                if (s == averagingWindow - 1)
                {
                    lastval = tlfo->get_output(0);
                }

                minval = std::min(tlfo->get_output(0), minval);
                maxval = std::max(tlfo->get_output(0), maxval);
                eval += tlfo->env_val * lfodata->magnitude.get_extended(lfodata->magnitude.val.f);
            }

            val = val / averagingWindow;
            wval = wval / averagingWindow;
            eval = eval / averagingWindow;
            val = ((-val + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
            wval = ((-wval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
            minwval = ((-minwval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
            maxwval = ((-maxwval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;

            float euval = ((-eval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
            float edval = ((eval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
            float xc = valScale * i / totalSamples;

            if (i == 0)
            {
                pathV.emplace_back(xc, val);
                eupathV.emplace_back(xc, euval);
                edpathV.emplace_back(xc, edval);

                priorval = val;
                priorwval = wval;
            }
            else
            {
                minval = ((-minval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
                maxval = ((-maxval + 1.0f) * 0.5 * scaleComp + scaleOff) * valScale;
                // Windows is sensitive to out-of-order line draws in a way which causes spikes.
                // Make sure we draw one closest to prior first. See #1438
                float firstval = minval;
                float secondval = maxval;

                if (priorval - minval < maxval - priorval)
                {
                    firstval = maxval;
                    secondval = minval;
                }

                pathV.emplace_back(xc - scaleOff * valScale / totalSamples, firstval);
                pathV.emplace_back(xc + scaleOff * valScale / totalSamples, secondval);

                priorval = val;
                eupathV.emplace_back(xc, euval);
                edpathV.emplace_back(xc, edval);
            }
        }

        tlfo->completedModulation();

        auto sx = box.size.x / valScale, sy = box.size.y / valScale;
        if (isuni)
            sy = sy * 2;
        for (const auto &pt : {eupathV, edpathV})
        {
            bool first{true};
            nvgBeginPath(vg);
            for (const auto &[x, y] : pt)
            {
                if (first)
                {
                    nvgMoveTo(vg, sx * x, sy * y);
                }
                else
                {
                    nvgLineTo(vg, sx * x, sy * y);
                }
                first = false;
            }
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
        }

        if (isuni)
        {
            nvgBeginPath(vg);
            auto y = (0.5 * scaleComp + scaleOff) * valScale;
            nvgMoveTo(vg, sx * 0, sy * y);
            nvgLineTo(vg, sx * valScale, sy * y);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
        }
        // Now fill the actual path with gradients then stroke the line
        auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
        auto gcp = col;
        gcp.a = 0.5;
        auto gcn = col;
        gcn.a = 0.0;

        for (int i = 0; i < 3; ++i)
        {
            if (isuni && i == 1)
                continue;
            bool first{true};
            nvgBeginPath(vg);

            for (const auto &[x, yorig] : pathV)
            {
                auto y = yorig;
                if (isuni)
                {
                }
                else if (i == 0)
                    y = std::max(sy * yorig, box.size.y * 0.5f) / sy;
                else if (i == 1)
                    y = std::min(sy * yorig, box.size.y * 0.5f) / sy;
                if (first)
                {
                    nvgMoveTo(vg, sx * x, sy * y);
                }
                else
                {
                    nvgLineTo(vg, sx * x, sy * y);
                }
                first = false;
            }
            if (i == 2)
            {
                nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
                nvgStrokeWidth(vg, 1.25);
                nvgLineJoin(vg, NVG_ROUND);
                nvgLineCap(vg, NVG_BUTT);
                nvgStroke(vg);
            }
            else
            {
                if (isuni)
                {
                    nvgLineTo(vg, box.size.x, box.size.y);
                    nvgLineTo(vg, 0, box.size.y);
                    std::swap(gcp, gcn);
                    nvgFillPaint(
                        vg, nvgLinearGradient(vg, 0, box.size.y * 0.2, 0, box.size.y, gcn, gcp));
                    // nvgFillColor(vg, nvgRGB(255, 0, 0));
                }
                else if (i == 1)
                {
                    nvgLineTo(vg, box.size.x, box.size.y * 0.5);
                    nvgLineTo(vg, 0, box.size.y * 0.5);
                    nvgFillPaint(vg, nvgLinearGradient(vg, 0, box.size.y * 0.2, 0, box.size.y * 0.5,
                                                       gcp, gcn));
                }
                else
                {
                    nvgLineTo(vg, box.size.x, box.size.y * 0.5);
                    nvgLineTo(vg, 0, box.size.y * 0.5);
                    nvgFillPaint(vg, nvgLinearGradient(vg, 0, box.size.y * 0.5, 0, box.size.y * 0.8,
                                                       gcn, gcp));
                }
                nvgFill(vg);
            }
        }

#if 0
        if (tFullWave)
        {
            tFullWave->completedModulation();
            delete tFullWave;
        }

        auto at = juce::AffineTransform()
                      .scale(waveform_display.getWidth() / valScale,
                             waveform_display.getHeight() / valScale)
                      .translated(waveform_display.getTopLeft());

        for (int i = -1; i < 2; ++i)
        {
            float off = i * 0.4 + 0.5; // so that/s 0.1, 0.5, 0.9
            float x0 = 0, y0 = valScale * off, x1 = valScale, y1 = valScale * off;

            at.transformPoint(x0, y0);
            at.transformPoint(x1, y1);

            if (i == 0)
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::Center));
            }
            else
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::Bounds));
            }

            g.drawLine(x0, y0, x1, y1, 1);
        }

        auto pointColor = skin->getColor(Colors::LFO::Waveform::Dots);
        int nxd = 61, nyd = 9;

        for (int xd = 0; xd < nxd; xd++)
        {
            float normx = 1.f * xd / (nxd - 1) * 0.99;
            for (int yd = 1; yd < nyd - 1; yd++)
            {
                if (yd == (nyd - 1) / 2)
                    continue;

                float normy = 1.f * yd / (nyd - 1);
                float dotPointX = normx * valScale, dotPointY = (0.8 * normy + 0.1) * valScale;
                at.transformPoint(dotPointX, dotPointY);
                float esize = 1.f;
                float xoff = (xd == 0 ? esize : 0);
                g.setColour(pointColor);
                g.fillEllipse(dotPointX - esize, dotPointY - esize, esize, esize);
            }
        }

        if (drawEnvelope)
        {
            g.setColour(skin->getColor(Colors::LFO::Waveform::Envelope));
            g.strokePath(eupath, juce::PathStrokeType(1.f), at);

            if (!isUnipolar())
            {
                g.strokePath(edpath, juce::PathStrokeType(1.f), at);
            }
        }

        if (drawBeats)
        {
            // calculate beat grid related data
            auto bpm = storage->temposyncratio * 120;

            auto denFactor = 4.0 / tsDen;
            auto beatsPerSecond = bpm / 60.0;
            auto secondsPerBeat = 1 / beatsPerSecond;
            auto deltaBeat = secondsPerBeat / drawnTime * valScale * denFactor;

            int nBeats = std::max(ceil(drawnTime * beatsPerSecond / denFactor), 1.0);

            auto measureLen = deltaBeat * tsNum;
            int everyMeasure = 1;

            while (measureLen < 4) // a hand selected parameter by playing around with it, tbh
            {
                measureLen *= 2;
                everyMeasure *= 2;
            }

            for (auto l = 0; l <= nBeats; ++l)
            {
                auto xp = deltaBeat * l;

                if (l % (tsNum * everyMeasure) == 0 || nBeats <= tsDen)
                {
                    auto soff = 0.0;
                    if (l > 10)
                        soff = 0.0;
                    float vruleSX = xp, vruleSY = valScale * .15, vruleEX = xp,
                          vruleEY = valScale * .85;
                    at.transformPoint(vruleSX, vruleSY);
                    at.transformPoint(vruleEX, vruleEY);
                    // major beat divisions on the LFO waveform bg
                    g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::ExtendedTicks));
                    g.drawLine(vruleSX, vruleSY, vruleEX, vruleEY, 1);

                    juce::Point<float> sp(xp, valScale * (.01)), ep(xp, valScale * (.1));
                    sp = sp.transformedBy(at);
                    ep = ep.transformedBy(at);
                    // ticks for major beats
                    if (l % tsNum == 0)
                        g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Ticks));
                    else
                        // small ticks for the ruler for nbeats case
                        g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::SmallTicks));

                    g.drawLine(sp.getX(), sp.getY(), ep.getX(), ep.getY(), 1.0);

                    char s[TXT_SIZE];
                    snprintf(s, TXT_SIZE, "%d", l + 1);

                    juce::Point<int> tp(xp + 1, valScale * 0.0);
                    tp = tp.transformedBy(at);
                    g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Text));
                    g.setFont(skin->fontManager->lfoTypeFont);
                    g.drawText(s, tp.x, tp.y, 20, 10, juce::Justification::bottomLeft);
                }
                else if (everyMeasure == 1)
                {
                    auto sp = juce::Point<float>(xp, valScale * 0.06).transformedBy(at);
                    auto ep = juce::Point<float>(xp, valScale * 0.1).transformedBy(at);

                    if (l % tsNum == 0)
                        g.setColour(juce::Colours::black);
                    else
                        // small ticks for the ruler
                        g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::SmallTicks));
                    g.drawLine(sp.x, sp.y, ep.x, ep.y, 0.5);
                }
            }
        }

        if (hasFullWave)
        {
            if (waveIsAmpWave)
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::GhostedWave));
                auto dotted = juce::Path();
                auto st = juce::PathStrokeType(0.3, juce::PathStrokeType::beveled,
                                               juce::PathStrokeType::butt);
                float dashLength[2] = {4.f, 2.f};
                st.createDashedStroke(dotted, deactPath, dashLength, 2, at);
                g.strokePath(dotted, st);
            }
            else
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::DeactivatedWave));
                g.strokePath(deactPath,
                             juce::PathStrokeType(0.5f, juce::PathStrokeType::beveled,
                                                  juce::PathStrokeType::butt),
                             at);
            }
        }

        g.setColour(skin->getColor(Colors::LFO::Waveform::Wave));
        g.strokePath(
            path,
            juce::PathStrokeType(1.f, juce::PathStrokeType::beveled, juce::PathStrokeType::butt),
            at);

        // lower ruler calculation
        // find time delta
        int maxNumLabels = 5;
        std::vector<float> timeDeltas = {0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0};
        auto currDelta = timeDeltas.begin();
        while (currDelta != timeDeltas.end() && (drawnTime / *currDelta) > maxNumLabels)
        {
            currDelta++;
        }
        float delta = timeDeltas.back();
        if (currDelta != timeDeltas.end())
            delta = *currDelta;

        int nLabels = (int)(drawnTime / delta) + 1;
        float dx = delta / drawnTime * valScale;

        for (int l = 0; l < nLabels; ++l)
        {
            float xp = dx * l;
            float yp = valScale * 0.9;
            float typ = yp;
            auto tp = juce::Point<float>(xp + 0.5, typ + 0.5).transformedBy(at);
            g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Text));
            g.setFont(skin->fontManager->lfoTypeFont);
            char txt[TXT_SIZE];
            float tv = delta * l;
            if (fabs(roundf(tv) - tv) < 0.05)
                snprintf(txt, TXT_SIZE, "%d s", (int)round(delta * l));
            else if (delta < 0.1)
                snprintf(txt, TXT_SIZE, "%.2f s", delta * l);
            else
                snprintf(txt, TXT_SIZE, "%.1f s", delta * l);
            g.drawText(txt, tp.x, tp.y, 30, 10, juce::Justification::topLeft);

            auto sp = juce::Point<float>(xp, valScale * 0.95).transformedBy(at);
            auto ep = juce::Point<float>(xp, valScale * 0.9).transformedBy(at);

            g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Ticks));
            g.drawLine(sp.x, sp.y, ep.x, ep.y, 1.0);
        }

        /*
         * In 1.8 I think we don't want to show the key release point in the simulated wave
         * with the MSEG but I wrote it to debug and we may change our mind so keeping this code
         * here
         */
        if (msegRelease && false)
        {
#if SHOW_RELEASE_TIMES
            float xp = msegReleaseAt / drawnTime * valScale;
            was a vstgui Point sp(xp, valScale * 0.9), ep(xp, valScale * 0.1);
            tf.transform(sp);
            tf.transform(ep);
            dc->setFrameColor(juce::Colours::red);
            dc->drawLine(sp, ep);
#endif
        }

        if (warnForInvalid)
        {
            g.setColour(skin->getColor(Colors::LFO::Waveform::Wave));
            g.setFont(skin->fontManager->getLatoAtSize(14, juce::Font::bold));
            g.drawText(invalidMessage, waveform_display.withTrimmedBottom(30),
                       juce::Justification::centred);
        }
#endif
    }
};

LFOWidget::LFOWidget(LFOWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    int screwWidth = 20;
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * screwWidth, rack::app::RACK_GRID_HEIGHT);

    typedef layout::LayoutEngine<LFOWidget, M::RATE> engine_t;
    engine_t::initializeModulationToBlank(this);

    auto bg = new widgets::Background(box.size, "LFO x EG", "other", "EGLFO");
    addChild(bg);

    typedef layout::LayoutItem li_t;

    {
        const auto col = layout::LayoutConstants::firstColumnCenter_MM;
        const auto cw = layout::LayoutConstants::lfoColumnWidth_MM;
        const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1];
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::KNOB9, "RATE", M::RATE, col, row1},
                {li_t::KNOB9, "PHASE", M::PHASE, col + cw, row1},
                {li_t::KNOB9, "DEFORM", M::DEFORM, col + 2 * cw, row1},
                {li_t::KNOB9, "AMP", M::AMPLITUDE, col + 3 * cw, row1},
                li_t::createGrouplabel("WAVEFORM", col, row1, 4, cw)
         };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }
    }

    {
        const auto knobCenter = layout::LayoutConstants::vcoRowCenters_MM[1];
        // We want the top of the sliders and top of the knobs to align
        const auto row1 = knobCenter + 4.75f;

        const auto middleOutputCenter = layout::LayoutConstants::firstColumnCenter_MM +
                                        layout::LayoutConstants::lfoColumnWidth_MM * 5;

        float cw = layout::LayoutConstants::vSliderSpacing_MM;
        float c0 = middleOutputCenter - 2.5 * cw;
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::VSLIDER, "D", M::E_DELAY, c0, row1},
                {li_t::VSLIDER, "A", M::E_ATTACK, c0 + cw, row1},
                {li_t::VSLIDER, "H", M::E_HOLD, c0 + 2 * cw, row1},
                {li_t::VSLIDER, "D", M::E_DECAY, c0 + 3 * cw, row1},
                {li_t::VSLIDER, "S", M::E_SUSTAIN, c0 + 4 * cw, row1},
                {li_t::VSLIDER, "R", M::E_RELEASE, c0 + 5 *cw, row1},
        };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }

        // OK this is a bit hacky. We know we want 40mm centered over the middle
        auto gl = new widgets::GroupLabel();
        auto ht = rack::mm2px(4.5);
        auto yup = rack::mm2px(1.75);
        gl->label = "ENVELOPE";
        gl->box.pos.x = rack::mm2px(middleOutputCenter - 20);
        gl->box.size.x = rack::mm2px(40);
        gl->box.pos.y = rack::mm2px(knobCenter - 8) - yup;
        gl->box.size.y = ht;
        gl->setup();
        addChild(gl);
    }

    {
        int col = 0;
        std::vector<std::string> labv{"GATE", "GATEENV", "CLOCK", "PHASE"};
        for (auto p :
             {M::INPUT_GATE, M::INPUT_GATE_ENVONLY, M::INPUT_CLOCK_RATE, M::INPUT_PHASE_DIRECT})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::lfoColumnWidth_MM * col;
            addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                              module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto lab = engine_t::makeLabelAt(bl, col, labv[col], style::XTStyle::TEXT_LABEL,
                                             layout::LayoutConstants::lfoColumnWidth_MM);
            if (p == M::INPUT_CLOCK_RATE)
            {
                lab->module = module;
                lab->hasDynamicLabel = true;
                lab->dynamicLabel = [](auto *xm) {
                    if (!xm)
                        return "CLOCK";
                    auto m = static_cast<LFO *>(xm);
                    typedef modules::ClockProcessor<LFO> cp_t;

                    if (m->clockProc.clockStyle == cp_t::QUARTER_NOTE)
                        return "CLOCK";
                    else
                        return "BPM";
                };
            }
            addChild(lab);

            col++;
        }
    }

    auto ht = layout::LayoutConstants::rowStart_MM - 11;
    auto lcd = widgets::LCDBackground::createWithHeight(ht, screwWidth);
    if (!module)
    {
        lcd->noModuleText = "LFO x EG";
        lcd->noModuleSize = 30;
    }
    addChild(lcd);

    auto tw = LFOTypeWidget::create(screwWidth, module, M::SHAPE);
    addChild(tw);

    auto ww = new LFOWaveform();
    ww->module = module;
    ww->box.pos.x = tw->box.pos.x;
    ww->box.pos.y = tw->box.pos.y + tw->box.size.y;
    ww->box.size.x = tw->box.size.x;
    ww->box.size.y = rack::mm2px(ht) - ww->box.pos.y - rack::mm2px(6);
    ww->setup();
    addChild(ww);
    wavePlot = ww;

    auto ws = new LFOStepWidget();
    ws->module = module;
    ws->box.pos.x = tw->box.pos.x;
    ws->box.pos.y = tw->box.pos.y + tw->box.size.y;
    ws->box.size.x = tw->box.size.x;
    ws->box.size.y = rack::mm2px(ht) - ww->box.pos.y - rack::mm2px(6);
    ws->setup();
    addChild(ws);
    ws->setVisible(false);
    stepEditor = ws;

    auto gutterX = ww->box.pos.x;
    auto gutterY = ww->box.pos.y + ww->box.size.y - rack::mm2px(0.5);
    auto gutterHeight = rack::mm2px(5);
    auto uni = widgets::PlotAreaSwitch::create(
        rack::Vec(gutterX, gutterY), rack::Vec(22, gutterHeight), "UNI", module, M::UNIPOLAR);
    addChild(uni);
    gutterX += 22 + 3;

    stepStart = widgets::LabeledPlotAreaControl::create(rack::Vec(gutterX, gutterY),
                                                        rack::Vec(42, gutterHeight), "START",
                                                        module, M::STEP_SEQUENCER_START);
    addChild(stepStart);
    stepStart->setVisible(false);
    gutterX += 42 + 3;
    stepEnd = widgets::LabeledPlotAreaControl::create(rack::Vec(gutterX, gutterY),
                                                      rack::Vec(35, gutterHeight), "END", module,
                                                      M::STEP_SEQUENCER_END);
    addChild(stepEnd);
    stepEnd->setVisible(false);
    gutterX += 35 + 3;

    {
        int col = 0;
        std::vector<std::string> labv{"LFO", "EG", "LFOxEG"};
        for (auto p : {M::OUTPUT_WAVE, M::OUTPUT_ENV, M::OUTPUT_MIX})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::lfoColumnWidth_MM * (3 + col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto cx = xp;

            auto boxx0 = cx - layout::LayoutConstants::lfoColumnWidth_MM * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));

            auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::lfoColumnWidth_MM, 5));

            auto lab = widgets::Label::createWithBaselineBox(
                p0, s0, labv[col], layout::LayoutConstants::labelSize_pt,
                style::XTStyle::Colors::TEXT_LABEL_OUTPUT);
            addChild(lab);

            col++;
        }
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIGA", "TRIGB", "EOC"};
        for (auto p : {M::OUTPUT_TRIGA, M::OUTPUT_TRIGB, M::OUTPUT_TRIGPHASE})
        {
            auto yp = layout::LayoutConstants::modulationRowCenters_MM[1];
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::lfoColumnWidth_MM * (3 + col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = yp - 5;
            auto cx = xp;

            auto boxx0 = cx - layout::LayoutConstants::lfoColumnWidth_MM * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::lfoColumnWidth_MM, 5));

            auto lab = widgets::Label::createWithBaselineBox(
                p0, s0, labv[col], layout::LayoutConstants::labelSize_pt,
                style::XTStyle::Colors::TEXT_LABEL_OUTPUT);
            if (p == M::OUTPUT_TRIGA)
            {
                lab->module = module;
                lab->hasDynamicLabel = true;
                lab->dynamicLabel = [](auto *m) {
                    if (!m)
                        return "TRIGA";
                    auto lf = static_cast<LFO *>(m);
                    auto t = lf->lfostorage->shape.val.i;
                    if (t == lt_stepseq)
                    {
                        return "TRIGA";
                    }
                    else
                    {
                        return "EOSEG";
                    }
                };
            }
            if (p == M::OUTPUT_TRIGB)
            {
                lab->module = module;
                lab->hasDynamicLabel = true;
                lab->dynamicLabel = [](auto *m) {
                    if (!m)
                        return "TRIGB";
                    auto lf = static_cast<LFO *>(m);
                    auto t = lf->lfostorage->shape.val.i;
                    if (t == lt_stepseq)
                    {
                        return "TRIGB";
                    }
                    else
                    {
                        return "EOEG";
                    }
                };
            }
            addChild(lab);

            col++;
        }
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::LFO_MOD_INPUT, 0,
                                   layout::LayoutConstants::lfoColumnWidth_MM);

    resetStyleCouplingToModule();
}

void LFOWidget::step()
{
    if (module)
    {
        auto lm = static_cast<LFO *>(module);
        auto sh = lm->lfostorage->shape.val.i;

        if (sh != priorShape)
        {
            priorShape = sh;

            auto stepSide = (sh == lt_stepseq);
            if (wavePlot)
                wavePlot->setVisible(!stepSide);
            if (stepEditor)
                stepEditor->setVisible(stepSide);
            if (stepStart)
                stepStart->setVisible(stepSide);
            if (stepEnd)
                stepEnd->setVisible(stepSide);
        }
    }
    widgets::XTModuleWidget::step();
}
} // namespace sst::surgext_rack::lfo::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeLFO = rack::createModel<sst::surgext_rack::lfo::ui::LFOWidget::M,
                                               sst::surgext_rack::lfo::ui::LFOWidget>("SurgeXTLFO");
