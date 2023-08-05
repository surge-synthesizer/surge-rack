/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_SRC_XTWIDGETS_H
#define SURGE_XT_RACK_SRC_XTWIDGETS_H

#include <rack.hpp>
#include <iostream>
#include "XTStyle.h"
#include "XTModule.h"
#include "LayoutConstants.h"
#include "sst/rackhelpers/ui.h"
#include "sst/rackhelpers/module_connector.h"

namespace sst::surgext_rack::widgets
{
using BufferedDrawFunctionWidget = sst::rackhelpers::ui::BufferedDrawFunctionWidget;
using BufferedDrawFunctionWidgetOnLayer = sst::rackhelpers::ui::BufferedDrawFunctionWidgetOnLayer;

struct DebugRect : rack::TransparentWidget
{
    NVGcolor fill{nvgRGBA(255, 255, 0, 40)}, stroke{nvgRGB(255, 0, 0)};
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
        nvgStrokeColor(args.vg, stroke);
        nvgFillColor(args.vg, fill);
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
    float size{layout::LayoutConstants::labelSize_pt};
    float tracking{0.0};
    float baselinePad{0};
    style::XTStyle::Colors color;
    Label(const rack::Vec &pos, const rack::Vec &sz)
        : BufferedDrawFunctionWidget(pos, sz, [this](auto vg) { drawLabel(vg); })
    {
        box.pos = pos;
        box.size = sz;
    }
    ~Label();

    modules::XTModule *module{nullptr};
    std::function<std::string(modules::XTModule *m)> dynamicLabel{};
    bool hasDynamicLabel{false};
    std::string priorDynamic{};

    static Label *
    createWithBaselineBox(const rack::Vec &pos, const rack::Vec &size, const std::string lab,
                          float szInPt = layout::LayoutConstants::labelSize_pt,
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
        auto col = style()->getColor(color);
        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, size * 96.0 / 72.0);
        nvgFillColor(vg, style()->getColor(color));
        nvgStrokeColor(vg, style()->getColor(color));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
        nvgTextLetterSpacing(vg, tracking);
        if (hasDynamicLabel)
        {
            nvgText(vg, box.size.x * 0.5, box.size.y - baselinePad, priorDynamic.c_str(), nullptr);
        }
        else
        {
            nvgText(vg, box.size.x * 0.5, box.size.y - baselinePad, label.c_str(), nullptr);
        }
#define DEBUG_RECT 0
#if DEBUG_RECT
        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGB(255, 0, 0));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgStroke(vg);
#endif
    }

    void step() override
    {
        if (hasDynamicLabel)
        {
            auto dl = dynamicLabel(module);
            if (dl != priorDynamic)
            {
                dirty = true;
            }
            priorDynamic = dl;
        }
        FramebufferWidget::step();
    }
    void onStyleChanged() override { dirty = true; }
};

struct Background : public rack::TransparentWidget, style::StyleParticipant
{
    std::string panelName, groupName, title;
    std::function<void(NVGcontext *)> moduleSpecificDraw;

    Label *titleLabel{nullptr};
    rack::app::SvgPanel *svgPanel{nullptr};

    Background(rack::Vec size, const std::string &t, const std::string &grp, const std::string &pn)
        : panelName(pn), groupName(grp), title(t)
    {
        box.size = size;

        onStyleChanged();

#if SURGEXT_RACK_DEBUG
        addDebug();
#endif
    }

    void drawStubLayer(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(0x50, 0x50, 0x60));
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

        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontId(vg));
        nvgFontSize(vg, 17);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFillColor(vg, nvgRGB(0xFF, 0x90, 0x00));
        nvgText(vg, box.size.x * 0.5, 2, title.c_str(), nullptr);
    }

    void onStyleChanged() override
    {
        std::string asset =
            style()->skinAssetDir() + "/panels/" + groupName + "/" + panelName + ".svg";

        auto panelLogo = rack::Svg::load(rack::asset::plugin(pluginInstance, asset));
        if (panelLogo)
        {
            bool addMe{false};
            if (!svgPanel)
            {
                addMe = true;
                svgPanel = new rack::app::SvgPanel();
            }
            svgPanel->box.pos = rack::Vec(0, 0);
            svgPanel->box.size = box.size;

            svgPanel->setBackground(panelLogo);
            if (addMe)
                addChild(svgPanel);
        }
        else
        {
            auto bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { drawStubLayer(vg); });
            addChild(bdw);
        }

        if (!titleLabel && !title.empty())
        {
            titleLabel = Label::createWithBaselineBox(
                rack::Vec(0, 0),
                rack::Vec(box.size.x, rack::mm2px(layout::LayoutConstants::mainLabelBaseline_MM)),
                title, layout::LayoutConstants::mainLabelSize_PT);
            titleLabel->tracking = 0.7;
            addChild(titleLabel);
        }

        if (alphaBetaWarning)
        {
            alphaBetaWarning->dirty = true;
        }

        if (debugWarning)
        {
            debugWarning->dirty = true;
        }
    }

    BufferedDrawFunctionWidget *alphaBetaWarning{nullptr};
    void addAlpha()
    {
        alphaBetaWarning = new BufferedDrawFunctionWidget(rack::Vec(0, 0), rack::Vec(100, 20),
                                                          [this](auto vg) { drawAlpha(vg); });
        addChild(alphaBetaWarning);
    }
    void addBeta()
    {
        alphaBetaWarning = new BufferedDrawFunctionWidget(rack::Vec(0, 0), rack::Vec(100, 20),
                                                          [this](auto vg) { drawBeta(vg); });
        addChild(alphaBetaWarning);
    }
    void drawAlpha(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 18);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
        nvgFillColor(vg, style()->getColor(style::XTStyle::TEXT_LABEL));
        nvgText(vg, 1.5, 1.5, "ALPHA", nullptr);
        nvgFillColor(vg, nvgRGB(255, 0, 0));
        nvgText(vg, 1, 1, "ALPHA", nullptr);
    }
    void drawBeta(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 18);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
        nvgFillColor(vg, style()->getColor(style::XTStyle::TEXT_LABEL));
        nvgText(vg, 1.5, 1.5, "BETA", nullptr);
        nvgFillColor(vg, nvgRGB(0, 255, 0));
        nvgText(vg, 1, 1, "BETA", nullptr);
    }

    BufferedDrawFunctionWidget *debugWarning{nullptr};
    void addDebug()
    {
        debugWarning = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { drawDebug(vg); });
        addChild(debugWarning);
    }
    void drawDebug(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 18);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_RIGHT);
        nvgFillColor(vg, style()->getColor(style::XTStyle::TEXT_LABEL));
        nvgText(vg, box.size.x - 1.5, 1.5, "DBG", nullptr);
        nvgFillColor(vg, nvgRGB(0, 0, 255));
        nvgText(vg, box.size.x - 1, 1, "DBG", nullptr);
    }
};

struct ModRingKnob;

struct ModulatableKnob
{
    virtual void setIsModEditing(bool b) = 0;
    virtual rack::Widget *asWidget() = 0;
};

struct KnobN : public rack::componentlibrary::RoundKnob, style::StyleParticipant, ModulatableKnob
{
    static constexpr float ringWidth_MM = 0.7f;
    static constexpr float ringPad_MM = 0.5f;
    static constexpr float ringWidth_PX = 1.5;

    float knobSize_MM = -1;
    float pointerSize_MM = -1;
    float shadowOffset_MM{0.4f}, shadowTwoOffset_MM{0.375f}, shadowOneW_MM{-1}, shadowOneH_MM{-1},
        shadowTwoW_MM{-1}, shadowTwoH_MM{-1};
    std::string knobPointerAsset, knobBackgroundAsset;

