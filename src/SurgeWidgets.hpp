#pragma once
#include <math.h>

#include "SurgeStyle.hpp"
#include "SurgeModuleCommon.hpp"

#if MAC
#include <execinfo.h>
#endif

void stackToInfo();


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
        moduleSpecificDraw = [](NVGcontext *) {};
        BufferedDrawFunctionWidget *bdw = new BufferedDrawFunctionWidget(
            pos, size, [this](NVGcontext *vg) { this->drawBG(vg); });
        addChild(bdw);
    }

    bool narrowMode = false;
    
    void drawBG(NVGcontext *vg) {
        SurgeStyle::drawPanelBackground(vg, box.size.x, box.size.y,
                                        displayName, narrowMode);
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
           NVGcolor color = nvgRGBA(255, 144, 0, 255)) {
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

    static TextDisplayLight *
    create(rack::Vec pos, rack::Vec size, 
           const StringCache *sc,
           int fsize = 15,
           int align = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
           NVGcolor color = nvgRGBA(255, 144, 0, 255)) {
        if( sc )
            return TextDisplayLight::create(pos, size, sc->getValue, sc->getDirty,
                                            fsize, align, color);
        else
            return TextDisplayLight::create(pos, size,
                                            []() { return "null"; },
                                            []() { return false; },
                                            fsize, align, color);
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
        if (align & NVG_ALIGN_CENTER)
            xp = box.size.x / 2;

        nvgText(vg, xp, yp, ch.c_str(), NULL);
    }
};

struct SurgeSmallKnob : rack::RoundKnob {
    SurgeSmallKnob() {
#if RACK_V1
        setSvg(rack::APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/vectors/surgeKnob.svg")));
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
        setSVG(rack::SVG::load(rack::assetPlugin(
            pluginInstance, "res/vectors/surgeKnob_34x34.svg")));
#endif
    }
};

struct SurgeKnobRooster : rack::RoundKnob {
    SurgeKnobRooster() {
#if RACK_V1
        setSvg(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/surgeKnobRooster.svg")));
#else
        setSVG(rack::SVG::load(rack::assetPlugin(
            pluginInstance, "res/vectors/surgeKnobRooster.svg")));
#endif
        shadow->box.size = rack::Vec(24, 24);
        shadow->box.pos = rack::Vec(5, 9.5);
    }
};

struct SurgeSwitch :
#if RACK_V1
    rack::app::SvgSwitch
#else
    rack::SVGSwitch,
    rack::ToggleSwitch
#endif
{
    SurgeSwitch() {
#if RACK_V1
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/SurgeSwitch_0.svg")));
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/SurgeSwitch_1.svg")));
#else
        addFrame(rack::SVG::load(rack::assetPlugin(
            pluginInstance, "res/vectors/SurgeSwitch_0.svg")));
        addFrame(rack::SVG::load(rack::assetPlugin(
            pluginInstance, "res/vectors/SurgeSwitch_1.svg")));
#endif
    }
};

struct SurgeSwitchFull :
#if RACK_V1
    rack::app::SvgSwitch
#else
    rack::SVGSwitch,
    rack::ToggleSwitch
#endif
{
    SurgeSwitchFull() {
#if RACK_V1
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/SurgeSwitchFull_0.svg")));
        addFrame(rack::APP->window->loadSvg(rack::asset::plugin(
            pluginInstance, "res/vectors/SurgeSwitchFull_1.svg")));
#else
        addFrame(rack::SVG::load(rack::assetPlugin(
            pluginInstance, "res/vectors/SurgeSwitchFull_0.svg")));
        addFrame(rack::SVG::load(rack::assetPlugin(
            pluginInstance, "res/vectors/SurgeSwitchFull_1.svg")));
#endif
    }
};

struct SurgeButtonBank :
#if RACK_V1
    public virtual rack::app::ParamWidget
#else
    public virtual rack::ParamWidget
#endif
{
    int rows, cols, fontSize;
    BufferedDrawFunctionWidget *bdw = nullptr;
    
    SurgeButtonBank() :
#if RACK_V1
        rack::app::ParamWidget()
#else
        rack::ParamWidget()
#endif
        {
        }


    //FIXME : override step in case I am changed from behind

    std::vector<std::string> cellLabels;
    float normalizeTo = 0.0;
    void addLabel(std::string label) {
        cellLabels.push_back(label);
    }

    int getSelectedCell() {
        float pv = getPValue();
        if( normalizeTo != 0 )
            pv *= normalizeTo;
        return (int)pv;
    }
    
    
    float getPValue() {
#if RACK_V1
        if( paramQuantity )
            return paramQuantity->getValue();
        return -2;
#else
        if( module )
            return module->params[paramId].value;
        return -2;
#endif
    }

    void setPValue(float v) {
        float apply = v;
        if( normalizeTo != 0 )
            apply = v / normalizeTo;
#if RACK_V1        
        if( paramQuantity )
            paramQuantity->setValue(apply);
#else
        value = apply;
        if( module )
            module->params[paramId].value = apply;
#endif        

    }
    
    int fontId = -1;
    void drawWidget(NVGcontext *vg);
    void drawSelectedButton(NVGcontext *vg, float x, float y, float w, float h, std::string label);
    void drawUnselectedButton(NVGcontext *vg, float x, float y, float w, float h, std::string label);
    void buttonPressedAt(float x, float y) {
        auto bw = box.size.x / rows;
        auto bh = box.size.y / cols;
        int r = (int)( x / bw );
        int c = (int)( y / bh );
        int cell = r + c*rows;

        setPValue(cell);

        if( bdw )
            bdw->dirty = true;

    }
        
#if RACK_V1    
    void onButton(const rack::event::Button &e) override {
        if(e.action == GLFW_PRESS)
            buttonPressedAt(e.pos.x,e.pos.y);
    }

    void onChange(const rack::event::Change &e) override {
        if( bdw )
            bdw->dirty = true;
    }
#else
    virtual void onMouseDown(rack::EventMouseDown &e) override {
        buttonPressedAt(e.pos.x,e.pos.y);
    }
    
    virtual void onChange(rack::EventChange &e) override {
        if(module)
            module->params[paramId].value = value;
        if( bdw )
            bdw->dirty = true;
    }

#endif    
    
    static SurgeButtonBank* create(rack::Vec pos, rack::Vec size,
                                   SurgeModuleCommon *module, int paramId,
                                   int rows, int cols, int fontSize=14) {
        SurgeButtonBank *res = rack::createWidget<SurgeButtonBank>(pos);
        res->box.pos = pos;
        res->box.size = size;
        res->rows = rows;
        res->cols = cols;
        res->fontSize = fontSize;

        res->bdw = new BufferedDrawFunctionWidget(rack::Vec(0,0), size, [res](NVGcontext *vg) { res->drawWidget(vg); } );
        res->addChild(res->bdw);

#if RACK_V1        
        if( module )
            res->paramQuantity = module->paramQuantities[paramId];
#else
        res->module = module;
        res->paramId = paramId;
        res->setLimits(0,rows*cols-1);
        res->setDefaultValue(0);
#endif        

        return res;
    }
};
