//
// Created by Paul Walker on 9/4/22.
//

#ifndef SURGEXT_RACK_XTWIDGETS_H
#define SURGEXT_RACK_XTWIDGETS_H

#include <rack.hpp>
#include <iostream>
#include "XTStyle.hpp"
#include "XTModule.hpp"

namespace sst::surgext_rack::widgets
{

struct BufferedDrawFunctionWidget : virtual rack::FramebufferWidget
{
    typedef std::function<void(NVGcontext *)> drawfn_t;
    drawfn_t drawf;

    struct InternalBDW : rack::TransparentWidget
    {
        drawfn_t drawf;
        InternalBDW(rack::Rect box_, drawfn_t draw_) : drawf(draw_) { box = box_; }

        void draw(const DrawArgs &args) override { drawf(args.vg); }
    };

    InternalBDW *kid = nullptr;
    BufferedDrawFunctionWidget(rack::Vec pos, rack::Vec sz, drawfn_t draw_) : drawf(draw_)
    {
        box.pos = pos;
        box.size = sz;
        auto kidBox = rack::Rect(rack::Vec(0, 0), box.size);
        kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
    }
};

struct BufferedDrawFunctionWidgetOnLayer : BufferedDrawFunctionWidget
{
    int layer{1};
    BufferedDrawFunctionWidgetOnLayer(rack::Vec pos, rack::Vec sz, drawfn_t draw_, int ly = 1)
        : BufferedDrawFunctionWidget(pos, sz, draw_), layer(ly)
    {
    }

    void draw(const DrawArgs &args) override { return; }
    void drawLayer(const DrawArgs &args, int dl) override
    {
        if (dl == layer)
        {
            BufferedDrawFunctionWidget::draw(args);
        }
    }
};

struct DebugRect : rack::TransparentWidget
{
    static DebugRect *create(const rack::Vec &pos, const rack::Vec &size)
    {
        auto r = new DebugRect();
        r->box.pos = pos;
        r->box.size = size;
        return r;
    }
    void draw(const DrawArgs &args) override
    {
        nvgBeginPath(args.vg);
        nvgStrokeColor(args.vg, nvgRGB(255, 0, 0));
        nvgFillColor(args.vg, nvgRGBA(255, 255, 0, 40));
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgFill(args.vg);
        nvgStroke(args.vg);
    }
};

struct DebugPoint : rack::TransparentWidget
{
    static DebugPoint *create(const rack::Vec &pos)
    {
        auto r = new DebugPoint();
        r->box.pos = pos;
        r->box.pos.x -= 2;
        r->box.pos.y -= 2;
        r->box.size = rack::Vec(4, 4);
        return r;
    }
    void draw(const DrawArgs &args) override
    {
        nvgBeginPath(args.vg);
        nvgStrokeColor(args.vg, nvgRGB(255, 255, 255));
        nvgFillColor(args.vg, nvgRGBA(0, 255, 255, 40));
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgFill(args.vg);
        nvgStroke(args.vg);
    }
};

struct Label : BufferedDrawFunctionWidget, style::StyleParticipant
{
    std::string label{};
    float size{7.3};
    float tracking{0.0};
    float baselinePad{0};
    style::XTStyle::Colors color;
    Label(const rack::Vec &pos, const rack::Vec &sz)
        : BufferedDrawFunctionWidget(pos, sz, [this](auto vg) { drawLabel(vg); })
    {
        box.pos = pos;
        box.size = sz;
    }

    static Label *
    createWithBaselineBox(const rack::Vec &pos, const rack::Vec &size, const std::string lab,
                          float szInPt = 7.3,
                          style::XTStyle::Colors col = style::XTStyle::Colors::TEXT_LABEL)
    {
        auto padSz = size;
        padSz.y += 4;
        auto res = new Label(pos, padSz); // FIXME on that obv
        res->baselinePad = 4;
        res->label = lab;
        res->size = szInPt;
        res->color = col;
        return res;
    }

    void drawLabel(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, size * 96.0 / 72.0);
        nvgFillColor(vg, style()->getColor(color));
        nvgStrokeColor(vg, style()->getColor(color));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
        nvgTextLetterSpacing(vg, tracking);
        nvgText(vg, box.size.x * 0.5, box.size.y - baselinePad, label.c_str(), nullptr);

#define DEBUG_RECT 0
#if DEBUG_RECT
        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGB(255, 0, 0));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgStroke(vg);
#endif
    }

    void onStyleChanged() override { dirty = true; }
};