    KnobN() {}
    ~KnobN()
    {
        // we removed shadow so it won't be deleted by the sweep
        if (shadow)
        {
            delete shadow;
            shadow = nullptr;
        }
    }
    Widget *asWidget() override { return this; }

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
        if (!style::XTStyle::getShowModulationAnimationOnKnobs())
            return 0;

        if (xtm)
            return xtm->modulationDisplayValue(paramId);
        return 0;
    }

    bool isModEditing{false};
    void setIsModEditing(bool b) override
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
        if (shadow)
        {
            fb->removeChild(shadow);
        }

        if (shadowOneH_MM < 0)
            shadowOneH_MM = knobSize_MM * 0.9911;
        if (shadowOneW_MM < 0)
            shadowOneW_MM = knobSize_MM - 0.5;
        if (shadowTwoH_MM < 0)
            shadowTwoH_MM = knobSize_MM * 1.0222;
        if (shadowTwoW_MM < 0)
            shadowTwoW_MM = shadowOneW_MM;
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

    BufferedDrawFunctionWidget *bw{nullptr}, *bwValue{nullptr}, *bwShadow{nullptr};
    void onChange(const ChangeEvent &e) override;
    void onStyleChanged() override { setupWidgets(); }
    void setupWidgets()
    {
        auto compDir = style()->skinAssetDir() + "/components";

        setSvg(
            rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/" + knobPointerAsset)));
        bg->setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, compDir + "/" + knobBackgroundAsset)));
        // bg->visible = false;

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

            bwShadow =
                new BufferedDrawFunctionWidget(rack::Vec(0, rack::mm2px(shadowOffset_MM)), box.size,
                                               [this](auto vg) { this->drawShadow(vg); });
            addChildBottom(bwShadow);
        }
        bw->dirty = true;
    }

    void drawShadow(NVGcontext *vg)
    {
        if (!style::XTStyle::getShowShadows())
            return;

        nvgBeginPath(vg);
        nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, rack::mm2px(shadowOneW_MM) * 0.5,
                   rack::mm2px(shadowOneH_MM) * 0.5);
        nvgFillColor(vg, style()->getColor(style::XTStyle::SHADOW_BASE));
        // nvgStrokeColor(vg, nvgRGB(255,0,0));
        // nvgStroke(vg);
        nvgFill(vg);

        auto s2 = rack::mm2px(shadowTwoOffset_MM);
        nvgSave(vg);
        nvgScissor(vg, 0, box.size.y * 0.5, box.size.x, box.size.y * 0.5);
        nvgBeginPath(vg);
        auto start = box.size.y * 0.5 + s2 + rack::mm2px(shadowTwoH_MM) * 0.25;
        auto end = box.size.y * 0.5 + s2 + rack::mm2px(shadowTwoH_MM) * 0.5;
        nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5 + s2, rack::mm2px(shadowTwoW_MM) * 0.5,
                   rack::mm2px(shadowTwoH_MM) * 0.5);
        nvgFillPaint(vg, nvgLinearGradient(vg, 0, start, 0, end,
                                           style()->getColor(style::XTStyle::SHADOW_OVER_GRADSTART),
                                           style()->getColor(style::XTStyle::SHADOW_OVER_GRADEND)));
        nvgFill(vg);
        // nvgStrokeColor(vg, nvgRGB(0,255,0));
        // nvgStroke(vg);

        nvgRestore(vg);
    }

    float priorMDA{0};
    bool priorBip{false};
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
            auto bip = isBipolar();
            if (bip != priorBip)
            {
                bwValue->dirty = true;
                priorBip = bip;
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

struct Knob14 : KnobN
{
    Knob14() : KnobN()
    {
        knobSize_MM = 14;
        pointerSize_MM = knobSize_MM - 2.1;
        knobPointerAsset = "knob-pointer-14.svg";
        knobBackgroundAsset = "knob-14.svg";
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

struct Port : public sst::rackhelpers::module_connector::PortConnectionMixin<rack::app::SvgPort>,
              style::StyleParticipant
{
    Port() { onStyleChanged(); }

    void onStyleChanged() override
    {
        setSvg(rack::Svg::load(
            rack::asset::plugin(pluginInstance, style()->skinAssetDir() + "/components/port.svg")));
    }
};

struct HasBDW
{
    virtual ~HasBDW() {}
    BufferedDrawFunctionWidget *bdw{nullptr};
};

struct SQPParamLabel : rack::ui::MenuLabel
{
    modules::SurgeParameterModulationQuantity *spq{nullptr};
    void step() override
    {
        if (spq)
        {
            auto ntext = spq->getLabel() + ": ";
            auto r = spq->getDisplayValueString();
            auto p = r.find("\n");
            if (p != std::string::npos)
                r = r.substr(0, p);
            ntext += r;
            text = ntext;
        }
        else
        {
            text = "SOFTWARE ERROR - null spq";
        }
        MenuLabel::step();
    }
};

struct ModRingKnob : rack::app::Knob, style::StyleParticipant, HasBDW
{
    rack::app::Knob *underlyerParamWidget{nullptr};

    int modIndex{0};
    float radius{1};

    ModRingKnob() { box.size = rack::Vec(45, 45); }
    ~ModRingKnob();

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
        {
            auto spq =
                dynamic_cast<modules::SurgeParameterModulationQuantity *>(getParamQuantity());
            if (spq)
                spq->abbreviate = true;
            rack::Knob::onButton(e);
            if (spq)
                spq->abbreviate = false;
        }
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

    void appendContextMenu(rack::Menu *menu) override
    {
        auto spq = dynamic_cast<modules::SurgeParameterModulationQuantity *>(getParamQuantity());
        if (spq)
        {
            if (menu->children.empty())
                return;

            auto firstItem = menu->children.front();
            menu->removeChild(firstItem);
            delete firstItem;

            auto spql = new SQPParamLabel;
            spql->spq = spq;
            menu->addChildBottom(spql);
        }
    }
};

struct GroupLabel : rack::widget::TransparentWidget, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};
    std::string label;
    bool shortLeft{false}, shortRight{false};

    static GroupLabel *
    createAboveCenterWithColSpan(const std::string &label, const rack::Vec &ctrInMM,
                                 float spanInColumns,
                                 float colWidthMM = layout::LayoutConstants::columnWidth_MM)
    {
        // If you change this remember to fix the LFO also
        float ht = rack::mm2px(4.5);
        float yup = rack::mm2px(1.75);
        auto res = new GroupLabel();

        res->box.pos.x = rack::mm2px(ctrInMM.x - colWidthMM * 0.5);
        res->box.pos.y = rack::mm2px(ctrInMM.y - 8) - yup;
        res->box.size.x = rack::mm2px(spanInColumns * colWidthMM);
        res->box.size.y = ht;
        res->label = label;
        res->setup();
        return res;
    }

    void setup()
    {
        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                             [this](auto v) { this->drawGroup(v); });
        addChild(bdw);
    }

    void drawGroup(NVGcontext *vg)
    {
#define DEBUG_RECT_GL 0
#if DEBUG_RECT_GL
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGBA(255, 0, 0, 20));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFill(vg);
#endif

        float textBox[4];
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::TEXT_LABEL));
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 6.0 * 96 / 72);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgText(vg, box.size.x * 0.5, 0, label.c_str(), nullptr);
        nvgTextBounds(vg, box.size.x * 0.5, 0, label.c_str(), nullptr, textBox);
        nvgFill(vg);

        float yline = (textBox[1] + textBox[3]) * 0.5 - rack::mm2px(0.1);
        float x0 = rack::mm2px(1.3);
        float x1 = box.size.x - x0;

        nvgBeginPath(vg);
        nvgMoveTo(vg, x0, shortLeft ? box.size.y - rack::mm2px(2.5) : box.size.y);
        nvgLineTo(vg, x0, yline + 3);
        nvgArcTo(vg, x0, yline, x0 + 2, yline, 2);
        nvgLineTo(vg, textBox[0] - 2, yline);
        nvgStrokeWidth(vg, 1.2);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgMoveTo(vg, x1, shortRight ? box.size.y - rack::mm2px(2.5) : box.size.y);
        nvgLineTo(vg, x1, yline + 3);
        nvgArcTo(vg, x1, yline, x1 - 2, yline, 2);
        nvgLineTo(vg, textBox[2] + 2, yline);
        nvgStrokeWidth(vg, 1.2);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
        nvgStroke(vg);
    }

    void onStyleChanged() override { bdw->dirty = true; }
};

