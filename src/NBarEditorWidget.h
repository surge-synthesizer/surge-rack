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

#ifndef SURGE_RACK_NBAREDITORWIDGET_H
#define SURGE_RACK_NBAREDITORWIDGET_H

#include "rack.hpp"
#include "XTStyle.h"

namespace sst::surgext_rack::widgets
{

template <int bars> struct NBarWidget : public rack::Widget, style::StyleParticipant
{
    /*
     * TODOS:
     * - RMB for direct edit
     * - Shift, Ctrl, etc... gestures
     */
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    modules::XTModule *module{nullptr};
    int par0;
    float valCache[3][bars];
    bool activateCache[bars];
    std::function<bool(modules::XTModule *m, int idx)> isActive = [](auto m, auto i) {
        return true;
    };
    std::string barLabel = "Step";
    std::function<void(NBarWidget *w, rack::Menu *m)> makeAdditionalMenu = [](auto w, auto m) {};

    NBarWidget()
    {
        memset(valCache, 0, 3 * sizeof(float) * bars);
        memset(activateCache, 0, sizeof(bool) * bars);
    }

    static NBarWidget<bars> *create(const rack::Vec &pos, const rack::Vec &size,
                                    modules::XTModule *module, int par0)
    {
        auto res = new NBarWidget<bars>;
        res->box.pos = pos;
        res->box.size = size;
        res->module = module;
        res->par0 = par0;

        res->bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                           [res](auto vg) { res->drawBG(vg); });
        res->addChild(res->bdw);
        res->bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), size, [res](auto vg) { res->drawLights(vg); });
        res->addChild(res->bdwLight);

        return res;
    }

    void drawBars(NVGcontext *vg, bool fill)
    {
        auto dx = box.size.x / bars;
        auto tickHeight = rack::mm2px(0.25);

        for (int i = 0; i < bars; ++i)
        {
            if (module && module->paramQuantities[par0 + i])
            {
                auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
                if (!isActive(module, i))
                    col = style()->getColor(style::XTStyle::PLOT_MARKS);
                auto gcp = col;
                gcp.a = 0.0;
                auto gcn = col;
                gcn.a = 0.9;

                auto pq = module->paramQuantities[par0 + i];
                bool uni{false};
                auto mx = pq->getMaxValue();
                auto mn = pq->getMinValue();
                if (mn > -0.1)
                    uni = true;

                if (uni)
                {
                    auto sc = 1.0 - pq->getValue() / (mx - mn);
                    nvgBeginPath(vg);
                    if (fill)
                    {
                        nvgRect(vg, dx * i, sc * box.size.y, dx, (1 - sc) * box.size.y);
                        nvgFillPaint(
                            vg, nvgLinearGradient(vg, 0, sc * box.size.y, 0, box.size.y, gcn, gcp));
                        nvgFill(vg);

                        nvgBeginPath(vg);
                        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
                        nvgMoveTo(vg, dx * i, box.size.y);
                        nvgLineTo(vg, dx * (i + 1), box.size.y);
                        nvgStrokeWidth(vg, 0.75);
                        nvgStroke(vg);
                    }
                    else
                    {
                        nvgRect(vg, dx * i, sc * box.size.y - tickHeight, dx, 2 * tickHeight);
                        nvgFillColor(vg, col);
                        nvgFill(vg);
                    }
                }
                else
                {
                    auto sc = (-pq->getValue()) / (mx - mn);
                    if (sc < 0)
                    {
                        nvgBeginPath(vg);
                        auto start = box.size.y * 0.5 + sc * box.size.y;
                        auto ht = box.size.y * 0.5 - start;
                        if (fill)
                        {
                            nvgRect(vg, dx * i, start, dx, ht);
                            nvgFillPaint(vg,
                                         nvgLinearGradient(vg, 0, start, 0, start + ht, gcn, gcp));
                            nvgFill(vg);
                        }
                        else
                        {
                            nvgRect(vg, dx * i, start - tickHeight, dx, tickHeight * 2);
                            nvgFillColor(vg, col);
                            nvgFill(vg);
                        }
                    }
                    else
                    {
                        nvgBeginPath(vg);
                        if (fill)
                        {
                            nvgRect(vg, dx * i, box.size.y * 0.5, dx, sc * box.size.y);
                            nvgFillPaint(vg, nvgLinearGradient(vg, 0, box.size.y * 0.5, 0,
                                                               (0.5 + sc) * box.size.y, gcp, gcn));
                            nvgFill(vg);
                        }
                        else
                        {
                            nvgRect(vg, dx * i, box.size.y * (0.5 + sc) - tickHeight, dx,
                                    2 * tickHeight);
                            nvgFillColor(vg, col);
                            nvgFill(vg);
                        }
                    }

                    if (fill)
                    {
                        nvgBeginPath(vg);
                        nvgMoveTo(vg, dx * i, box.size.y * 0.5);
                        nvgLineTo(vg, dx * (i + 1), box.size.y * 0.5);
                        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
                        nvgStrokeWidth(vg, 0.75);
                        nvgStroke(vg);

                        nvgBeginPath(vg);
                        nvgMoveTo(vg, dx * i, box.size.y);
                        nvgLineTo(vg, dx * (i + 1), box.size.y);
                        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
                        nvgStrokeWidth(vg, 0.75);
                        nvgStroke(vg);
                    }
                }
            }
        }
    }
    void drawBG(NVGcontext *vg)
    {
        auto dx = box.size.x / bars;
        for (int i = 0; i <= bars; ++i)
        {
            nvgBeginPath(vg);
            nvgMoveTo(vg, (i)*dx, 0);
            nvgLineTo(vg, (i)*dx, box.size.y);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }

        drawBars(vg, true);
    }

    void drawLights(NVGcontext *vg) { drawBars(vg, false); }

    rack::Vec currentEditPos;
    rack::ui::Tooltip *toolTip{nullptr};
    double removeTooltipAfter{-1};
    bool ignoreMove{true};
    int dragMods{0};
    void onButton(const ButtonEvent &e) override
    {
        if (e.action == GLFW_PRESS)
        {
            if (e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
            {
                createRightMouseMenu(e.pos.x);
                e.consume(this);
                return;
            }
            dragMods = (e.mods & RACK_MOD_MASK);
            currentEditPos = e.pos;
            ignoreMove = false;
            updateFromEditPosition();

            e.consume(this);
        }

        if (e.action == GLFW_RELEASE)
        {
            ignoreMove = true;
            // removeTooltipAfter = rack::system::getTime() + 0.5;
            e.consume(this);
        }
    }

    void onHover(const HoverEvent &e) override
    {
        if (toolTip)
        {
            auto dx = box.size.x / bars;
            auto b = std::clamp((int)(e.pos.x / dx), 0, bars - 1);
            setTooltipText(b);
        }
        e.consume(this);
    }
    void onEnter(const EnterEvent &e) override
    {
        if (!toolTip && rack::settings::tooltips)
        {
            toolTip = new rack::ui::Tooltip;
            removeTooltipAfter = -1;
            rack::Widget *p = this;
            while (p->getParent())
                p = p->getParent();
            p->addChild(toolTip);
        }
        e.consume(this);
    }
    void onLeave(const LeaveEvent &e) override
    {
        removeTooltipAfter = rack::system::getTime() + 0.05;
        e.consume(this);
    }

    // Basically ParamWidget text field adapted to this multi-approach
    struct StepTextField : rack::ui::TextField
    {
        void step() override
        {
            // Keep selected
            APP->event->setSelectedWidget(this);
            TextField::step();
        }

        NBarWidget<bars> *container{nullptr};
        modules::XTModule *module{nullptr};
        int par0{0};
        int idx{0};
        void setStep(NBarWidget<bars> *c, modules::XTModule *m, int p, int s)
        {
            container = c;
            module = m;
            par0 = p;
            idx = s;
            auto pq = module->paramQuantities[par0 + idx];
            text = pq->getDisplayValueString();
        }

        void onSelectKey(const SelectKeyEvent &e) override
        {
            if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER))
            {
                auto val = std::atof(text.c_str());
                container->setBarTo(idx, val);
                rack::ui::MenuOverlay *overlay = getAncestorOfType<rack::ui::MenuOverlay>();
                overlay->requestDelete();
                e.consume(this);
            }

            if (!e.getTarget())
                TextField::onSelectKey(e);
        }
    };

    void createRightMouseMenu(float xp)
    {
        if (!module)
            return;
        auto dx = box.size.x / bars;
        auto b = std::clamp((int)(xp / dx), 0, bars - 1);
        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel("Set " + barLabel + " " + std::to_string(b)));

        auto tc = new StepTextField;
        tc->box.size.x = 100;
        tc->setStep(this, module, par0, b);
        menu->addChild(tc);

        menu->addChild(new rack::MenuSeparator);
        menu->addChild(rack::createMenuLabel("Set to..."));
        menu->addChild(rack::createMenuItem("Zero", "", [this, b]() { setBarTo(b, 0.f); }));
        if (module->paramQuantities[par0 + b]->getDefaultValue() != 0)
        {
            menu->addChild(rack::createMenuItem("Default", "", [this, b]() {
                setBarTo(b, module->paramQuantities[par0 + b]->getDefaultValue());
            }));
        }
        menu->addChild(rack::createMenuItem("Max", "", [this, b]() {
            setBarTo(b, module->paramQuantities[par0 + b]->getMaxValue());
        }));
        menu->addChild(rack::createMenuItem("Min", "", [this, b]() {
            setBarTo(b, module->paramQuantities[par0 + b]->getMinValue());
        }));

        makeAdditionalMenu(this, menu);
    }

    void setTooltipText(int bar)
    {
        if (!toolTip)
            return;
        if (!module)
            return;
        auto pq = module->paramQuantities[par0 + bar];
        toolTip->text = fmt::format("{} {}: {}", barLabel, bar, pq->getDisplayValueString());
    }
    void setBarTo(int bar, float val)
    {
        if (!module)
            return;
        auto pq = module->paramQuantities[par0 + bar];
        if (val != pq->getValue())
        {
            auto *h = new rack::history::ParamChange;
            h->name = fmt::format("change {} {} to {}", barLabel, bar, val);
            h->moduleId = pq->module->id;
            h->paramId = pq->paramId;
            h->oldValue = pq->getValue();
            h->newValue = val;
            APP->history->push(h);
        }
        pq->setValue(val);
        setTooltipText(bar);

        bdw->dirty = true;
        bdwLight->dirty = true;
    }

    void onDragMove(const DragMoveEvent &e) override
    {
        if (ignoreMove)
            return;

        auto fac = 1.0;
        if (dragMods & GLFW_MOD_SHIFT)
            fac = 0.1;
        currentEditPos.x += e.mouseDelta.x / getAbsoluteZoom();
        currentEditPos.y += fac * e.mouseDelta.y / getAbsoluteZoom();
        updateFromEditPosition();
        e.consume(this);
    }

    void onDoubleClick(const DoubleClickEvent &e) override
    {
        if (module)
        {
            ignoreMove = true;
            auto dx = box.size.x / bars;
            auto bi = std::clamp((int)(currentEditPos.x / dx), 0, bars - 1);

            auto pq = module->paramQuantities[par0 + bi];

            setBarTo(bi, pq->getDefaultValue());
            e.consume(this);

            bdw->dirty = true;
            bdwLight->dirty = true;
        }
    }
    void updateFromEditPosition()
    {
        auto dx = box.size.x / bars;
        auto bi = std::clamp((int)(currentEditPos.x / dx), 0, bars - 1);
        auto py = 1 - std::clamp(currentEditPos.y / box.size.y, 0.f, 1.f);
        if (module)
        {
            // todo undo history
            auto pq = module->paramQuantities[par0 + bi];
            auto sv = py * (pq->getMaxValue() - pq->getMinValue()) + pq->getMinValue();

            setBarTo(bi, sv);
        }
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
    }

    void step() override
    {
        if (module)
        {
            for (int i = 0; i < bars; ++i)
            {
                auto pq = module->paramQuantities[par0 + i];
                if (pq)
                {
                    if (pq->getValue() != valCache[0][i] || pq->getMinValue() != valCache[1][i] ||
                        pq->getMaxValue() != valCache[2][i])
                    {
                        bdw->dirty = true;
                        bdwLight->dirty = true;
                    }
                    auto ia = isActive(module, i);
                    if (ia != activateCache[i])
                    {
                        bdw->dirty = true;
                        bdwLight->dirty = true;
                    }
                    valCache[0][i] = pq->getValue();
                    valCache[1][i] = pq->getMinValue();
                    valCache[2][i] = pq->getMaxValue();
                    activateCache[i] = ia;
                }
            }
        }
        if (toolTip && removeTooltipAfter > 0)
        {
            if (rack::system::getTime() > removeTooltipAfter)
            {
                removeTooltipAfter = -1;
                auto p = toolTip->getParent();
                if (p)
                    p->removeChild(toolTip);
                delete toolTip;
                toolTip = nullptr;
            }
        }
        rack::Widget::step();
    }
};

} // namespace sst::surgext_rack::widgets

#endif // RACK_HACK_NBAREDITORWIDGET_H
