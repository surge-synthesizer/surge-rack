//
// Created by Paul Walker on 9/4/22.
//

#ifndef SURGEXT_RACK_XTWIDGETS_H
#define SURGEXT_RACK_XTWIDGETS_H

#include <rack.hpp>
#include "XTStyle.hpp"

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
        auto res = new Label(pos, size); // FIXME on that obv
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
        nvgText(vg, box.size.x * 0.5, box.size.y, label.c_str(), nullptr);

#if DEBUG_RECT
        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGB(255, 0, 0));
        nvgRect(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
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
        label->tracking = 1;
        addChild(label);
    }
};

struct Knob9 : public rack::componentlibrary::RoundKnob, style::StyleParticipant
{
    static constexpr float ringWidth_MM = 0.7f;
    static constexpr float ringPad_MM = 0.5f;
    static constexpr float knobSize_MM = 9.0f;
    static constexpr float pointerSize_MM = 6.9f;
    static constexpr float ringWidth_PX = 1.5;
    Knob9()
    {
        float angleSpreadDegrees = 40.0;

        minAngle = -M_PI * (180 - angleSpreadDegrees) / 180;
        maxAngle = M_PI * (180 - angleSpreadDegrees) / 180;

        onStyleChanged();
        fb->removeChild(shadow);
    }

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

    BufferedDrawFunctionWidget *bw{nullptr};
    void onChange(const ChangeEvent &e) override
    {
        bw->dirty = true;
        SvgKnob::onChange(e);
    }

    void onStyleChanged() override
    {
        auto compDir = style()->skinAssetDir() + "/components";

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

struct OctaveControl : public rack::app::Knob, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};
    static OctaveControl *create(rack::Vec pos, rack::Vec sz, SurgeModuleCommon *module,
                                 int paramId)
    {
        auto *res = rack::createWidget<OctaveControl>(pos);

        res->box.pos = pos;
        res->box.size = sz;

        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        res->bdw = new BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), res->box.size, [res](NVGcontext *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);

        return res;
    }

    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        if (!pq)
            return;

        auto pv = (int)std::round(pq->getValue());

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, 0, box.size.y * 0.5, "OCT", nullptr);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
        nvgRect(vg, box.size.x * 0.65, 0, box.size.x * 0.35, box.size.y);
        nvgFill(vg);

        auto valString = std::to_string(pv);
        if (pv > 0)
            valString = "+" + valString;
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_FG));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, box.size.x * 0.825, box.size.y * 0.5, valString.c_str(), nullptr);
    }

    void onStyleChanged() override { bdw->dirty = true; }
    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        Widget::onChange(e);
    }
};

} // namespace sst::surgext_rack::widgets

#endif // SURGEXT_RACK_XTWIDGETS_H