struct ActivateKnobSwitch : rack::app::Switch, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    float radius;

    enum RenderType
    {
        POWER,
        EXTENDED
    } type{POWER};

    ActivateKnobSwitch()
    {
        box.size = rack::mm2px(rack::Vec(3.5, 3.5));
        radius = rack::mm2px(1.1);
        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                             [this](auto v) { this->drawBackground(v); });
        bdwLight = new BufferedDrawFunctionWidgetOnLayer(rack::Vec(0, 0), box.size,
                                                         [this](auto v) { this->drawLight(v); });
        addChild(bdw);
        addChild(bdwLight);
    }

    bool hovered{false};
    void onHover(const HoverEvent &e) override
    {
        e.consume(this);
        rack::app::Switch::onHover(e);
    }
    void onEnter(const EnterEvent &e) override
    {
        hovered = true;
        bdw->dirty = true;
        e.consume(this);

        rack::app::Switch::onEnter(e);
    }
    void onLeave(const LeaveEvent &e) override
    {
        hovered = false;
        bdw->dirty = true;
        e.consume(this);

        rack::app::Switch::onLeave(e);
    }

    void setupExtendedPath(NVGcontext *vg)
    {
        const float crossWidth = rack::mm2px(0.6);
        const float crossRadius = crossWidth / 2;
        const float shrinkBy = rack::mm2px(0.6);
        auto ctrx = box.size.x * 0.5;
        auto ctry = box.size.y * 0.5;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, ctrx - crossRadius, shrinkBy, crossWidth, box.size.y - 2 * shrinkBy,
                       crossRadius);
        nvgRoundedRect(vg, shrinkBy, ctry - crossRadius, box.size.x - 2 * shrinkBy, crossWidth,
                       crossRadius);
    }

    void drawBackground(NVGcontext *vg)
    {
        auto col = style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_OFF);
        if (hovered)
        {
            col.r *= 1.2;
            col.g *= 1.2;
            col.b *= 1.2;
        }
        if (type == POWER)
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
            nvgFillColor(vg, col);
            nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, radius, radius);
            nvgFill(vg);
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }
        if (type == EXTENDED)
        {
            setupExtendedPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
            nvgFillColor(vg, col);
            nvgStrokeWidth(vg, 1.2);
            nvgStroke(vg);
            nvgFill(vg);
        }
    }

    void drawLight(NVGcontext *vg)
    {
        if (!getParamQuantity())
            return;
        auto q = getParamQuantity()->getValue();
        if (q < 0.5)
            return;

        const float halo = rack::settings::haloBrightness;

        if (halo > 0.f)
        {
            nvgBeginPath(vg);
            nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, box.size.x * 0.5, box.size.x * 0.5);

            auto pcol = style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_ON);
            NVGcolor icol = pcol;
            icol.a = halo;
            NVGcolor ocol = pcol;
            ocol.a = 0.f;
            NVGpaint paint = nvgRadialGradient(vg, box.size.x * 0.5, box.size.y * 0.5, radius,
                                               box.size.x * 0.5, icol, ocol);
            nvgFillPaint(vg, paint);
            nvgFill(vg);

            drawBackground(vg);
        }

        if (type == POWER)
        {
            nvgBeginPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_ON));
            nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, radius * 0.9, radius * 0.9);
            nvgFill(vg);
        }
        if (type == EXTENDED)
        {
            setupExtendedPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_ON));
            nvgFill(vg);
        }
    }

    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
        Widget::onChange(e);
    }

    float phalo{0.f};
    void step() override
    {
        const float halo = rack::settings::haloBrightness;
        if (phalo != halo)
        {
            phalo = halo;
            bdw->dirty = true;
            bdwLight->dirty = true;
        }
        Switch::step();
    }
    void onStyleChanged() override {}
};

