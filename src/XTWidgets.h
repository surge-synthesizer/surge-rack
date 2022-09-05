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
    std::string panelName, groupName;
    std::function<void(NVGcontext *)> moduleSpecificDraw;

    Background(rack::Vec size, const std::string &grp, const std::string &pn)
        : groupName(grp), panelName(pn)
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
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFontFaceId(vg, style::SurgeStyle::fontId(vg));
        nvgFontSize(vg, 17);

        std::string s = groupName + "::" + panelName;
        nvgFillColor(vg, nvgRGB(0xFF, 0x90, 0x00));
        nvgText(vg, box.size.x * 0.5, 2, s.c_str(), nullptr);

        nvgBeginPath(vg);
        nvgFontFaceId(vg, style::SurgeStyle::fontId(vg));
        nvgFontSize(vg, 17);
        nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);
        nvgFillColor(vg, nvgRGB(0xFF, 0x90, 0x00));
        nvgText(vg, box.size.x * 0.5, box.size.y - 2, "Missing Panel", nullptr);
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

    void onStyleChanged() override
    {
        auto childrenCopy = children;
        for (auto k : childrenCopy)
            removeChild(k);

        std::string asset =
            style::SurgeStyle::skinAssetDir() + "/panels/" + groupName + "/" + panelName + ".svg";

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
    }
};

struct Knob9 : public rack::componentlibrary::RoundKnob, style::StyleListener
{
    static constexpr float ringWidth_MM = 0.5f;
    static constexpr float ringPad_MM = 0.5f;
    static constexpr float knobSize_MM = 9.0f;
    static constexpr float pointerSize_MM = 6.9f;
    Knob9() { onStyleChanged(); }

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

    BufferedDrawFunctionWidget *bw{nullptr};
    void onChange(const ChangeEvent &e) override
    {
        bw->dirty = true;
        SvgKnob::onChange(e);
    }

    void onStyleChanged() override
    {
        auto compDir = style::SurgeStyle::skinAssetDir() + "/components";

        setSvg(rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/knob-pointer.svg")));
        bg->setSvg(rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/knob-9.svg")));

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
        }
        bw->dirty = true;
    }
};

struct Port : public rack::app::SvgPort, style::StyleListener
{
    Port() { onStyleChanged(); }

    void onStyleChanged() override
    {
        setSvg(rack::Svg::load(rack::asset::plugin(
            pluginInstance, style::SurgeStyle::skinAssetDir() + "/components/port.svg")));
    }
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

                NVGcolor icol = rack::color::mult(nvgRGB(0xFF, 0x90, 0x00), halo);
                NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
                NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
                nvgFillPaint(args.vg, paint);
                nvgFill(args.vg);
            }
            nvgBeginPath(args.vg);
            nvgFillColor(args.vg, nvgRGB(0xFF, 0x90, 0x00));
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
            nvgFillColor(args.vg, nvgRGB(0x82, 0x82, 0x82));
            nvgEllipse(args.vg, box.size.x / 2, box.size.y / 2, light_pixelRadius,
                       light_pixelRadius);
            nvgFill(args.vg);
        }
    }

    void setState(bool b) { pressedState = b; }

    void onStyleChanged() override
    {
        svg->setSvg(rack::Svg::load(rack::asset::plugin(
            pluginInstance, style::SurgeStyle::skinAssetDir() + "/components/mod-button.svg")));
    }
};

} // namespace sst::surgext_rack::widgets

#endif // SURGEXT_RACK_XTWIDGETS_H
