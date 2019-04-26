#include "rack.hpp"

struct BufferedDrawFunctionWidget : virtual rack::FramebufferWidget {
    typedef std::function<void(NVGcontext *)> drawfn_t;
    drawfn_t drawf;

    struct InternalBDW : rack::TransparentWidget {
        drawfn_t drawf;
        InternalBDW(rack::Rect box_, drawfn_t draw_) : drawf(draw_) { box = box_; }
        void draw(const DrawArgs &args) override { drawf(args.vg); }
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