template <typename T> struct GlowOverlayHoverButton : T, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bw{nullptr}, *bwGlow{nullptr}, *bwShadow{nullptr};

    bool pressedState{false};
    rack::SvgWidget *svg{nullptr};

    float button_MM = 6.5, light_MM = 2.75;
    float light_pixelRadius = rack::mm2px(light_MM) * 0.5;

    GlowOverlayHoverButton()
    {
        svg = new rack::widget::SvgWidget();
        svg->box.pos.x = 0;
        svg->box.pos.y = 0;
        // this is a hack to load the SVG and get the size
        onStyleChanged();
        this->box.size = svg->box.size;

        bw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), this->box.size,
                                            [this](auto vg) { this->drawButtonBG(vg); });

        bwGlow = new BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), this->box.size, [this](auto vg) { this->drawButtonGlow(vg); });

        T::addChild(svg);
        T::addChild(bw);
        T::addChild(bwGlow);

        auto shadowSize = this->box;
        shadowSize.pos.y += rack::mm2px(0.5);
        shadowSize = shadowSize.grow(2);

        bwShadow = new BufferedDrawFunctionWidget(shadowSize.pos, shadowSize.size,
                                                  [this](auto vg) { this->drawShadow(vg); });
        T::addChildBottom(bwShadow);
    }

    void drawShadow(NVGcontext *vg)
    {
        if (!style::XTStyle::getShowShadows())
            return;

        auto knobSize_MM = button_MM;
        auto shadowOneH_MM = knobSize_MM * 0.9911;
        auto shadowOneW_MM = knobSize_MM - 0.5;
        auto shadowTwoH_MM = knobSize_MM * 1.0222;
        auto shadowTwoW_MM = shadowOneW_MM;
        auto shadowTwoOffset_MM{0.375f};
        auto shadowSize = this->box;
        shadowSize = shadowSize.grow(2);
        auto box = shadowSize;

        nvgBeginPath(vg);
        nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, rack::mm2px(shadowOneW_MM) * 0.5,
                   rack::mm2px(shadowOneH_MM) * 0.5);
        nvgFillColor(vg, style()->getColor(style::XTStyle::SHADOW_BASE));
        nvgFill(vg);

        auto s2 = rack::mm2px(shadowTwoOffset_MM);
        nvgSave(vg);
        nvgScissor(vg, 0, box.size.y * 0.5, box.size.x, box.size.y * 0.5);
        nvgBeginPath(vg);
        auto start = box.size.y * 0.5 + s2 + rack::mm2px(shadowTwoH_MM) * 0.25;
        auto end = box.size.y * 0.5 + s2 + rack::mm2px(shadowTwoH_MM) * 0.5;
        nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5 + s2, rack::mm2px(shadowTwoW_MM) * 0.5,
                   rack::mm2px(shadowTwoH_MM) * 0.5);
        nvgFillPaint(vg, nvgLinearGradient(vg, 0, start, 0, end,
                                           style()->getColor(style::XTStyle::SHADOW_OVER_GRADSTART),
                                           style()->getColor(style::XTStyle::SHADOW_OVER_GRADEND)));
        nvgFill(vg);

        nvgRestore(vg);
    }

    bool hovered{false};
    void onHover(const typename T::HoverEvent &e) override
    {
        e.consume(this);
        T::onHover(e);
    }
    void onEnter(const typename T::EnterEvent &e) override
    {
        hovered = true;
        bw->dirty = true;
        e.consume(this);
        T::onEnter(e);
    }
    void onLeave(const typename T::LeaveEvent &e) override
    {
        hovered = false;
        bw->dirty = true;
        e.consume(this);
        T::onLeave(e);
    }

    void drawButtonGlow(NVGcontext *vg)
    {
        if (!pressedState)
            return;

        const float halo = rack::settings::haloBrightness;
        auto c = this->box.size.div(2);

        if (halo > 0.f)
        {
            float radius = light_pixelRadius;
            float oradius = rack::mm2px(button_MM) * 0.5;

            nvgBeginPath(vg);
            nvgRect(vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

            NVGcolor icol =
                rack::color::mult(style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_ON), halo);
            NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
            NVGpaint paint = nvgRadialGradient(vg, c.x, c.y, radius, oradius, icol, ocol);
            nvgFillPaint(vg, paint);
            nvgFill(vg);
        }
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_ON));
        nvgEllipse(vg, c.x, c.y, light_pixelRadius, light_pixelRadius);
        nvgFill(vg);
    }

    void drawButtonBG(NVGcontext *vg)
    {
        if (!pressedState)
        {
            nvgBeginPath(vg);
            auto col = style()->getColor(style::XTStyle::MOD_BUTTON_LIGHT_OFF);
            if (hovered)
            {
                col.r *= 1.2;
                col.g *= 1.2;
                col.b *= 1.2;
            }
            nvgFillColor(vg, col);
            nvgEllipse(vg, this->box.size.x / 2, this->box.size.y / 2, light_pixelRadius,
                       light_pixelRadius);
            nvgFill(vg);
        }
    }

    float phalo{0.f};
    void step() override
    {
        const float halo = rack::settings::haloBrightness;
        if (phalo != halo)
        {
            phalo = halo;
            bw->dirty = true;
            bwGlow->dirty = true;
        }
        T::step();
    }

    void setState(bool b)
    {
        pressedState = b;
        bw->dirty = true;
        bwGlow->dirty = true;
    }

    void onStyleChanged() override
    {
        svg->setSvg(rack::Svg::load(rack::asset::plugin(
            pluginInstance, style()->skinAssetDir() + "/components/mod-button.svg")));
        if (bw)
            bw->dirty = true;
        if (bwGlow)
            bwGlow->dirty = true;
        if (bwShadow)
            bwShadow->dirty = true;
    }
};

struct ModToggleButton : GlowOverlayHoverButton<rack::widget::Widget>
{
    std::function<void(bool)> onToggle = [](bool isOn) {};

    bool armed{false};
    void onButton(const ButtonEvent &e) override
    {
        if (e.action == GLFW_PRESS)
        {
            armed = true;
            e.consume(this);
        }
        if (armed && e.action == GLFW_RELEASE)
        {
            setState(!pressedState);
            onToggle(pressedState);
            armed = false;
            e.consume(this);
        }
    }

    void onLeave(const LeaveEvent &e) override
    {
        armed = false;
        GlowOverlayHoverButton<rack::widget::Widget>::onLeave(e);
    }
};

struct MomentaryParamButton : GlowOverlayHoverButton<rack::app::ParamWidget>
{
    void onButton(const ButtonEvent &e) override
    {
        if (e.action == GLFW_PRESS)
        {
            setState(true);
            getParamQuantity()->setValue(1.0);
            e.consume(this);
        }
        if (e.action == GLFW_RELEASE)
        {
            setState(false);
            getParamQuantity()->setValue(0.0);
            e.consume(this);
        }
    }
};

struct ToggleParamButton : GlowOverlayHoverButton<rack::app::ParamWidget>
{
    void onButton(const ButtonEvent &e) override
    {
        if (e.action == GLFW_PRESS)
        {
            auto ns = !pressedState;
            setState(ns);
            getParamQuantity()->setValue(ns ? 1 : 0);
            e.consume(this);
        }
    }

    void step() override
    {
        if (getParamQuantity())
        {
            bool o = getParamQuantity()->getValue() > 0.5;
            if (o != pressedState)
                setState(o);
        }
        ParamWidget::step();
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
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        nvgText(vg, 0, box.size.y * 0.5, label.c_str(), nullptr);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
        nvgRect(vg, box.size.x - box_px, 0, box_px, box.size.y);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_FG));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        nvgText(vg, box.size.x - box_px * 0.5, box.size.y * 0.5, pv.c_str(), nullptr);
    }

    void onStyleChanged() override { bdw->dirty = true; }
    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        Widget::onChange(e);
    }

    bool parameterMenuOnClick{true};
    int minScale{0}, maxScale{0}, maxVal{0};
    double timeOfAction{-1}, timeOfDoubleClick{-1};
    bool didAction{false}, didDoubleClick{false};

    void onAction(const ActionEvent &e) override
    {
        didAction = true;
        timeOfAction = rack::system::getTime();
        Knob::onAction(e);
    }
    void onDoubleClick(const DoubleClickEvent &e) override
    {
        didDoubleClick = true;
        timeOfDoubleClick = rack::system::getTime();
        ParamWidget::onDoubleClick(e);
    }
    void step() override
    {
        if (didAction && !didDoubleClick)
        {
            if (rack::system::getTime() - timeOfAction > 0.15)
            {
                showQuantityMenu();
                didAction = false;
            }
        }
        if (didDoubleClick)
        {
            if (rack::system::getTime() - timeOfDoubleClick > 0.3)
            {
                didDoubleClick = false;
            }
            didAction = false;
        }
        rack::Knob::step();
    }
    void showQuantityMenu()
    {
        if (parameterMenuOnClick && getParamQuantity())
        {
            auto pq = getParamQuantity();
            if (pq->snapEnabled)
            {
                auto men = rack::createMenu();
                men->addChild(rack::createMenuLabel(pq->getLabel()));
                auto v = (int)std::round(pq->getValue());
                for (int i = pq->getMaxValue(); i >= pq->getMinValue(); --i)
                {
                    men->addChild(rack::createMenuItem(std::to_string(i), CHECKMARK(i == v),
                                                       [pq, i]() { pq->setValue(i); }));
                }
            }
            else if (minScale != maxScale)
            {
                auto men = rack::createMenu();
                men->addChild(rack::createMenuLabel(pq->getLabel()));

                auto v = Parameter::intUnscaledFromFloat(pq->getValue(), maxScale, minScale);
                for (int i = maxVal; i >= minScale; --i)
                {
                    men->addChild(
                        rack::createMenuItem(std::to_string(i), CHECKMARK(i == v), [this, pq, i]() {
                            pq->setValue(Parameter::intScaledToFloat(i, maxScale, minScale));
                        }));
                }
            }
        }
    }
};

struct PlotAreaSwitch : public rack::app::Switch, style::StyleParticipant
{
    static constexpr float padTop_MM = 1.4;
    static constexpr float padBot_MM = 1.6;
    static constexpr float box_px = 13;
    BufferedDrawFunctionWidget *bdw{nullptr};
    std::string label;

