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
    static NVGcolor surgeBlueBright() {
        return nvgRGBA(18 * 1.5, 52 * 1.5, 99 * 1.8, 255);
    }
    static NVGcolor surgeWhite() { return nvgRGBA(255, 255, 255, 255); }
    static NVGcolor surgeOrange() { return nvgRGBA(255, 144, 0, 255); }
    static NVGcolor color2() { return nvgRGBA(27, 28, 32, 255); }
    static NVGcolor color2Bright() { return nvgRGBA(40, 40, 52, 255); }
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

    static NVGcolor backgroundGray() { return nvgRGBA(205, 206, 212, 255); }
    static NVGcolor backgroundLightGray() { return nvgRGBA(215, 216, 222, 255); }
    static NVGcolor backgroundLightOrange() { return nvgRGBA(239, 210, 172, 255); }

    static const char *fontFace() {
        return "res/EncodeSansSemiCondensed-Medium.ttf";
    }
    static const char *fontFaceCondensed() {
        return "res/EncodeSansCondensed-Medium.ttf";
    }
};

struct SurgeLayout
{
    static float constexpr portX = 24.6721;
    static float constexpr portY = 24.6721;
    static float constexpr surgeKnobX = 24;
    static float constexpr surgeKnobY = 24;
    static float constexpr surgeRoosterX = 34;
    static float constexpr surgeRoosterY = 34;
    static float constexpr orangeLine = 323;
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
            fontMap[resName] = nvgCreateFont(vg, resName.c_str(), fontPath.c_str());
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
        setSvg(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/surgeKnob.svg")));
#else
        setSVG(rack::SVG::load(
            rack::assetPlugin(pluginInstance, "res/vectors/surgeKnob.svg")));
#endif
    }
};

struct SurgeKnob : rack::RoundKnob {
    SurgeKnob() {
#if RACK_V1
        setSvg(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/surgeKnob_34x34.svg")));
#else
        setSVG(rack::SVG::load(
            rack::assetPlugin(pluginInstance, "res/vectors/surgeKnob_34x34.svg")));
#endif
    }
};

struct SurgeKnobRooster : rack::RoundKnob {
    SurgeKnobRooster() {
#if RACK_V1
        setSvg(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/surgeKnobRooster.svg")));
#else
        setSVG(rack::SVG::load(
            rack::assetPlugin(pluginInstance, "res/vectors/surgeKnobRooster.svg")));
#endif
        shadow->box.size = rack::Vec(24,24);
        shadow->box.pos = rack::Vec(5,9.5);
    }
};

struct SurgeSwitch :
#if RACK_V1    
    rack::app::SvgSwitch
#else
    rack::SVGSwitch, rack::ToggleSwitch
#endif
{
    SurgeSwitch() {
#if RACK_V1        
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(pluginInstance,"res/vectors/SurgeSwitch_0.svg")));
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(pluginInstance,"res/vectors/SurgeSwitch_1.svg")));
#else
        addFrame(
            rack::SVG::load(rack::assetPlugin(pluginInstance,"res/vectors/SurgeSwitch_0.svg")));
        addFrame(
            rack::SVG::load(rack::assetPlugin(pluginInstance,"res/vectors/SurgeSwitch_1.svg")));
#endif        
    }
};

struct SurgeSwitchFull :
#if RACK_V1    
    rack::app::SvgSwitch
#else
    rack::SVGSwitch, rack::ToggleSwitch
#endif
{
    SurgeSwitchFull() {
#if RACK_V1        
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(pluginInstance,"res/vectors/SurgeSwitchFull_0.svg")));
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(pluginInstance,"res/vectors/SurgeSwitchFull_1.svg")));
#else
        addFrame(
            rack::SVG::load(rack::assetPlugin(pluginInstance,"res/vectors/SurgeSwitchFull_0.svg")));
        addFrame(
            rack::SVG::load(rack::assetPlugin(pluginInstance,"res/vectors/SurgeSwitchFull_1.svg")));
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

    InternalBDW *kid = nullptr;
    BufferedDrawFunctionWidget(rack::Vec pos, rack::Vec sz, drawfn_t draw_)
        : drawf(draw_) {
        box.pos = pos;
        box.size = sz;
        auto kidBox = rack::Rect(rack::Vec(0, 0), box.size);
        kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
    }
};


struct SurgeRackBG : public rack::TransparentWidget {
    std::string displayName;
    std::function<void(NVGcontext *)> moduleSpecificDraw;
    
