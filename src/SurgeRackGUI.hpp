#include "Surge.hpp"
#include "rack.hpp"
#include <map>
#include <functional>

#ifndef RACK_V1
#include "widgets.hpp"
#endif

/*
** Matches dave's SurgeVCV structure
*/
struct SurgeStyle {
    static NVGcolor surgeBlue() { return nvgRGBA(18, 52, 99, 255); }
    static NVGcolor surgeWhite() { return nvgRGBA(255, 255, 255, 255); }
    static NVGcolor surgeOrange() { return nvgRGBA(255, 144, 0, 255); }
    static NVGcolor color2() { return nvgRGBA(27, 28, 32, 255); }
    static NVGcolor color4() { return nvgRGBA(255, 255, 255, 255); }
    static NVGcolor surgeOrange2() { return nvgRGBA(101, 50, 3, 255); }
    static NVGcolor surgeOrange3() { return nvgRGBA(227, 112, 8, 255); }
    static NVGcolor gradient2Color() { return nvgRGBA(12, 12, 12, 255); }
    static NVGcolor gradient2Color3() { return nvgRGBA(29, 29, 29, 255); }
    static NVGcolor gradient2Color5() { return nvgRGBA(23, 23, 23, 255); }
    static NVGcolor color() { return nvgRGBA(75, 81, 93, 255); }
    static NVGcolor color5() { return nvgRGBA(0, 133, 230, 255); }
    static NVGcolor color6() { return nvgRGBA(145, 145, 145, 255); }
    static NVGcolor fillColor() { return nvgRGBA(255, 255, 255, 255); }
    static NVGcolor color7() { return nvgRGBA(205, 206, 212, 255); }
    static NVGcolor color9() { return nvgRGBA(156, 157, 160, 255); }

    static const char *fontFace() {
        return "res/EncodeSansSemiCondensed-Medium.ttf";
    }
};

// Font dictionary
struct InternalFontMgr {
    static std::map<std::string, int> fontMap;
    static int get(NVGcontext *vg, std::string resName) {
        if (fontMap.find(resName) == fontMap.end()) {
#ifdef RACK_V1
            std::string fontPath = rack::asset::plugin(pluginInstance, resName);
#else
            std::string fontPath = rack::assetPlugin(pluginInstance, resName);
#endif
            INFO("LOADING FROM %s", fontPath.c_str());
            fontMap[resName] =
                nvgCreateFont(vg, resName.c_str(), fontPath.c_str());
            INFO("LOADING FROM %s -> %d", fontPath.c_str(), fontMap[resName]);
#ifndef RACK_V1
            // FIXME
            if (fontMap[resName] < 0)
                fontMap[resName] = 1;
#endif
        }
        return fontMap[resName];
    }
};

#ifndef RACK_V1
using rack::INFO_LEVEL;
#endif

struct SurgeSmallKnob : rack::RoundKnob {
    SurgeSmallKnob() {
#if RACK_V1
        setSvg(APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/smallKnob.svg")));
#else
        setSVG(rack::SVG::load(
            rack::assetPlugin(pluginInstance, "res/vectors/smallKnob.svg")));
#endif
    }
};

struct BufferedDrawFunctionWidget : virtual rack::FramebufferWidget {
    typedef std::function<void(NVGcontext *)> drawfn_t;
    drawfn_t drawf;

    struct InternalBDW : rack::TransparentWidget {
        drawfn_t drawf;
        InternalBDW(rack::Rect box_, drawfn_t draw_) : drawf(draw_) {
            box = box_;
        }
#if RACK_V1
        void draw(const DrawArgs &args) override { drawf(args.vg); }
#else
        void draw(NVGcontext *vg) override { drawf(vg); }
#endif
    };

    BufferedDrawFunctionWidget(rack::Vec pos, rack::Vec sz, drawfn_t draw_)
        : drawf(draw_) {
        box.pos = pos;
        box.size = sz;
        auto kidBox = rack::Rect(rack::Vec(0, 0), box.size);
        InternalBDW *kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
    }
};

struct SurgeRackBG : public rack::TransparentWidget {
    std::string displayName;
    SurgeRackBG(rack::Vec pos, rack::Vec size, std::string _displayName)
        : displayName(_displayName) {
        box.size = size;
        BufferedDrawFunctionWidget *bdw = new BufferedDrawFunctionWidget(
            pos, size, [this](NVGcontext *vg) { this->drawBG(vg); });
        addChild(bdw);
    }