struct Background : public rack::TransparentWidget, style::StyleParticipant
{
    std::string panelName, groupName, title;
    std::function<void(NVGcontext *)> moduleSpecificDraw;

    static constexpr float mainLabelBaseline_MM = 6.298, mainLabelSize_PT = 11;
    Label *titleLabel{nullptr};

    Background(rack::Vec size, const std::string &t, const std::string &grp, const std::string &pn)
        : title(t), groupName(grp), panelName(pn)
    {
        box.size = size;
        onStyleChanged();
    }

    void drawStubLayer(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(0x20, 0x20, 0x20));
        nvgStrokeColor(vg, nvgRGB(0xFF, 0x00, 0x00));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFill(vg);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontId(vg));
        nvgFontSize(vg, 17);
        nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);
        nvgFillColor(vg, nvgRGB(0xFF, 0x90, 0x00));
        nvgText(vg, box.size.x * 0.5, box.size.y - 2, "Missing Panel", nullptr);
    }

    void onStyleChanged() override
    {
        auto childrenCopy = children;
        for (auto k : childrenCopy)
            removeChild(k);

        std::string asset =
            style()->skinAssetDir() + "/panels/" + groupName + "/" + panelName + ".svg";

        auto panelLogo = rack::Svg::load(rack::asset::plugin(pluginInstance, asset));
        if (panelLogo)
        {
            auto svgp = new rack::app::SvgPanel();
            svgp->box.pos = rack::Vec(0, 0);
            svgp->box.size = box.size;

            svgp->setBackground(panelLogo);
            addChild(svgp);
        }
        else
        {
            auto bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { drawStubLayer(vg); });
            addChild(bdw);
        }

        auto label = Label::createWithBaselineBox(
            rack::Vec(0, 0), rack::Vec(box.size.x, rack::mm2px(mainLabelBaseline_MM)), title,
            mainLabelSize_PT);
        label->tracking = 0.7;
        addChild(label);
    }
};

struct ModRingKnob;

struct KnobN : public rack::componentlibrary::RoundKnob, style::StyleParticipant
{
    static constexpr float ringWidth_MM = 0.7f;
    static constexpr float ringPad_MM = 0.5f;
    static constexpr float ringWidth_PX = 1.5;

    float knobSize_MM = -1;
    float pointerSize_MM = -1;
    std::string knobPointerAsset, knobBackgroundAsset;

    KnobN() {}

    bool isBipolar()
    {
        auto xtm = dynamic_cast<modules::XTModule *>(module);
        if (xtm)
            return xtm->isBipolar(paramId);
        return false;
    }

    float modDepthForAnimation()
    {
        auto xtm = dynamic_cast<modules::XTModule *>(module);
        if (xtm)
            return xtm->modulationDisplayValue(paramId);
        return 0;
    }

    bool isModEditing{false};
    void setIsModEditing(bool b)
    {
        isModEditing = b;
        bwValue->dirty = true;
        bw->dirty = true;
    }

    void completeConstructor()
    {
        float angleSpreadDegrees = 40.0;

        minAngle = -M_PI * (180 - angleSpreadDegrees) / 180;
        maxAngle = M_PI * (180 - angleSpreadDegrees) / 180;

        setupWidgets();
        fb->removeChild(shadow);
    }

    virtual void setupProperties() {}