    static PlotAreaSwitch *create(rack::Vec pos, rack::Vec sz, const std::string &lab,
                                  rack::Module *module, int paramId)
    {
        auto *res = rack::createWidget<PlotAreaSwitch>(pos);

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

        return res;
    }

    void drawWidget(NVGcontext *vg)
    {
        bool isOn{false};
        auto *pq = getParamQuantity();
        if (pq)
            isOn = pq->getValue() > 0.5;

        if (!isOn)
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
            nvgRect(vg, 0.5, 0.5, box.size.x - 1, box.size.y - 1);
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);

            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, label.c_str(), nullptr);
        }
        else
        {
            nvgBeginPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
            nvgRect(vg, 0, 0, box.size.x, box.size.y);
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_FG));
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, label.c_str(), nullptr);
        }
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
    std::function<std::string(const std::string &)> transformLabel;
    std::function<void()> onShowMenu = []() {};
    bool upcaseDisplay{true};
    bool centerDisplay{false};

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
        res->transformLabel = [](const std::string &s) { return s; };

        return res;
    }

    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        if (!pq)
            return;

        auto pv = pq->getDisplayValueString();
        if (upcaseDisplay)
            for (auto &q : pv)
                q = std::toupper(q);
        pv = transformLabel(pv);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        if (centerDisplay)
        {
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, pv.c_str(), nullptr);
        }
        else
        {
            nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
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

struct PlotAreaToggleClick : public rack::app::Switch, style::StyleParticipant
{
    static constexpr float padTop_MM = 1.4;
    static constexpr float padBot_MM = 1.6;
    BufferedDrawFunctionWidget *bdw{nullptr};
    enum Align
    {
        LEFT,
        RIGHT,
        CENTER
    } align{RIGHT};

    static PlotAreaToggleClick *create(rack::Vec pos, rack::Vec sz, rack::Module *module,
                                       int paramId)
    {
        auto *res = rack::createWidget<PlotAreaToggleClick>(pos);

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

        return res;
    }

    std::function<std::string(rack::ParamQuantity *q)> getDisplay = [](auto *p) {
        return p->getDisplayValueString();
    };
    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        if (!pq)
            return;

        auto pv = getDisplay(pq);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        if (align == LEFT)
        {
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
            nvgText(vg, 0, box.size.y * 0.5, pv.c_str(), nullptr);
        }
        else if (align == CENTER)
        {
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, pv.c_str(), nullptr);
        }
        else
        {
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
            nvgText(vg, box.size.x, box.size.y * 0.5, pv.c_str(), nullptr);
        }

        /*
        nvgBeginPath(vg);
        nvgRect(vg, 0,0,box.size.x, box.size.y);
        nvgStrokeColor(vg,nvgRGB(255,0,255));
        nvgStroke(vg);
         */
    }

    void onStyleChanged() override { bdw->dirty = true; }
    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        Widget::onChange(e);
    }
};
template <typename T> struct GenericPresetJogSelector : public T, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};

    rack::Vec leftJogSize, rightJogSize, leftJogPos, rightJogPos;

    virtual void setup()
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
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        nvgText(vg, this->box.size.x * 0.5, this->box.size.y * 0.5, getPresetName().c_str(),
                nullptr);

        if (hasPresets())
        {
            // left arrow
            float onemm = rack::mm2px(1.2);
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

    void onDoubleClick(const rack::widget::Widget::DoubleClickEvent &e) override
    {
        // Fast click is just go ahead not double click.
        e.consume(this);
    }

    void onStyleChanged() override {}
};

typedef GenericPresetJogSelector<rack::Widget> PresetJogSelector;
typedef GenericPresetJogSelector<rack::ParamWidget> ParamJogSelector;

struct SteppedParamAsPresetJog : GenericPresetJogSelector<rack::ParamWidget>
{
    int lastValue{-1};
    void setup() override
    {
        GenericPresetJogSelector<rack::ParamWidget>::setup();
        if (module)
            initParamQuantity();
    }

    rack::engine::SwitchQuantity *getQuantity()
    {
        if (!module)
            return nullptr;
        return dynamic_cast<rack::engine::SwitchQuantity *>(module->paramQuantities[paramId]);
    }

    void onPresetJog(int dir /* +/- 1 */) override
    {
        auto sq = getQuantity();
        if (!sq)
            return;
        auto v = (int)std::round(sq->getValue());
        v += dir;
        if (v < sq->getMinValue())
            v = sq->getMaxValue();
        else if (v > sq->getMaxValue())
            v = sq->getMinValue();
        sq->setValue(v);
    }
    void onShowMenu() override
    {
        auto sq = getQuantity();
        if (!sq)
            return;

        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel(sq->getLabel()));
        menu->addChild(new rack::ui::MenuSeparator);
        float minValue = sq->getMinValue();
        int index = (int)std::floor(sq->getValue() - minValue);
        int numStates = sq->labels.size();
        for (int i = 0; i < numStates; i++)
        {
            std::string label = sq->labels[i];
            menu->addChild(
                rack::createMenuItem(label, CHECKMARK(index == i), [sq, i]() { sq->setValue(i); }));
        }
    }
    std::string getPresetName() override
    {
        auto sq = getQuantity();
        if (!sq)
            return "ERROR";
        auto v = std::clamp((int)std::round(sq->getValue() - sq->getMinValue()), 0,
                            (int)(sq->labels.size() - 1));
        return sq->labels[v];
    }
    bool isDirty() override
    {
        auto sq = getQuantity();
        if (!sq)
            return false;
        auto v = (int)std::round(sq->getValue());
        bool res = v != lastValue;
        lastValue = v;
        return res;
    };
};

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
    int noModuleSize{17};
    static constexpr float posx = 12.08506f;
    static constexpr float posx_MM = posx * 25.4 / 75.0;
    static constexpr float posy = 25.408199;
    static constexpr float posy_MM = posy * 25.4 / 75.0;

    static constexpr float contentPosX_MM = 5.08; // turns out this is 15px
    static constexpr float contentPosY_MM = 27.35 - 36 * 0.5;

    static constexpr float padX_MM = contentPosX_MM - posx_MM;
    static constexpr float padY_MM = contentPosY_MM - posy_MM;

    bool centerRule{false};
    bool splitLower{false};

    static LCDBackground *createWithHeight(float endPosInMM, float widthInScrews = 12,
                                           float startPosY = posy)
    {
        auto width = rack::app::RACK_GRID_WIDTH * widthInScrews - 2 * posx;
        auto height = rack::mm2px(endPosInMM) - startPosY;

        auto res = new LCDBackground();
        res->setup(rack::Vec(posx, startPosY), rack::Vec(width, height));

        return res;
    }

    static LCDBackground *createAtYPosition(float startY, float height, float widthInScrews = 12,
                                            float posxDiff = 0)
    {
        auto width = rack::app::RACK_GRID_WIDTH * widthInScrews - 2 * (posx - posxDiff);

        auto res = new LCDBackground();
        res->setup(rack::Vec(posx - posxDiff, startY), rack::Vec(width, height));

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

        if (centerRule)
        {
            auto yc = box.size.y * 0.5;
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgMoveTo(vg, rack::mm2px(2 * padX_MM), yc);
            nvgLineTo(vg, box.size.x - rack::mm2px(2 * padX_MM), yc);
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);
        }

        if (splitLower)
        {
            auto yc = box.size.y * 0.5;
            auto xc = box.size.x * 0.5;
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgMoveTo(vg, xc, yc + rack::mm2px(padY_MM));
            nvgLineTo(vg, xc, box.size.y - rack::mm2px(padY_MM));
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);
        }

        if (!noModuleText.empty())
        {
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, noModuleSize);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, noModuleText.c_str(), nullptr);
        }
    }
    void onStyleChanged() override { bdw->dirty = true; }
};

