//
// Created by Paul Walker on 9/4/22.
//

#ifndef SURGEXT_RACK_XTWIDGETS_H
#define SURGEXT_RACK_XTWIDGETS_H

#include <rack.hpp>
#include "SurgeStyle.hpp"

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

struct Background : public rack::TransparentWidget, style::StyleListener
{
    std::string panelName;
    std::function<void(NVGcontext *)> moduleSpecificDraw;

    Background(rack::Vec size, std::string pn) : panelName(pn)
    {
        box.size = size;
        auto svgp = new rack::app::SvgPanel();
        svgp->box.pos = rack::Vec(0, 0);
        svgp->box.size = size;

        auto panelLogo =
            rack::Svg::load(rack::asset::plugin(pluginInstance, "res/xt/dark/panels/String.svg"));
        svgp->setBackground(panelLogo);
        addChild(svgp);
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
#if 0
        if (layer == 1)
        {
            std::array<float, 4> columCenters_MM{9.48, 23.48, 37.48, 51.48};
            std::array<float, 5> rowCenters_MM{55, 71, 85.32, 100.16, 114.5};

            for (auto x : columCenters_MM)
            {
                auto xp = x * style::SVG_PX_PER_MM;
                nvgBeginPath(args.vg);
                nvgMoveTo(args.vg, xp, 0);
                nvgLineTo(args.vg, xp, 380);
                nvgStrokeColor(args.vg, nvgRGB(255, 0, 0));
                nvgStrokeWidth(args.vg, 1);
                nvgStroke(args.vg);
            }

            for (auto y : rowCenters_MM)
            {
                auto yp = y * style::SVG_PX_PER_MM;
                nvgBeginPath(args.vg);
                nvgMoveTo(args.vg, 0, yp);-
                nvgLineTo(args.vg, 12 * 15, yp);
                nvgStrokeColor(args.vg, nvgRGB(255, 0, 0));
                nvgStrokeWidth(args.vg, 1);
                nvgStroke(args.vg);
            }
        }
#endif
        rack::TransparentWidget::drawLayer(args, layer);
    }

    void onStyleChanged() override { throw "Implement Me"; }
};

struct Knob9 : public rack::componentlibrary::RoundKnob, style::StyleListener
{
    static constexpr float ringWidth_MM = 0.5f;
    static constexpr float ringPad_MM = 0.5f;
    static constexpr float knobSize_MM = 9.0f;
    Knob9()
    {
        setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, "res/xt/dark/components/knob-pointer.svg")));
        bg->setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, "res/xt/dark/components/knob-9.svg")));

        auto omm = rack::mm2px(ringWidth_MM * 2 + ringPad_MM);
        auto hmm = omm * 0.5;
        box.size.x += omm;
        box.size.y += omm;
        sw->box.pos.x += hmm;
        sw->box.pos.y += hmm;
        bg->box.pos.x += hmm;
        bg->box.pos.y += hmm;

        bw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                            [this](auto vg) { this->drawRing(vg); });
        addChildBottom(bw);
    }

    void drawRing(NVGcontext *vg)
    {
        float radius = rack::mm2px(ringWidth_MM * 2 + knobSize_MM) * 0.5;
        nvgBeginPath(vg);
        nvgArc(vg, box.size.x * 0.5, box.size.y * 0.5, radius, minAngle - M_PI_2, maxAngle - M_PI_2,
               NVG_CW);
        nvgStrokeWidth(vg, 1);
        // FIXME - into style
        nvgStrokeColor(vg, nvgRGB(0x82, 0x82, 0x82));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);
    }

    BufferedDrawFunctionWidget *bw;
    void onChange(const ChangeEvent &e) override
    {
        bw->dirty = true;
        SvgKnob::onChange(e);
    }

    void onStyleChanged() override { throw "Implement Me"; }
};