    void drawRing(NVGcontext *vg)
    {
        float radius = rack::mm2px(ringWidth_MM * 2 + knobSize_MM) * 0.5;
        nvgBeginPath(vg);
        nvgArc(vg, box.size.x * 0.5, box.size.y * 0.5, radius, minAngle - M_PI_2, maxAngle - M_PI_2,
               NVG_CW);
        nvgStrokeWidth(vg, ringWidth_PX);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::KNOB_RING));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);
    }

    void drawValueRing(NVGcontext *vg)
    {
        if (isModEditing)
            return;
        auto *pq = getParamQuantity();
        if (!pq)
            return;

        auto pv = pq->getSmoothValue();

        float angle;
        angle = rack::math::rescale(pv, pq->getMinValue(), pq->getMaxValue(), minAngle, maxAngle);
        float startAngle = minAngle;
        if (isBipolar())
            startAngle = 0;

        float radius = rack::mm2px(ringWidth_MM * 2 + knobSize_MM) * 0.5;
        nvgBeginPath(vg);
        nvgArc(vg, box.size.x * 0.5, box.size.y * 0.5, radius, startAngle - M_PI_2, angle - M_PI_2,
               startAngle < angle ? NVG_CW : NVG_CCW);
        nvgStrokeWidth(vg, ringWidth_PX);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::KNOB_RING_VALUE));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);

        float mda = modDepthForAnimation();
        if (mda != 0)
        {
            auto modAngle =
                std::clamp(rack::math::rescale(mda, -0.5, 0.5, minAngle, maxAngle) + angle,
                           minAngle, maxAngle);
            nvgBeginPath(vg);
            nvgArc(vg, box.size.x * 0.5, box.size.y * 0.5, radius, modAngle - M_PI_2,
                   angle - M_PI_2, modAngle < angle ? NVG_CW : NVG_CCW);
            nvgStrokeWidth(vg, ringWidth_PX);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::KNOB_MOD_PLUS));
            nvgLineCap(vg, NVG_ROUND);
            nvgStroke(vg);
        }

        auto w = box.size.y;
        auto h = box.size.x;

        auto ox = std::sin(angle) * radius + w / 2;
        auto oy = h - (std::cos(angle) * radius + h / 2);

        nvgBeginPath(vg);
        nvgEllipse(vg, ox, oy, 1, 1);
        nvgFillColor(vg, style()->getColor(style::XTStyle::KNOB_MOD_MARK));
        nvgFill(vg);
    }

    std::unordered_set<ModRingKnob *> modRings;

    BufferedDrawFunctionWidget *bw{nullptr}, *bwValue{nullptr};
    void onChange(const ChangeEvent &e) override;
    void onStyleChanged() override { setupWidgets(); }
    void setupWidgets()
    {
        auto compDir = style()->skinAssetDir() + "/components";

        setSvg(
            rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/" + knobPointerAsset)));
        bg->setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, compDir + "/" + knobBackgroundAsset)));

        // SetSVG changes box.size
        box.size = rack::mm2px(rack::Vec(knobSize_MM, knobSize_MM));

        auto omm = rack::mm2px(ringWidth_MM * 2 + ringPad_MM);
        auto hmm = omm * 0.5;
        box.size.x += omm;
        box.size.y += omm;

        if (!bw)
        {
            // first time through reposition a few items to center the pointer and leave ring room
            auto ptrOffset = rack::mm2px(knobSize_MM - pointerSize_MM) * 0.5;
            sw->box.pos.x += ptrOffset;
            sw->box.pos.y += ptrOffset;

            sw->box.pos.x += hmm;
            sw->box.pos.y += hmm;
            bg->box.pos.x += hmm;
            bg->box.pos.y += hmm;

            bw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                [this](auto vg) { this->drawRing(vg); });
            addChildBottom(bw);

            bwValue = new BufferedDrawFunctionWidgetOnLayer(
                rack::Vec(0, 0), box.size, [this](auto vg) { this->drawValueRing(vg); });
            addChild(bwValue);
        }
        bw->dirty = true;
    }

    float priorMDA{0};
    void step() override
    {
        if (module)
        {
            auto mda = modDepthForAnimation();
            if (mda != priorMDA)
            {
                bwValue->dirty = true;
                priorMDA = mda;
            }
        }
        rack::componentlibrary::RoundKnob::step();
    }
};

struct Knob9 : KnobN
{
    Knob9() : KnobN()
    {
        knobSize_MM = 9;
        pointerSize_MM = 6.9;
        knobPointerAsset = "knob-pointer-9.svg";
        knobBackgroundAsset = "knob-9.svg";
        completeConstructor();
    }
};

struct Knob12 : KnobN
{
    Knob12() : KnobN()
    {
        knobSize_MM = 12;
        pointerSize_MM = 9.9;
        knobPointerAsset = "knob-pointer-12.svg";
        knobBackgroundAsset = "knob-12.svg";
        completeConstructor();
    }
};