template <int nLights = 4>
struct ThereAreFourLights : rack::app::SliderKnob, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdwRings{nullptr}, *bdwLight{nullptr};

    static constexpr float ring_MM{2.6}, pad_MM{1.3}, halo_MM{0.75};

    static ThereAreFourLights<nLights> *createCentered(rack::Vec pos, rack::Module *module,
                                                       int paramId)
    {
        auto *res = rack::createWidget<ThereAreFourLights<nLights>>(pos);

        res->box.size.x = rack::mm2px(ring_MM + 2 * halo_MM);
        res->box.size.y = rack::mm2px(ring_MM) * nLights + rack::mm2px(pad_MM) * (nLights - 1) +
                          2 * rack::mm2px(halo_MM);
        res->box.pos.x = pos.x - res->box.size.x / 2.f;
        res->box.pos.y = pos.y - res->box.size.y / 2.f;
        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        res->bdwLight = new BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), res->box.size, [res](NVGcontext *vg) { res->drawLight(vg); });
        res->addChild(res->bdwLight);

        res->bdwRings = new BufferedDrawFunctionWidget(
            rack::Vec(0, 0), res->box.size, [res](NVGcontext *vg) { res->drawRings(vg); });
        res->addChild(res->bdwRings);
        return res;
    }

    void drawRings(NVGcontext *vg)
    {
        auto ringpx = rack::mm2px(ring_MM);
        auto padpx = rack::mm2px(pad_MM);
        auto halopx = rack::mm2px(halo_MM);
        for (int i = 0; i < nLights; ++i)
        {
            auto y0 = i * (ringpx + padpx) + halopx;
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
            nvgFillColor(vg, style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_OFF));
            nvgEllipse(vg, box.size.x * 0.5, y0 + ringpx * 0.5, ringpx * 0.5, ringpx * 0.5);
            nvgFill(vg);
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);
        }
    }
    void drawLight(NVGcontext *vg)
    {
        if (!getParamQuantity())
            return;
        auto ringpx = rack::mm2px(ring_MM);
        auto padpx = rack::mm2px(pad_MM);
        auto halopx = rack::mm2px(halo_MM);

        const float halo = rack::settings::haloBrightness;

        auto pq = nLights - 1 -
                  Parameter::intUnscaledFromFloat(getParamQuantity()->getValue(), nLights - 1);
        auto y0 = pq * (ringpx + padpx) + halopx;

        if (halo > 0.f)
        {
            nvgBeginPath(vg);
            nvgEllipse(vg, box.size.x * 0.5, y0 + ringpx * 0.5, ringpx * 0.5 + halopx,
                       ringpx * 0.5 + halopx);

            auto pcol = style()->getColor(style::XTStyle::KNOB_RING_VALUE);
            NVGcolor icol = pcol;
            icol.a = halo;
            NVGcolor ocol = pcol;
            ocol.a = 0.f;
            NVGpaint paint = nvgRadialGradient(vg, box.size.x * 0.5, y0 + ringpx * 0.5,
                                               ringpx * 0.5, ringpx * 0.5 + halopx, icol, ocol);
            nvgFillPaint(vg, paint);
            nvgFill(vg);
        }

        nvgBeginPath(vg);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
        // This is a value slider not a power button
        nvgFillColor(vg, style()->getColor(style::XTStyle::KNOB_RING_VALUE));
        nvgEllipse(vg, box.size.x * 0.5, y0 + ringpx * 0.5, ringpx * 0.5, ringpx * 0.5);
        nvgFill(vg);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
    }

    void onChange(const ChangeEvent &e) override
    {
        bdwLight->dirty = true;
        bdwRings->dirty = true;
        rack::app::Knob::onChange(e);
    }

    void onStyleChanged() override
    {
        bdwLight->dirty = true;
        bdwRings->dirty = true;
    }

    float phalo{0.f};
    void step() override
    {
        const float halo = rack::settings::haloBrightness;
        if (phalo != halo)
        {
            phalo = halo;
            bdwRings->dirty = true;
            bdwLight->dirty = true;
        }
        SliderKnob::step();
    }

    float buttonY{-1};
    void onButton(const ButtonEvent &e) override
    {
        buttonY = e.pos.y;
        SliderKnob::onButton(e);
    }
    void onAction(const ActionEvent &e) override
    {
        auto ringpx = rack::mm2px(ring_MM);
        auto padpx = rack::mm2px(pad_MM);
        auto halopx = rack::mm2px(halo_MM);
        for (int i = 0; i < nLights; ++i)
        {
            auto y0 = i * (ringpx + padpx) + halopx;

            if (buttonY >= y0 && buttonY <= y0 + ringpx)
            {
                if (getParamQuantity())
                {
                    getParamQuantity()->setValue(
                        Parameter::intScaledToFloat(nLights - 1 - i, nLights - 1));
                }
            }
        }
        Knob::onAction(e);
    }

    void appendContextMenu(rack::Menu *menu) override
    {
        auto pq = getParamQuantity();
        auto spq = dynamic_cast<modules::SurgeParameterParamQuantity *>(pq);

        if (!spq)
            return;

        while (menu->children.size() > 1)
        {
            auto back = menu->children.back();
            menu->removeChild(back);
            delete back;
        }

        auto pqv = Parameter::intUnscaledFromFloat(getParamQuantity()->getValue(), nLights - 1);

        for (int i = 0; i < nLights; ++i)
        {
            auto fval = Parameter::intScaledToFloat(i, nLights - 1);
            auto sval = spq->getDisplayValueStringForValue(fval);
            menu->addChild(rack::createMenuItem(sval, CHECKMARK(i == pqv),
                                                [pq, fval]() { pq->setValue(fval); }));
        }

        menu->addChild(new rack::MenuSeparator);
        menu->addChild(
            rack::createMenuItem("Initialize", "Double-click", [this]() { this->resetAction(); }));
    }
};

struct VerticalSliderModulator;