    bool hasInput = false;
    bool hasOutput = true;
    int orangeLine = 323;
    int ioMargin = 7;
    int ioRegionWidth = 70;
    int fontId = -1;

    void drawBG(NVGcontext *vg) {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, SurgeStyle::color7());
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, orangeLine, box.size.x, box.size.y - orangeLine);
        nvgFillColor(vg, SurgeStyle::surgeOrange());
        nvgFill(vg);

        for (int i = 0; i < 2; ++i) {
            if ((i == 0 && hasInput) || (i == 1 && hasOutput)) {
                nvgBeginPath(vg);
                int x0 = 0;
                if (i == 1)
                    x0 = box.size.x - ioRegionWidth - 2 * ioMargin;
                nvgRoundedRect(
                    vg, x0 + ioMargin, orangeLine + ioMargin, ioRegionWidth,
                    box.size.y - orangeLine - 2 * ioMargin, ioMargin);
                nvgFillColor(vg, SurgeStyle::surgeBlue());
                nvgFill(vg);
                nvgStrokeColor(vg, SurgeStyle::color7());
                nvgStroke(vg);

                nvgFillColor(vg, SurgeStyle::color7());
                nvgFontFaceId(vg, fontId);
                nvgFontSize(vg, 12);
                nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
                nvgText(vg, x0 + ioMargin + ioRegionWidth / 2,
                        orangeLine + ioMargin, (i == 0 ? "Input" : "Output"),
                        NULL);
            }
        }
    }

    rack::Vec ioPortLocation(bool input, bool left) {
        float portX = 24.6721, portY = 24.6721;
        int x0 = 0;
        if (!input)
            x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

        int padFromEdge = (ioRegionWidth / 2 - portX) / 2;
        int xRes =
            x0 + ioMargin + padFromEdge + (left ? 0 : (ioRegionWidth / 2));
        int yRes = box.size.y - 1.5 * ioMargin - portY;

        return rack::Vec(xRes, yRes);
    }
};

#if RACK_V1
struct TextDisplayLight : public rack::widget::Widget
#else
struct TextDisplayLight : public rack::Component
#endif
{
    typedef std::function<std::string()> stringGetter_t;
    typedef std::function<bool()> stringDirtyGetter_t;

    stringGetter_t getfn;
    stringDirtyGetter_t dirtyfn;
    int align;
    int fontsize;
    NVGcolor color;

    TextDisplayLight() : Widget() {}

    void setup() {
        addChild(new BufferedDrawFunctionWidget(
            rack::Vec(0, 0), box.size,
            [this](NVGcontext *vg) { this->drawChars(vg); }));
    }

#if RACK_V1
    void step() override {
        if (dirtyfn()) {
            for (auto w : children) {
                if (auto fw = dynamic_cast<rack::FramebufferWidget *>(w)) {
                    fw->dirty = true;
                }
            }
        }
        rack::widget::Widget::step();
    }
#else
    void draw(NVGcontext *vg) override {
        if (dirtyfn()) {
            for (auto w : children) {
                if (auto fw = dynamic_cast<rack::FramebufferWidget *>(w)) {
                    fw->dirty = true;
                }
            }
        }
        rack::Component::draw(vg);
    }
#endif

    int fontId = -1;

    static TextDisplayLight *
    create(rack::Vec pos, rack::Vec size, stringGetter_t gf,
           stringDirtyGetter_t dgf, int fsize = 15,
           int align = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
           NVGcolor color = nvgRGBA(255, 144, 0, 255) 
        ) {
        TextDisplayLight *res = rack::createWidget<TextDisplayLight>(pos);
        res->getfn = gf;
        res->dirtyfn = dgf;
        res->box.pos = pos;
        res->box.size = size;
        res->fontsize = fsize;
        res->align = align;
        res->color = color;

        res->setup();

        return res;
    }

    void drawChars(NVGcontext *vg) {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

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

        nvgText(vg, xp, yp, ch.c_str(), NULL);
    }
};