struct Knob16 : KnobN
{
    Knob16() : KnobN()
    {
        knobSize_MM = 16;
        pointerSize_MM = 13.9;
        knobPointerAsset = "knob-pointer-16.svg";
        knobBackgroundAsset = "knob-16.svg";
        completeConstructor();
    }
};

struct Port : public rack::app::SvgPort, style::StyleParticipant
{
    Port() { onStyleChanged(); }

    void onStyleChanged() override
    {
        setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, style()->skinAssetDir() + "/components/port.svg")));
    }
};

struct ModRingKnob : rack::app::Knob, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};
    rack::app::Knob *underlyerParamWidget{nullptr};

    int modIndex{0};
    float radius{1};

    ModRingKnob() { box.size = rack::Vec(45, 45); }
    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        auto *uq = underlyerParamWidget->getParamQuantity();
        if (!pq || !uq)
            return;

        auto uv = uq->getSmoothValue();
        auto pv = pq->getSmoothValue();

        auto toAngle = [this](float q, auto *qq, float fac) {
            float angle;
            angle =
                rack::math::rescale(q, fac * qq->getMinValue(), fac * qq->getMaxValue(),
                                    underlyerParamWidget->minAngle, underlyerParamWidget->maxAngle);
            angle = std::fmod(angle, 2 * M_PI);
            return angle;
        };
        float angle = toAngle(uv, uq, 1.0);
        float modAngle = toAngle(pv, pq, 0.5);

        auto w = box.size.y;
        auto h = box.size.x;

        auto ox = std::sin(angle) * radius + w / 2;
        auto oy = h - (std::cos(angle) * radius + h / 2);

        nvgBeginPath(vg);
        auto aStart = std::clamp(angle - modAngle, underlyerParamWidget->minAngle,
                                 underlyerParamWidget->maxAngle) -
                      M_PI_2;
        auto aEnd = std::clamp(angle + modAngle, underlyerParamWidget->minAngle,
                               underlyerParamWidget->maxAngle) -
                    M_PI_2;
        nvgArc(vg, w / 2, h / 2, radius, angle - M_PI_2, aStart, -modAngle < 0 ? NVG_CCW : NVG_CW);
        nvgStrokeWidth(vg, Knob9::ringWidth_PX);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::KNOB_MOD_MINUS));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgArc(vg, w / 2, h / 2, radius, angle - M_PI_2, aEnd, modAngle < 0 ? NVG_CCW : NVG_CW);
        nvgStrokeWidth(vg, Knob9::ringWidth_PX);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::KNOB_MOD_PLUS));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgEllipse(vg, ox, oy, 1, 1);
        nvgFillColor(vg, style()->getColor(style::XTStyle::KNOB_MOD_MARK));
        nvgFill(vg);
    }

    static ModRingKnob *createCentered(rack::Vec pos, float diameter, rack::Module *module,
                                       int paramId)
    {
        auto *res = rack::createWidget<ModRingKnob>(pos);

        res->radius = diameter * 0.5;
        res->box.size.x = diameter + rack::mm2px(1);
        res->box.size.y = diameter + rack::mm2px(1);
        res->box.pos.x = pos.x - res->box.size.x / 2.f;
        res->box.pos.y = pos.y - res->box.size.y / 2.f;
        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        res->bdw = new BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), res->box.size, [res](NVGcontext *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);

        return res;
    }

    void onChange(const ChangeEvent &e) override
    {
        if (bdw)
            bdw->dirty = true;

        rack::app::Knob::onChange(e);
    }

    void onStyleChanged() override
    {
        if (bdw)
            bdw->dirty = true;
    }
    bool bypassGesture()
    {
        if (APP->window)
        {
            int mods = APP->window->getMods();
            if ((mods & RACK_MOD_MASK) == (GLFW_MOD_ALT | GLFW_MOD_SHIFT))
            {
                return true;
            }
        }
        return false;
    }
    void onHover(const HoverEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onHover(e);
    }
    void onButton(const ButtonEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onButton(e);
    }
    void onDragStart(const DragStartEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onDragStart(e);
    }
    void onDragEnd(const DragEndEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onDragEnd(e);
    }
    void onDragMove(const DragMoveEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onDragMove(e);
    }
    void onDragLeave(const DragLeaveEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onDragLeave(e);
    }
    void onHoverScroll(const HoverScrollEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onHoverScroll(e);
    }
    void onLeave(const LeaveEvent &e) override
    {
        if (!bypassGesture())
            rack::Knob::onLeave(e);
    }
};