struct VerticalSlider : rack::app::SliderKnob, style::StyleParticipant, ModulatableKnob
{
    rack::widget::FramebufferWidget *baseFB{nullptr}, *handleFB{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    rack::widget::SvgWidget *tray{nullptr}, *handle{nullptr};

    std::unordered_set<VerticalSliderModulator *> modSliders;
    std::string bgname;

    static VerticalSlider *createCentered(const rack::Vec &pos, float height,
                                          modules::XTModule *module, int paramId,
                                          const std::string bgsvg = "fader_bg.svg")
    {
        auto res = new VerticalSlider();

        auto compDir = res->style()->skinAssetDir() + "/components";
        res->bgname = bgsvg;
        auto bg = rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/" + bgsvg));

        auto sz = rack::Vec(5, 20);
        if (bg)
            sz = bg->getSize();
        res->box.pos = pos;
        res->box.pos.x -= sz.x / 2;
        res->box.pos.y -= sz.y / 2;
        res->box.size = sz;

        res->setup();
        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        return res;
    }

    float modDepthForAnimation()
    {
        auto xtm = dynamic_cast<modules::XTModule *>(module);
        if (!style::XTStyle::getShowModulationAnimationOnKnobs())
            return 0;

        if (xtm)
            return xtm->modulationDisplayValue(paramId);
        return 0;
    }

    void setup()
    {
        baseFB = new rack::widget::FramebufferWidget();
        baseFB->box.size = box.size;
        baseFB->box.pos = rack::Vec(0, 0);

        handleFB = new rack::widget::FramebufferWidget();
        handleFB->box.size = box.size;
        handleFB->box.pos = rack::Vec(0, 0);

        tray = new rack::SvgWidget();
        handle = new rack::SvgWidget();
        auto compDir = style()->skinAssetDir() + "/components";

        tray->setSvg(rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/" + bgname)));
        baseFB->addChild(tray);

        handle->setSvg(
            rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/fader_handle.svg")));
        handle->box.pos.x = 1;
        handle->box.pos.y = 0;
        handleFB->addChild(handle);

        bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                             [this](auto *vg) { this->drawSlider(vg); });
        bdwLight = new BufferedDrawFunctionWidgetOnLayer(rack::Vec(0, 0), box.size,
                                                         [this](auto *vg) { this->drawLight(vg); });

        addChild(baseFB);
        addChild(bdw);
        addChild(handleFB);
        addChild(bdwLight);
        baseFB->setDirty();
        bdw->setDirty();
        bdwLight->setDirty();
        handleFB->setDirty();

        speed = 2.0;
    }

    void onChange(const ChangeEvent &e) override;

    void positionHandleByQuantity()
    {
        auto pq = getParamQuantity();
        if (!pq || !handle || handle->box.size.y < 1 || !tray || tray->box.size.y < 1)
            return;

        auto npos = (pq->getValue() - pq->getMinValue()) / (pq->getMaxValue() - pq->getMinValue());
        npos = 1.0 - npos;
        auto hsize = handle->box.size.y;
        auto tsize = tray->box.size.y;
        auto span = tsize - hsize - 2;
        handle->box.pos.y = npos * span + 1;
        handleFB->dirty = true;
    }

    float priorV{-103241.f};
    float priorModV{-13824.f};
    void step() override
    {
        auto pq = getParamQuantity();
        if (!pq)
            return;
        if (pq->getValue() != priorV)
        {
            positionHandleByQuantity();
            bdw->dirty = true;
            bdwLight->dirty = true;

            priorV = pq->getValue();
        }

        auto v = modDepthForAnimation();
        if (v != priorModV)
        {
            priorModV = v;
            bdwLight->dirty = true;
        }

        rack::app::SliderKnob::step();
    }

    void drawSlider(NVGcontext *vg)
    {
        auto rwidth = rack::mm2px(0.88);
        auto hsize = handle->box.size.y;
        auto tsize = tray->box.size.y;
        auto off = rack::mm2px(0.4);
        auto span = box.size.y - 2 * off;

        nvgBeginPath(vg);
        nvgRect(vg, box.size.x * 0.5 - rwidth * 0.5, off, rwidth, span);
        nvgFillColor(vg, style()->getColor(style::XTStyle::LED_PANEL));
        nvgFill(vg);
    }

    void drawLight(NVGcontext *vg)
    {
        auto pq = getParamQuantity();
        if (!pq || !handle || handle->box.size.y < 1 || !tray || tray->box.size.y < 1)
            return;

        if (isModEdit)
            return;

        auto rwidth = rack::mm2px(0.88);
        auto hsize = handle->box.size.y;
        auto tsize = tray->box.size.y;
        auto off = rack::mm2px(0.4);
        auto span = box.size.y - 2 * off;

        auto nv{0.f};
        if (pq)
        {
            nv = (pq->getValue() - pq->getMinValue()) / (pq->getMaxValue() - pq->getMinValue());
        }
        auto np = (1 - nv) * span;

        auto sp = handle->box.pos.y + handle->box.size.y;
        nvgSave(vg);
        nvgScissor(vg, 0, sp, box.size.x, box.size.y - sp);

        nvgBeginPath(vg);
        nvgRect(vg, box.size.x * 0.5 - rwidth * 0.5, np + off, rwidth, span - np);
        nvgFillColor(vg, style()->getColor(style::XTStyle::SLIDER_RING_VALUE));
        nvgFill(vg);
        nvgStrokeWidth(vg, 0.5);
        nvgStroke(vg);
        nvgRestore(vg);

        if (style::XTStyle::getShowModulationAnimationOnKnobs())
        {
            auto v = modDepthForAnimation(); // this is units -1..1 in scale
            if (v == 0.f)
                return;

            float startY, endY;
            startY = np + 1.0;
            endY = startY - (v * box.getHeight());
            if (startY > endY)
                std::swap(startY, endY);

            // Draw above the handle
            auto hp = handle->box.pos;
            auto hs = handle->box.size;

            auto c = style()->getColor(style::XTStyle::KNOB_MOD_PLUS);

            auto inset = box.size.x * 0.5 - rwidth * 0.5;
            auto height = endY - startY;
            nvgSave(vg);
            if (v > 0)
                nvgScissor(vg, 0, 0, box.size.x, hp.y);
            else
                nvgScissor(vg, 0, hp.y + hs.y, box.size.x, box.size.y - hp.y - hs.y);

            nvgBeginPath(vg);
            nvgRect(vg, inset, startY, rwidth, height);
            nvgFillColor(vg, c);
            nvgFill(vg);
            nvgRestore(vg);

            // And on transparently
            nvgSave(vg);
            nvgScissor(vg, 0, hp.y, box.size.x, hs.y);
            nvgBeginPath(vg);
            nvgRect(vg, inset, startY, rwidth, height);
            c.a = 0.3;
            nvgFillColor(vg, c);
            nvgFill(vg);
            nvgRestore(vg);
        }
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;

        auto compDir = style()->skinAssetDir() + "/components";

        auto ts = rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/" + bgname));
        auto hs =
            rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/fader_handle.svg"));

        tray->setSvg(ts);
        handle->setSvg(hs);
    }
    Widget *asWidget() override { return this; }

    bool isModEdit{false};
    void setIsModEditing(bool b) override
    {
        isModEdit = b;
        bdw->dirty = true;
        bdwLight->dirty = true;
    }
};

struct VerticalSliderModulator : rack::SliderKnob, style::StyleParticipant, HasBDW
{
    VerticalSlider *underlyerParamWidget{nullptr};

    int modIndex{0};

    VerticalSliderModulator() { box.size = rack::Vec(45, 45); }
    ~VerticalSliderModulator();

    void drawWidget(NVGcontext *vg)
    {
        auto *pq = getParamQuantity();
        auto *uq = underlyerParamWidget->getParamQuantity();
        if (!pq || !uq)
            return;

        auto off = rack::mm2px(0.4);
        auto span = box.size.y - 2 * off;

        auto uv = (uq->getValue() - uq->getMinValue()) / (uq->getMaxValue() - uq->getMinValue());

        auto np = (1 - uv) * span + off;

        auto mv = pq->getValue();
        auto mp = std::clamp(1.f - (uv + mv), 0.f, 1.f) * span + off;
        auto dp = std::clamp(1.f - (uv - mv), 0.f, 1.f) * span + off;

        auto hp = underlyerParamWidget->handle->box.pos;
        auto hs = underlyerParamWidget->handle->box.size;

        for (const auto &[v, h, c] : {std::make_tuple(mp, mp - np, style::XTStyle::KNOB_MOD_PLUS),
                                      std::make_tuple(dp, np - dp, style::XTStyle::KNOB_MOD_MINUS)})
        {
            auto start = std::min(v, np);
            auto height = fabs(h);
            auto inset = rack::mm2px(1.5f);

            // Draw above the handle
            nvgSave(vg);
            nvgScissor(vg, 0, 0, box.size.x, hp.y);
            nvgBeginPath(vg);
            nvgRect(vg, inset, start, box.size.x - 2 * inset, height);
            nvgFillColor(vg, style()->getColor(c));
            nvgFill(vg);
            nvgRestore(vg);

            // And below
            nvgSave(vg);
            nvgScissor(vg, 0, hp.y + hs.y, box.size.x, box.size.y - hp.y - hs.y);
            nvgBeginPath(vg);
            nvgRect(vg, inset, start, box.size.x - 2 * inset, height);
            nvgFillColor(vg, style()->getColor(c));
            nvgFill(vg);
            nvgRestore(vg);

            // And on transparently
            nvgSave(vg);
            nvgScissor(vg, 0, hp.y, box.size.x, hs.y);
            nvgBeginPath(vg);
            nvgRect(vg, inset, start, box.size.x - 2 * inset, height);
            auto co = style()->getColor(c);
            co.a = 0.3;
            nvgFillColor(vg, co);
            nvgFill(vg);
            nvgRestore(vg);
        }

        if (0)
        {
            auto start = std::min(np, dp);
            auto height = fabs(np - dp);
            auto inset = rack::mm2px(1.5f);

            nvgBeginPath(vg);
            nvgRect(vg, inset, start, box.size.x - 2 * inset, height);
            nvgFillColor(vg, style()->getColor(style::XTStyle::KNOB_MOD_MINUS));
            nvgFill(vg);
        }
    }

    static VerticalSliderModulator *createCentered(rack::Vec pos, float heightMM,
                                                   rack::Module *module, int paramId)
    {
        auto *res = rack::createWidget<VerticalSliderModulator>(pos);

        auto width = rack::mm2px(4);
        auto height = rack::mm2px(heightMM);
        res->box.pos = pos;
        res->box.pos.x -= width / 2;
        res->box.pos.y -= height / 2;
        res->box.size = rack::Vec(width, height);
        res->bdw = new BufferedDrawFunctionWidgetOnLayer(rack::Vec(0, 0), res->box.size,
                                                         [res](auto *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);

        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();

        return res;
    }

    void onChange(const ChangeEvent &e) override
    {
        if (bdw)
            bdw->dirty = true;

        rack::app::SliderKnob::onChange(e);
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
            rack::SliderKnob::onHover(e);
    }
    void onButton(const ButtonEvent &e) override
    {
        if (!bypassGesture())
        {
            auto spq =
                dynamic_cast<modules::SurgeParameterModulationQuantity *>(getParamQuantity());
            if (spq)
                spq->abbreviate = true;
            rack::SliderKnob::onButton(e);
            if (spq)
                spq->abbreviate = false;
        }
    }
    void onDragStart(const DragStartEvent &e) override
    {
        if (!bypassGesture())
            rack::SliderKnob::onDragStart(e);
    }
    void onDragEnd(const DragEndEvent &e) override
    {
        if (!bypassGesture())
            rack::SliderKnob::onDragEnd(e);
    }
    void onDragMove(const DragMoveEvent &e) override
    {
        if (!bypassGesture())
            rack::SliderKnob::onDragMove(e);
    }
    void onDragLeave(const DragLeaveEvent &e) override
    {
        if (!bypassGesture())
            rack::SliderKnob::onDragLeave(e);
    }
    void onHoverScroll(const HoverScrollEvent &e) override
    {
        if (!bypassGesture())
            rack::SliderKnob::onHoverScroll(e);
    }
    void onLeave(const LeaveEvent &e) override
    {
        if (!bypassGesture())
            rack::SliderKnob::onLeave(e);
    }

    void appendContextMenu(rack::Menu *menu) override
    {
        auto spq = dynamic_cast<modules::SurgeParameterModulationQuantity *>(getParamQuantity());
        if (spq)
        {
            if (menu->children.empty())
                return;

            auto firstItem = menu->children.front();
            menu->removeChild(firstItem);
            delete firstItem;

            auto spql = new SQPParamLabel;
            spql->spq = spq;
            menu->addChildBottom(spql);
        }
    }
};

inline void VerticalSlider::onChange(const rack::widget::Widget::ChangeEvent &e)
{
    bdw->dirty = true;
    bdwLight->dirty = true;
    for (auto *m : modSliders)
    {
        m->bdw->dirty = true;
    }

    positionHandleByQuantity();

    baseFB->setDirty(true);
    handleFB->setDirty(true);

    SliderKnob::onChange(e);
}

struct OutputDecoration : rack::Widget, style::StyleParticipant
{
    BufferedDrawFunctionWidget *bdw{nullptr};

    void setup()
    {
        if (!bdw)
        {
            bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                 [this](auto v) { drawRegion(v); });
            addChild(bdw);
        }
    }
    void drawRegion(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 2.5);
        nvgFillPaint(vg, nvgLinearGradient(vg, 0, 0, 0, box.size.y,
                                           style()->getColor(style::XTStyle::OUTPUTBG_START),
                                           style()->getColor(style::XTStyle::OUTPUTBG_END)));

        nvgFill(vg);
    }
    void onStyleChanged() override
    {
        if (bdw)
            bdw->dirty = true;
    }
};

struct CurveSwitch : rack::Switch, style::StyleParticipant
{
    enum Direction
    {
        ATTACK,
        HALF_DECAY,
        HALF_RELEASE,
        FULL_RELEASE
    } drawDirection{ATTACK};