#if RACK_V1
struct SurgeParamLargeWidget : public rack::widget::Widget
#else
struct SurgeParamLargeWidget : public rack::TransparentWidget
#endif
{
#if RACK_V1
    SurgeParamLargeWidget() : rack::widget::Widget() {}
#else
    SurgeParamLargeWidget() : rack::TransparentWidget() {}
#endif
    static const int height = 40;
    static const int width = 12 * SCREW_WIDTH;

    static constexpr float portX = 24.6721;
    static constexpr float portY = 24.6721;
    static const int knobX = 24;
    static const int knobY = 24;
    static const int toggleX = 13;
    static const int toggleY = 20;
    static const int itemMargin = 3;

    static SurgeParamLargeWidget *
    create(rack::ModuleWidget *mw, rack::Module *module, rack::Vec pos,
           int paramID, int cvID, int extendedSwitchID,
           TextDisplayLight::stringGetter_t labelfn,
           TextDisplayLight::stringDirtyGetter_t labelDirtyFn,
           TextDisplayLight::stringGetter_t sublabelfn,
           TextDisplayLight::stringDirtyGetter_t sublabelDirtyFn,
           TextDisplayLight::stringGetter_t valuefn,
           TextDisplayLight::stringDirtyGetter_t valueDirtyFn) {
        SurgeParamLargeWidget *res = new SurgeParamLargeWidget();
        res->box.pos = pos;
        res->box.size = rack::Vec(width, height);

        res->addChild(new BufferedDrawFunctionWidget(
            rack::Vec(0, 0), res->box.size,
            [res](NVGcontext *vg) { res->drawBG(vg); }));
        int text0 = portX + knobX + toggleX + 4 * itemMargin;

        res->addChild(TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2), rack::Vec(res->box.size.x - text0 - 6, 14),
                          labelfn, labelDirtyFn, 16));
        res->addChild(TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2),
                          rack::Vec(res->box.size.x - text0 - 6, 143),
                          sublabelfn, sublabelDirtyFn, 10, NVG_ALIGN_TOP | NVG_ALIGN_RIGHT));
        res->addChild(TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2),
                          rack::Vec(res->box.size.x - text0 - 4, res->box.size.y - 4),
                          valuefn, valueDirtyFn, 18, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT,
                          SurgeStyle::surgeWhite()));

        mw->addInput(rack::createInput<rack::PJ301MPort>(
                         rack::Vec(res->box.pos.x + itemMargin,
                                   res->box.pos.y + res->box.size.y / 2 - portX / 2),
            module, cvID));

        mw->addParam(rack::createParam<rack::CKSS>(
                         rack::Vec(res->box.pos.x + 2 * itemMargin + portX,
                                   res->box.pos.y + res->box.size.y / 2 - toggleY / 2), module,
                         extendedSwitchID
#ifndef RACK_V1
                         ,
                         0, 1, 0
#endif
                         ));
        
        mw->addParam(rack::createParam<SurgeSmallKnob>(
                         rack::Vec(res->box.pos.x + 3 * itemMargin + portX + toggleX,
                                   res->box.pos.y + res->box.size.y / 2 - knobY/2 ), module,
                         paramID
#ifndef RACK_V1
                         ,
                         0, 1, 0.5
#endif
                         ));

        return res;
    }

    void drawBG(NVGcontext *vg) {
        int text0 = portX + knobX + toggleX + 4 * itemMargin;

        nvgBeginPath(vg);
        nvgRoundedRect(vg, text0, 0, box.size.x - text0, box.size.y, 5);
        nvgFillColor(vg, SurgeStyle::color2());
        nvgFill(vg);
        nvgStrokeColor(vg, SurgeStyle::surgeOrange());
        nvgStroke(vg);
    }
};


struct SurgeRoundedRect : public rack::TransparentWidget
{
    SurgeRoundedRect(rack::Vec pos, rack::Vec size) : TransparentWidget() {
        box.pos = pos;
        box.size = size;
        addChild(new BufferedDrawFunctionWidget(rack::Vec(0,0), size,
                                                [this](NVGcontext *vg)
                                                {
                                                    this->drawRR(vg);
                                                }
                     ));
    }

    void drawRR(NVGcontext *vg) {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5);
        nvgFillColor(vg, SurgeStyle::color2());
        nvgFill(vg);
        nvgStrokeColor(vg, SurgeStyle::surgeOrange());
        nvgStroke(vg);
    }
};