struct ActivateKnobSwitch : rack::app::Switch, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    float radius;
    ActivateKnobSwitch()
    {
        box.size = rack::mm2px(rack::Vec(3, 3));
        radius = rack::mm2px(1.1);
        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                             [this](auto v) { this->drawBackground(v); });
        bdwLight = new BufferedDrawFunctionWidgetOnLayer(rack::Vec(0, 0), box.size,
                                                         [this](auto v) { this->drawLight(v); });
        addChild(bdw);
        addChild(bdwLight);
    }

    void drawBackground(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::KNOB_RING));
        nvgFillColor(vg, style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_OFF));
        nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, radius, radius);
        nvgFill(vg);
        nvgStrokeWidth(vg, 0.75);
        nvgStroke(vg);
    }

    void drawLight(NVGcontext *vg)
    {
        if (!getParamQuantity())
            return;
        auto q = getParamQuantity()->getValue();
        if (q < 0.5)
            return;
        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGB(0, 0, 0));
        nvgFillColor(vg, style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_ON));
        nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, radius * 0.9, radius * 0.9);
        nvgFill(vg);
    }

    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
        Widget::onChange(e);
    }
    void onStyleChanged() override {}
};

struct ModToggleButton : rack::widget::Widget, style::StyleParticipant
{
    bool pressedState{false};
    std::function<void(bool)> onToggle = [](bool isOn) {};
    rack::SvgWidget *svg{nullptr};

    float button_MM = 6.5, light_MM = 2.75;
    float light_pixelRadius = rack::mm2px(light_MM) * 0.5;

    ModToggleButton()
    {
        svg = new rack::widget::SvgWidget();
        svg->box.pos.x = 0;
        svg->box.pos.y = 0;
        onStyleChanged();
        box.size = svg->box.size;
        addChild(svg);
    }

    void onButton(const ButtonEvent &e) override
    {
        if (e.action == GLFW_RELEASE)
        {
            pressedState = !pressedState;
            onToggle(pressedState);
            e.consume(this);
        }
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1 && pressedState)
        {
            const float halo = rack::settings::haloBrightness;
            auto c = box.size.div(2);

            if (halo > 0.f)
            {
                float radius = light_pixelRadius;
                float oradius = rack::mm2px(button_MM) * 0.5;

                nvgBeginPath(args.vg);
                nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

                NVGcolor icol =
                    rack::color::mult(style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_ON), halo);
                NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
                NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
                nvgFillPaint(args.vg, paint);
                nvgFill(args.vg);
            }
            nvgBeginPath(args.vg);
            nvgFillColor(args.vg, style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_ON));
            nvgEllipse(args.vg, c.x, c.y, light_pixelRadius, light_pixelRadius);
            nvgFill(args.vg);
        }

        Widget::drawLayer(args, layer);
    }

    void draw(const DrawArgs &args) override
    {
        Widget::draw(args);

        if (!pressedState)
        {
            nvgBeginPath(args.vg);
            nvgFillColor(args.vg, style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_OFF));
            nvgEllipse(args.vg, box.size.x / 2, box.size.y / 2, light_pixelRadius,
                       light_pixelRadius);
            nvgFill(args.vg);
        }
    }

    void setState(bool b) { pressedState = b; }

    void onStyleChanged() override
    {
        svg->setSvg(rack::Svg::load(rack::asset::plugin(
            pluginInstance, style()->skinAssetDir() + "/components/mod-button.svg")));
    }
};

struct LabeledPlotAreaControl : public rack::app::Knob, style::StyleParticipant
{
    static constexpr float padTop_MM = 1.4;
    static constexpr float padBot_MM = 1.6;
    static constexpr float box_px = 13;
    BufferedDrawFunctionWidget *bdw{nullptr};
    std::string label;
    std::function<std::string(float, const std::string &)> formatLabel;

