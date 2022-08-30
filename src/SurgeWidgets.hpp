#pragma once
#include <math.h>

#include "SurgeModuleCommon.hpp"
#include "SurgeStyle.hpp"

#if MAC
#include <execinfo.h>
#endif

namespace logger = rack::logger;
using rack::appGet;
using rack::logger::log;

void stackToInfo();

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

struct SurgeRackBG : public rack::TransparentWidget
{
    std::string displayName;
    std::function<void(NVGcontext *)> moduleSpecificDraw;

    SurgeRackBG(rack::Vec pos, rack::Vec size, std::string _displayName) : displayName(_displayName)
    {
        box.size = size;
        moduleSpecificDraw = [](NVGcontext *) {};
        BufferedDrawFunctionWidget *bdw =
            new BufferedDrawFunctionWidget(pos, size, [this](NVGcontext *vg) { this->drawBG(vg); });
        addChild(bdw);
    }

    bool narrowMode = false;

    void drawBG(NVGcontext *vg)
    {
        SurgeStyle::drawPanelBackground(vg, box.size.x, box.size.y, displayName, narrowMode);
        moduleSpecificDraw(vg);
    }
};

struct TextDisplayLight : public rack::widget::Widget, SurgeStyle::StyleListener
{
    typedef std::function<std::string()> stringGetter_t;
    typedef std::function<bool()> stringDirtyGetter_t;

    stringGetter_t getfn;
    stringDirtyGetter_t dirtyfn;
    int align;
    int fontsize;
    NVGcolor color;

    TextDisplayLight() : Widget() { SurgeStyle::addStyleListener(this); }
    ~TextDisplayLight() { SurgeStyle::removeStyleListener(this); }

    void setup()
    {
        addChild(new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                [this](NVGcontext *vg) { this->drawChars(vg); }));
    }

    void step() override
    {
        if (dirtyfn())
        {
            for (auto w : children)
            {
                if (auto fw = dynamic_cast<rack::FramebufferWidget *>(w))
                {
                    fw->dirty = true;
                }
            }
        }
        rack::widget::Widget::step();
    }

    virtual void styleHasChanged() override
    {
        if (colorKey != "")
            color = SurgeStyle::getColorFromMap(colorKey);

        if (hasColorLambda)
            color = colorLambda();

        for (auto w : children)
        {
            if (auto fw = dynamic_cast<rack::FramebufferWidget *>(w))
            {
                fw->dirty = true;
            }
        }
    }

    std::string font = SurgeStyle::fontFace();
    int fontId = -1;
    std::string colorKey;
    bool hasColorLambda = false;
    std::function<NVGcolor()> colorLambda;

    static TextDisplayLight *create(rack::Vec pos, rack::Vec size, stringGetter_t gf,
                                    stringDirtyGetter_t dgf, int fsize = 15,
                                    int align = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                                    std::string colorKey = SurgeStyle::parameterNameText_KEY())
    {
        TextDisplayLight *res = rack::createWidget<TextDisplayLight>(pos);
        res->getfn = gf;
        res->dirtyfn = dgf;
        res->box.pos = pos;
        res->box.size = size;
        res->fontsize = fsize;
        res->align = align;
        res->colorKey = colorKey;
        res->color = SurgeStyle::getColorFromMap(colorKey);

        res->setup();

        return res;
    }

    static TextDisplayLight *create(rack::Vec pos, rack::Vec size, const StringCache *sc,
                                    int fsize = 15, int align = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                                    std::string colorKey = SurgeStyle::parameterNameText_KEY())
    {
        if (sc)
            return TextDisplayLight::create(pos, size, sc->getValue, sc->getDirty, fsize, align,
                                            colorKey);
        else
            return TextDisplayLight::create(
                pos, size, []() { return "null"; }, []() { return false; }, fsize, align, colorKey);
    }

    void drawChars(NVGcontext *vg)
    {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, font);

        std::string ch = getfn();

        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, fontsize);
        nvgFillColor(vg, color);
        nvgTextAlign(vg, align);

        float xp = 1, yp = 1;
        if (align & NVG_ALIGN_BOTTOM)
            yp = box.size.y - 1;
        if (align & NVG_ALIGN_MIDDLE)
            yp = box.size.y / 2;

        if (align & NVG_ALIGN_RIGHT)
            xp = box.size.x - 1;
        if (align & NVG_ALIGN_CENTER)
            xp = box.size.x / 2;

        nvgText(vg, xp, yp, ch.c_str(), NULL);
    }
};