    SurgeRackBG(rack::Vec pos, rack::Vec size, std::string _displayName)
        : displayName(_displayName) {
        box.size = size;
        moduleSpecificDraw = [](NVGcontext *){};
        BufferedDrawFunctionWidget *bdw = new BufferedDrawFunctionWidget(
            pos, size, [this](NVGcontext *vg) { this->drawBG(vg); });
        addChild(bdw);
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(box.size.x - SCREW_WIDTH, box.size.y - SCREW_WIDTH)));
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(0, box.size.y - SCREW_WIDTH)));
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(box.size.x - SCREW_WIDTH, 0)));
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(0, 0)));

    }

    std::string font = SurgeStyle::fontFace();
    int fontId = -1;

    void drawBG(NVGcontext *vg) {
        int orangeLine = SurgeLayout::orangeLine;
        
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, font);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, SurgeStyle::backgroundGray());
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, orangeLine, box.size.x, box.size.y - orangeLine);
        nvgFillColor(vg, SurgeStyle::surgeOrange());
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, orangeLine);
        nvgLineTo(vg, box.size.x, orangeLine);
        nvgStrokeColor(vg, SurgeStyle::surgeBlue());
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
            
        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgFillColor(vg, SurgeStyle::surgeWhite());
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 9);

        char version[1024];
        snprintf(version, 1024, "bld: %s %s", __DATE__, __TIME__);
        nvgText(vg, box.size.x/2, box.size.y - 2, version, NULL);

        snprintf(version, 1024, "%s: %s.%s.%s",
#if WINDOWS
                 "win",
#endif
#if MAC
                 "macos",
#endif
#if LINUX                 
                 "linux",
#endif                 
                 TOSTRING(SURGE_RACK_BASE_VERSION),
                 TOSTRING(SURGE_RACK_PLUG_VERSION),
                 TOSTRING(SURGE_RACK_SURGE_VERSION));
        nvgText(vg, box.size.x/2, box.size.y - 12, version, NULL);

        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgFillColor(vg, SurgeStyle::surgeOrange2());
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 30);
        nvgText(vg, box.size.x/2 + 1, box.size.y - 25 + 1, displayName.c_str(), NULL);

        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgFillColor(vg, SurgeStyle::surgeWhite());
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 30);
        nvgText(vg, box.size.x/2, box.size.y - 25, displayName.c_str(), NULL);

        moduleSpecificDraw(vg);
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

    std::string font = SurgeStyle::fontFace();
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
    static const int width = 14 * SCREW_WIDTH;

    static constexpr float portX = 24.6721;
    static constexpr float portY = 24.6721;
    static const int sknobX = 24;
    static const int sknobY = 24;
    static const int knobX = 34;
    static const int knobY = 34;
    static const int toggleX = 7;
    static const int toggleY = 20;
    static const int itemMargin = 3;

    static SurgeParamLargeWidget *
    create(rack::ModuleWidget *mw, rack::Module *module, rack::Vec pos,
           int paramID, int paramGainID, int cvID, int extendedSwitchID,
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
        int text0 = portX + knobX +sknobX + toggleX + 6 * itemMargin;

        res->addChild(TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2),
                          rack::Vec(res->box.size.x - text0 - 6, res->box.size.y),
                                    sublabelfn, sublabelDirtyFn, 8, NVG_ALIGN_TOP | NVG_ALIGN_RIGHT));

        TextDisplayLight *lt;
        res->addChild(lt = TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2), rack::Vec(res->box.size.x - text0 - 6, 14),
                          labelfn, labelDirtyFn, 14));
        lt->font = SurgeStyle::fontFaceCondensed();
        
        res->addChild(lt = TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2),
                          rack::Vec(res->box.size.x - text0 - 4, res->box.size.y - 4),
                          valuefn, valueDirtyFn, 18, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT,
                          SurgeStyle::surgeWhite()));
        lt->font = SurgeStyle::fontFaceCondensed();

        mw->addInput(rack::createInput<rack::PJ301MPort>(
                         rack::Vec(res->box.pos.x + itemMargin,
                                   res->box.pos.y + res->box.size.y / 2 - portX / 2),
            module, cvID));

        mw->addParam(rack::createParam<SurgeSmallKnob>(
                         rack::Vec(res->box.pos.x + 2 * itemMargin + portX,
                                   res->box.pos.y + res->box.size.y / 2 - sknobY / 2), module,
                         paramGainID
#ifndef RACK_V1
                         ,
                         0,1,0.5
#endif
                         ));
        
        mw->addParam(rack::createParam<SurgeSwitch>(
                         rack::Vec(res->box.pos.x + sknobX + 4 * itemMargin + portX,
                                   res->box.pos.y + res->box.size.y / 2 - toggleY / 2), module,
                         extendedSwitchID
#ifndef RACK_V1
                         ,
                         0, 1, 0
#endif
                         ));
        
        mw->addParam(rack::createParam<SurgeKnobRooster>(
                         rack::Vec(res->box.pos.x + sknobX + 5 * itemMargin + portX + toggleX,
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
        /*
          FIXME this overpaints the input port
        nvgBeginPath(vg);
        nvgRoundedRect(vg, itemMargin / 2, box.size.y / 2 - portY/2 - itemMargin / 2,
                       portX + itemMargin, portY + itemMargin, 3 );
        nvgFillColor(vg, SurgeStyle::surgeOrange());
        nvgFill(vg);
        nvgStrokeColor(vg, SurgeStyle::surgeOrange2());
        nvgStrokeWidth(vg, 2);
        nvgStroke(vg);
        */

        int text0 = portX + sknobX + knobX + toggleX + 6 * itemMargin;

        nvgBeginPath(vg);
        nvgRoundedRect(vg, text0, 0, box.size.x - text0, box.size.y, 5);
        NVGpaint gradient =
            nvgLinearGradient(vg, text0, 0, text0, box.size.y,
                              SurgeStyle::color2Bright(), SurgeStyle::color2());
        // nvgFillColor(vg, SurgeStyle::color2());
        nvgFillPaint(vg, gradient);
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