    static LabeledPlotAreaControl *create(rack::Vec pos, rack::Vec sz, const std::string &lab,
                                          rack::Module *module, int paramId)
    {
        auto *res = rack::createWidget<LabeledPlotAreaControl>(pos);

        res->box.pos = pos;
        res->box.pos.y += rack::mm2px(padTop_MM);
        res->box.size = sz;
        res->box.size.y -= rack::mm2px(padBot_MM);
        res->label = lab;

        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        res->bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), res->box.size,
                                                  [res](NVGcontext *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);
        res->formatLabel = [](float f, const std::string &s) {
            auto sp = s.find(" ");
            return s.substr(0, sp);
        };

        return res;
    }

    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        if (!pq)
            return;

        auto pv = formatLabel(pq->getValue(), pq->getDisplayValueString());

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, 0, box.size.y * 0.5, label.c_str(), nullptr);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
        nvgRect(vg, box.size.x - box_px, 0, box_px, box.size.y);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_FG));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, box.size.x - box_px * 0.5, box.size.y * 0.5, pv.c_str(), nullptr);
    }

    void onStyleChanged() override { bdw->dirty = true; }
    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        Widget::onChange(e);
    }
};

struct PlotAreaMenuItem : public rack::app::Knob, style::StyleParticipant
{
    static constexpr float padTop_MM = 1.4;
    static constexpr float padBot_MM = 1.6;
    BufferedDrawFunctionWidget *bdw{nullptr};
    std::function<std::string(float, const std::string &)> formatLabel;
    std::function<void()> onShowMenu = []() {};

    static PlotAreaMenuItem *create(rack::Vec pos, rack::Vec sz, rack::Module *module, int paramId)
    {
        auto *res = rack::createWidget<PlotAreaMenuItem>(pos);

        res->box.pos = pos;
        res->box.pos.y += rack::mm2px(padTop_MM);
        res->box.size = sz;
        res->box.size.y -= rack::mm2px(padBot_MM);

        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        res->bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), res->box.size,
                                                  [res](NVGcontext *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);
        res->formatLabel = [](float f, const std::string &s) {
            auto sp = s.find(" ");
            return s.substr(0, sp);
        };

        return res;
    }

    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        if (!pq)
            return;

        auto pv = pq->getDisplayValueString();
        for (auto &q : pv)
            q = std::toupper(q);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, box.size.x - box.size.y - rack::mm2px(0.5), box.size.y * 0.5, pv.c_str(),
                nullptr);

        float gapX = rack::mm2px(0.5);
        float gapY = rack::mm2px(0.7);
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgMoveTo(vg, box.size.x - box.size.y + gapX, gapY);
        nvgLineTo(vg, box.size.x - gapX, gapY);
        nvgLineTo(vg, box.size.x - box.size.y * 0.5, box.size.y - gapY);
        nvgFill(vg);
        nvgStroke(vg);
    }

    void onStyleChanged() override { bdw->dirty = true; }
    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        Widget::onChange(e);
    }

    void onAction(const ActionEvent &e) override
    {
        onShowMenu();
        e.consume(this);
    }
};