struct SurgeSmallKnob : rack::RoundKnob, SurgeStyle::StyleListener
{
    SurgeSmallKnob()
    {
        SurgeStyle::addStyleListener(this);
        setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobBG"))));
        overlay = new rack::widget::SvgWidget;
        fb->addChild(overlay);
        overlay->setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobOverlay"))));
        twfg = new rack::widget::TransformWidget;
        twfg->box.size = sw->box.size;
        fb->addChild(twfg);
        fg = new rack::widget::SvgWidget;
        fg->setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobFG"))));
        twfg->addChild(fg);
    }
    ~SurgeSmallKnob() { SurgeStyle::removeStyleListener(this); }
    void styleHasChanged() override
    {
        setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobBG"))));
        overlay->setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobOverlay"))));
        fg->setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobFG"))));
    }
    rack::widget::SvgWidget *overlay;
    rack::widget::TransformWidget *twfg;
    rack::widget::SvgWidget *fg;
    virtual void onChange(const rack::event::Change &e) override
    {
        rack::RoundKnob::onChange(e);
        if (getParamQuantity())
        {
            for (auto i = 0; i < 6; ++i)
                twfg->transform[i] = tw->transform[i];
        }
    }
};

struct SurgeKnobRooster : rack::RoundKnob, SurgeStyle::StyleListener
{
    SurgeKnobRooster()
    {
        SurgeStyle::addStyleListener(this);
        setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobRoosterFG"))));

        underlay = new rack::widget::SvgWidget;
        fb->removeChild(shadow);
        fb->addChildBottom(underlay);
        fb->addChildBottom(shadow);

        underlay->setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobRoosterBG"))));

        shadow->box.size = rack::Vec(24, 24);
        shadow->box.pos = rack::Vec(5, 9.5);
    }
    ~SurgeKnobRooster() { SurgeStyle::removeStyleListener(this); }
    void styleHasChanged() override
    {
        setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobRoosterFG"))));
        underlay->setSvg(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, SurgeStyle::getAssetPath("surgeKnobRoosterBG"))));

        shadow->box.size = rack::Vec(24, 24);
        shadow->box.pos = rack::Vec(5, 9.5);
    }

    rack::widget::SvgWidget *underlay;
};

struct SurgeUpdateColorSwitch : rack::app::SvgSwitch, SurgeStyle::StyleListener
{

    int lastSvgCol = 0xFF0090FF;
    void updateColor()
    {
        auto pt = SurgeStyle::switchHandle();
        int svgcol = (255 << 24) + (((int)(pt.b * 255)) << 16) + (((int)(pt.g * 255)) << 8) +
                     (int)(pt.r * 255);
        for (auto f : frames)
        {
            auto hn = f->handle;
            for (auto s = hn->shapes; s; s = s->next)
            {
                if (s->fill.type == NSVG_PAINT_COLOR && s->fill.color == lastSvgCol)
                {
                    s->fill.color = svgcol;
                }
            }
        }
        lastSvgCol = svgcol;
    }
    virtual void styleHasChanged() override
    {
        updateColor();
        fb->dirty = true;
    }

    virtual void resetFrames() = 0;
};

struct SurgeSwitch : SurgeUpdateColorSwitch
{
    SurgeSwitch()
    {
        SurgeStyle::addStyleListener(this);
        resetFrames();
        updateColor();
    }

    ~SurgeSwitch() { SurgeStyle::removeStyleListener(this); }

    virtual void resetFrames() override
    {
        frames.clear();
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitch_0.svg")));
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitch_1.svg")));
    }
};

struct SurgeDisableStateSwitch : SurgeUpdateColorSwitch
{
    SurgeDisableStateSwitch()
    {
        SurgeStyle::addStyleListener(this);
        resetFrames();
        updateColor();
    }

    ~SurgeDisableStateSwitch() { SurgeStyle::removeStyleListener(this); }

    virtual void onDragStart(const rack::event::DragStart &e) override
    {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        if (getParamQuantity())
        {
            if (getParamQuantity()->getValue() < 0) // disabled
                return;
            int ov, nv;
            if (getParamQuantity()->getValue() > 0.5) // basically 1
            {
                ov = 1;
                nv = 0;
                getParamQuantity()->setValue(0);
            }
            else
            {
                ov = 0;
                nv = 1;
                getParamQuantity()->setValue(1);
            }

            // Push ParamChange history action
            rack::history::ParamChange *h = new rack::history::ParamChange;
            h->name = "move switch";
            h->moduleId = getParamQuantity()->module->id;
            h->paramId = getParamQuantity()->paramId;
            h->oldValue = ov;
            h->newValue = nv;
            APP->history->push(h);

            return;
        }
        SurgeUpdateColorSwitch::onDragStart(e);
    }

    virtual void resetFrames() override
    {
        frames.clear();
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitch_dis.svg")));
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitch_0.svg")));
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitch_1.svg")));
    }
};

struct SurgeSwitchFull : SurgeUpdateColorSwitch
{
    SurgeSwitchFull()
    {
        SurgeStyle::addStyleListener(this);
        resetFrames();
        updateColor();
    }
    ~SurgeSwitchFull() { SurgeStyle::removeStyleListener(this); }

    virtual void resetFrames() override
    {
        frames.clear();
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitchFull_0.svg")));
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeSwitchFull_1.svg")));
    }
};