struct Port : public rack::app::SvgPort, style::StyleListener
{
    Port()
    {
        setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, "res/xt/dark/components/port.svg")));
    }

    void onStyleChanged() override { throw "Implement Me"; }
};

struct LinePlotWidget : public rack::widget::TransparentWidget, style::StyleListener
{
};

struct ModRingKnob : rack::app::Knob
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

        auto toAngle = [this](float q, auto *qq) {
            float angle;
            angle =
                rack::math::rescale(q, qq->getMinValue(), qq->getMaxValue(),
                                    underlyerParamWidget->minAngle, underlyerParamWidget->maxAngle);
            angle = std::fmod(angle, 2 * M_PI);
            return angle;
        };
        float angle = toAngle(uv, uq);
        float modAngle = toAngle(pv, pq);

        auto w = box.size.y;
        auto h = box.size.x;

        auto ox = std::sin(angle) * radius + w / 2;
        auto oy = h - (std::cos(angle) * radius + h / 2);

        nvgBeginPath(vg);
        nvgArc(vg, w / 2, h / 2, radius, angle - M_PI_2, angle - modAngle - M_PI_2,
               -modAngle < 0 ? NVG_CCW : NVG_CW);
        nvgStrokeWidth(vg, 1.25);
        nvgStrokeColor(vg, nvgRGB(180, 180, 200));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgArc(vg, w / 2, h / 2, radius, angle - M_PI_2, angle + modAngle - M_PI_2,
               modAngle < 0 ? NVG_CCW : NVG_CW);
        nvgStrokeWidth(vg, 1.25);
        nvgStrokeColor(vg, nvgRGB(0xFF, 0x90, 0x00));
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgEllipse(vg, ox, oy, 1, 1);
        nvgFillColor(vg, nvgRGB(255, 255, 255));
        nvgFill(vg);
    }

    static ModRingKnob *createCentered(rack::Vec pos, int diameter, SurgeModuleCommon *module,
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

        res->bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), res->box.size,
                                                  [res](NVGcontext *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);

        return res;
    }

    void onChange(const ChangeEvent &e) override
    {
        if (bdw)
            bdw->dirty = true;

        rack::app::Knob::onChange(e);
    }
};

struct ModToggleButton : rack::widget::Widget, style::StyleListener
{
    BufferedDrawFunctionWidget *bdw = nullptr;
    bool pressedState{false};
    bool isHovered{false};
    std::function<void(bool)> onToggle = [](bool isOn) {};

    ModToggleButton()
    {
        box.size = rack::Vec(25, 25);
        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                             [this](auto *v) { this->drawToggle(v); });
        addChild(bdw);
    }

    void drawToggle(NVGcontext *vg)
    {
        auto w = box.size.y;
        auto h = box.size.x;
        nvgBeginPath(vg);
        nvgEllipse(vg, h / 2, w / 2, h * 0.45, w * 0.45);
        if (pressedState)
            nvgFillColor(vg, nvgRGB(0xA0, 0xA0, 0xFF));
        else
            nvgFillColor(vg, nvgRGB(0x60, 0x60, 0x9F));
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgEllipse(vg, h / 2, w / 2, h * 0.45, w * 0.45);
        if (isHovered)
            nvgFillColor(vg, nvgRGB(20, 20, 90));
        else
            nvgFillColor(vg, nvgRGB(0, 0, 0));
        nvgStrokeWidth(vg, 2);
        nvgStroke(vg);
    }

    void onButton(const ButtonEvent &e) override
    {
        if (e.action == GLFW_RELEASE)
        {
            pressedState = !pressedState;
            onToggle(pressedState);
            if (bdw)
                bdw->dirty = true;
            e.consume(this);
        }
    }

    void setState(bool b)
    {
        pressedState = b;
        if (bdw)
            bdw->dirty = true;
    }

    void onStyleChanged() override { throw "Implement Me"; }
};

} // namespace sst::surgext_rack::widgets

#endif // SURGEXT_RACK_XTWIDGETS_H