    std::pair<rack::Vec, rack::Vec> endpoints()
    {
        switch (drawDirection)
        {
        case ATTACK:
            return {{0, box.size.y}, {box.size.x, 0}};
        case HALF_DECAY:
            return {{0, 0}, {box.size.x, box.size.y * 0.5f}};
        case HALF_RELEASE:
            return {{0, box.size.y * 0.5f}, {box.size.x, box.size.y}};
        case FULL_RELEASE:
            return {{0, 0}, {box.size.x, box.size.y}};
        }
        return {{}, {}};
    }

    void draw(const DrawArgs &args) override
    {
        auto vg = args.vg;

        auto [start, end] = endpoints();

        nvgBeginPath(vg);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
        nvgStrokeWidth(vg, 0.75);
        nvgMoveTo(vg, start.x, start.y);
        nvgLineTo(vg, end.x, end.y);
        nvgStroke(vg);

        auto val = 1;
        if (getParamQuantity())
            val = (int)std::round(getParamQuantity()->getValue());

        switch (val)
        {
        case 0:
        case 2:
        { // faster - want the y smaller
            nvgBeginPath(vg);
            nvgMoveTo(vg, start.x, start.y);
            auto dy = end.y - start.y;
            for (int i = 1; i < box.size.x; ++i)
            {
                float xn = i / box.size.x;
                auto a = xn;
                if ((val == 2 && dy < 0) || (val == 0 && dy > 0))
                    a = 1 - sqrt(1 - xn * xn);
                else
                    a = sqrt(1 - (xn - 1) * (xn - 1));

                float y = dy * a + start.y;
                nvgLineTo(vg, xn * box.size.x, y);
            }
            nvgLineTo(vg, end.x, end.y);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 1.25);
            nvgStroke(vg);
        }

        break;
        case 1:
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 1.25);
            nvgMoveTo(vg, start.x, start.y);
            nvgLineTo(vg, end.x, end.y);
            nvgStroke(vg);
        }
        break;
        }
        ParamWidget::draw(args);
    }
    void onStyleChanged() override {}
};

} // namespace sst::surgext_rack::widgets

#endif // SURGEXT_RACK_XTWIDGETS_H