template <typename T> struct GenericPresetJogSelector : public T, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};

    rack::Vec leftJogSize, rightJogSize, leftJogPos, rightJogPos;

    void setup()
    {
        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0),
                                             rack::Vec(this->box.size.x, this->box.size.y),
                                             [this](auto *v) { this->drawSelector(v); });
        this->addChild(bdw);

        leftJogSize = rack::Vec(this->box.size.y, this->box.size.y);
        rightJogSize = leftJogSize;

        leftJogPos = rack::Vec(0, 0);
        rightJogPos = rack::Vec(this->box.size.x - this->box.size.y, 0);
    }

    void drawSelector(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        auto txtColor = style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT);
        if (!hasPresets())
            txtColor.a = 0.30;
        nvgFillColor(vg, txtColor);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, this->box.size.x * 0.5, this->box.size.y * 0.5, getPresetName().c_str(),
                nullptr);

        if (hasPresets())
        {
            // left arrow
            float onemm = rack::mm2px(1.0);
            nvgBeginPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
            nvgMoveTo(vg, leftJogPos.x + onemm, leftJogPos.y + leftJogSize.y * 0.5);
            nvgLineTo(vg, leftJogPos.x + leftJogSize.x - onemm, leftJogPos.y + onemm);
            nvgLineTo(vg, leftJogPos.x + leftJogSize.x - onemm,
                      leftJogPos.y + leftJogSize.y - onemm);
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
            nvgMoveTo(vg, rightJogPos.x + rightJogSize.x - onemm,
                      rightJogPos.y + rightJogSize.y * 0.5);
            nvgLineTo(vg, rightJogPos.x + onemm, rightJogPos.y + onemm);
            nvgLineTo(vg, rightJogPos.x + onemm, rightJogPos.y + rightJogSize.y - onemm);
            nvgFill(vg);
        }
    }

    void step() override
    {
        if (isDirty())
        {
            bdw->dirty = true;
        }
        T::step();
    }
    virtual void onPresetJog(int dir /* +/- 1 */) = 0;
    virtual void onShowMenu() = 0;
    virtual std::string getPresetName() = 0;
    virtual bool isDirty() = 0;
    virtual bool hasPresets() { return true; }

    void onButton(const typename T::ButtonEvent &e) override
    {
        if (e.action == GLFW_PRESS)
        {
            if (e.pos.x >= leftJogPos.x && e.pos.x <= leftJogPos.x + leftJogSize.x &&
                e.pos.y >= leftJogPos.y && e.pos.y <= leftJogPos.y + leftJogSize.y)
            {
                onPresetJog(-1);
            }
            else if (e.pos.x >= rightJogPos.x && e.pos.x <= rightJogPos.x + rightJogSize.x &&
                     e.pos.y >= rightJogPos.y && e.pos.y <= rightJogPos.y + rightJogSize.y)
            {
                onPresetJog(1);
            }
            else
            {
                onShowMenu();
            }
            e.consume(this);
            bdw->dirty = true;
        }
        if (e.action == GLFW_RELEASE)
        {
            e.consume(this);
            bdw->dirty = true;
        }
    }

    void onStyleChanged() override {}
};

typedef GenericPresetJogSelector<rack::Widget> PresetJogSelector;
typedef GenericPresetJogSelector<rack::ParamWidget> ParamJogSelector;

inline void KnobN::onChange(const rack::widget::Widget::ChangeEvent &e)
{
    bw->dirty = true;
    bwValue->dirty = true;
    for (auto *m : modRings)
    {
        m->bdw->dirty = true;
    }
    SvgKnob::onChange(e);
}

struct LCDBackground : public rack::widget::TransparentWidget, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};
    std::string noModuleText;
    static LCDBackground *createWithHeight(float endPosInMM, float widthInScrews = 12)
    {
        auto posx = 12.08506f;
        auto posy = 25.408199;
        auto width = rack::app::RACK_GRID_WIDTH * widthInScrews - 2 * posx;
        auto height = rack::mm2px(endPosInMM) - posy;

        auto res = new LCDBackground();
        res->setup(rack::Vec(posx, posy), rack::Vec(width, height));

        return res;
    }

    void setup(const rack::Vec &pos, const rack::Vec &size)
    {
        box.pos = pos;
        box.size = size;

        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                             [this](auto vg) { this->drawBackground(vg); });
        addChild(bdw);
    }

    void drawBackground(NVGcontext *vg)
    {
        float cornerRadius = 2 * 0.66406;
        float offset = 1;
        float inset = offset / 2.f;

        // OK so gray rectangle, light path, shifted gray rectangle, black path
        nvgBeginPath(vg);
        nvgRoundedRect(vg, inset, inset, box.size.x - 2 * inset, box.size.y - 2 * inset,
                       cornerRadius);
        nvgFillColor(vg, style()->getColor(style::XTStyle::LED_PANEL));
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::LED_HIGHLIGHT));
        nvgFill(vg);
        nvgStrokeWidth(vg, offset);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, inset, inset, box.size.x - 2 * inset, box.size.y - 2 * inset - offset,
                       cornerRadius);
        nvgFillColor(vg, style()->getColor(style::XTStyle::LED_PANEL));
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::LED_BORDER));
        nvgFill(vg);
        nvgStrokeWidth(vg, offset);
        nvgStroke(vg);

        if (!noModuleText.empty())
        {
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, 17);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, noModuleText.c_str(), nullptr);
        }
    }
    void onStyleChanged() override { bdw->dirty = true; }
};
} // namespace sst::surgext_rack::widgets

#endif // SURGEXT_RACK_XTWIDGETS_H
