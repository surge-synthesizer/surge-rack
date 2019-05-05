#include "Surge.hpp"
#include "rack.hpp"
#include <map>
#include <functional>
#include "SurgeStyle.hpp"
#ifndef RACK_V1
#include "widgets.hpp"
#endif

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

        // FIXME: If you are narrow enough only add one screw
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(box.size.x - SCREW_WIDTH, box.size.y - SCREW_WIDTH)));
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(0, box.size.y - SCREW_WIDTH)));
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(box.size.x - SCREW_WIDTH, 0)));
        addChild(rack::createWidget<rack::ScrewSilver>(rack::Vec(0, 0)));

    }

    void drawBG(NVGcontext *vg) {
        SurgeStyle::drawPanelBackground(vg, box.size.x, box.size.y, displayName);
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
        if( align & NVG_ALIGN_CENTER )
            xp = box.size.x / 2;
        
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
        int text0 = portX + sknobX + knobX + toggleX + 6 * itemMargin;
        SurgeStyle::drawTextBGRect(vg, text0, 0, box.size.x - text0, box.size.y);
    }
};