struct SurgeThreeSwitch : SurgeUpdateColorSwitch
{
    SurgeThreeSwitch()
    {
        SurgeStyle::addStyleListener(this);
        resetFrames();
        updateColor();
    }

    ~SurgeThreeSwitch() { SurgeStyle::removeStyleListener(this); }

    virtual void resetFrames() override
    {
        frames.clear();
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeThree_0.svg")));
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeThree_1.svg")));
        addFrame(APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/SurgeThree_2.svg")));
    }
};

struct SurgeButtonBank : public virtual rack::app::ParamWidget
{
    int rows, cols, fontSize;
    BufferedDrawFunctionWidget *bdw = nullptr;

    SurgeButtonBank() : rack::app::ParamWidget() {}

    // FIXME : override step in case I am changed from behind

    std::vector<std::string> cellLabels;
    float normalizeTo = 0.0;
    void addLabel(std::string label) { cellLabels.push_back(label); }

    int getSelectedCell()
    {
        float pv = getPValue();
        if (normalizeTo != 0)
            pv *= normalizeTo;
        return (int)pv;
    }

    float getPValue()
    {
        if (getParamQuantity())
            return getParamQuantity()->getValue();
        return -2;
    }

    void setPValue(float v)
    {
        float apply = v;
        if (normalizeTo != 0)
            apply = v / normalizeTo;
        if (getParamQuantity())
            getParamQuantity()->setValue(apply);
    }

    int fontId = -1;
    void drawWidget(NVGcontext *vg);
    void drawSelectedButton(NVGcontext *vg, float x, float y, float w, float h, std::string label);
    void drawUnselectedButton(NVGcontext *vg, float x, float y, float w, float h,
                              std::string label);
    void buttonPressedAt(float x, float y)
    {
        auto bw = box.size.x / rows;
        auto bh = box.size.y / cols;
        int r = (int)(x / bw);
        int c = (int)(y / bh);
        int cell = r + c * rows;

        setPValue(cell);

        if (bdw)
            bdw->dirty = true;
    }

    void onButton(const rack::event::Button &e) override
    {
        if (e.action == GLFW_PRESS)
            buttonPressedAt(e.pos.x, e.pos.y);
    }

    void onChange(const rack::event::Change &e) override
    {
        if (bdw)
            bdw->dirty = true;
    }

    static SurgeButtonBank *create(rack::Vec pos, rack::Vec size, SurgeModuleCommon *module,
                                   int paramId, int rows, int cols, int fontSize = 14)
    {
        SurgeButtonBank *res = rack::createWidget<SurgeButtonBank>(pos);
        res->box.pos = pos;
        res->box.size = size;
        res->rows = rows;
        res->cols = cols;
        res->fontSize = fontSize;
        res->module = module;
        res->paramId = paramId;

        res->bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                  [res](NVGcontext *vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);

        return res;
    }
};

struct SurgeModulatableRing : rack::app::Knob
{
    BufferedDrawFunctionWidget *bdw{nullptr};
    rack::app::Knob *underlyerParamWidget{nullptr};

    int modIndex{0};

    SurgeModulatableRing() { box.size = rack::Vec(45, 45); }
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

        auto ox = std::sin(angle) * w * 0.45 + w / 2;
        auto oy = h - (std::cos(angle) * h * 0.45 + h / 2);

        nvgBeginPath(vg);
        nvgArc(vg, w / 2, h / 2, w * 0.45, angle - M_PI_2, angle - modAngle - M_PI_2,
               -modAngle < 0 ? NVG_CCW : NVG_CW);
        nvgStrokeWidth(vg, 2);
        nvgStrokeColor(vg, nvgRGB(0, 200, 0));
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgArc(vg, w / 2, h / 2, w * 0.45, angle - M_PI_2, angle + modAngle - M_PI_2,
               modAngle < 0 ? NVG_CCW : NVG_CW);
        nvgStrokeWidth(vg, 4);
        nvgStrokeColor(vg, nvgRGB(0, 255, 0));
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgEllipse(vg, ox, oy, 5, 5);
        nvgFillColor(vg, nvgRGB(255, 0, 0));
        nvgFill(vg);
    }


    static SurgeModulatableRing *create(rack::Vec pos, SurgeModuleCommon *module, int paramId)
    {
        return create(pos, 50, module, paramId);
    }
    static SurgeModulatableRing *create(rack::Vec pos, int radius, SurgeModuleCommon *module, int paramId)
    {
        auto *res = rack::createWidget<SurgeModulatableRing>(pos);
        res->box.size.x = radius;
        res->box.size.y = radius;
        res->box.pos = pos;
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

struct SurgeUIOnlyToggleButton : rack::widget::Widget
{
    BufferedDrawFunctionWidget *bdw = nullptr;
    bool pressedState{false};
    bool isHovered{false};
    std::function<void(bool)> onToggle = [](bool isOn) {};

    SurgeUIOnlyToggleButton()
    {
        box.size = rack::Vec(20, 20);
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
};