#include "rack.hpp"

#ifndef RACK_V1
#include "widgets.hpp"
#endif


struct BufferedDrawFunctionWidget : virtual rack::FramebufferWidget {
    typedef std::function<void(NVGcontext *)> drawfn_t;
    drawfn_t drawf;

    struct InternalBDW : rack::TransparentWidget {
        drawfn_t drawf;
        InternalBDW(rack::Rect box_, drawfn_t draw_) : drawf(draw_) { box = box_; }
#if RACK_V1        
        void draw(const DrawArgs &args) override { drawf(args.vg); }
#else
        void draw(NVGcontext *vg) override { drawf(vg); }
#endif        
    };

    BufferedDrawFunctionWidget(rack::Vec pos, rack::Vec sz, drawfn_t draw_) : drawf(draw_) {
        box.pos = pos;
        box.size = sz;
        auto kidBox = rack::Rect(rack::Vec(0, 0), box.size);
        InternalBDW *kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
    }
};

struct SurgeRackBG : public rack::TransparentWidget
{
    std::string displayName;
    SurgeRackBG(rack::Vec pos, rack::Vec size, std::string _displayName) : displayName(_displayName) {
        box.size = size;
        BufferedDrawFunctionWidget *bdw = new BufferedDrawFunctionWidget(pos, size, [this](NVGcontext *vg) { this->drawBG(vg); } );
        addChild(bdw);
    }

    void drawBG(NVGcontext *vg) {
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, nvgRGBA(200,130,130,255));
        nvgFill(vg);
    }
};

#if RACK_V1
struct TextDisplayLight : public rack::widget::Widget
#else
struct TextDisplayLight : public rack::Widget
#endif
{
    typedef std::function<std::string()> stringGetter_t;
    typedef std::function<bool()> stringDirtyGetter_t;

    stringGetter_t getfn;
    stringDirtyGetter_t dirtyfn;
    
    TextDisplayLight() : Widget() {}

    void setup() {
        addChild(new BufferedDrawFunctionWidget(rack::Vec(0,0), box.size,
                                                [this](NVGcontext *vg) { this->drawChars(vg); } ) );
    }

#if RACK_V1
    void step() override {
        if(dirtyfn())
        {
            for(auto w : children)
            {
                if(auto fw = dynamic_cast<rack::FramebufferWidget *>(w))
                {
                    fw->dirty = true;
                }
            }
        }
        rack::widget::Widget::step();
    }
#endif    
    
    static TextDisplayLight *create(rack::Vec pos, rack::Vec size, stringDirtyGetter_t dgf, stringGetter_t gf) {
        TextDisplayLight *res = rack::createWidget<TextDisplayLight>(pos);
        res->getfn = gf;
        res->dirtyfn = dgf;
        res->box.pos = pos;
        res->box.size = size;

        res->setup();
        
        return res;
    }

    void drawChars(NVGcontext *vg) {
        std::string ch = getfn();

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, nvgRGBA(20, 20, 25 ,255));
        nvgFill(vg);

        nvgFillColor( vg, nvgRGBA( 100, 100, 255, 255 ) );
        nvgFontSize( vg, 15 );
        nvgTextAlign( vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
        nvgText( vg, 2, 2, ch.c_str(), NULL );

    }
};
