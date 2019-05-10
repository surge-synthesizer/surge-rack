#include "SurgeStyle.hpp"
#include "rack.hpp"
#ifndef RACK_V1
#include "widgets.hpp"
#endif
#ifdef RACK_V1
#include "svg.hpp"
#endif

/*
** These are purposefully not exposed
*/
namespace SurgeInternal {
#ifdef RACK_V1
typedef std::shared_ptr<rack::Svg> svg_t;
#else
typedef std::shared_ptr<rack::SVG> svg_t;
#endif
static svg_t surgeLogoBlue = nullptr, surgeLogoWhite = nullptr;
static svg_t getSurgeLogo(bool whiteVersion) {
    if (surgeLogoBlue == nullptr) {
#if RACK_V1
        surgeLogoBlue = rack::APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/SurgeLogoOnlyBlue.svg"));
        surgeLogoWhite = rack::APP->window->loadSvg(
            rack::asset::plugin(pluginInstance, "res/SurgeLogoOnlyWhite.svg"));
#else
        surgeLogoBlue = rack::SVG::load(
            rack::assetPlugin(pluginInstance, "res/SurgeLogoOnlyBlue.svg"));
        surgeLogoWhite = rack::SVG::load(
            rack::assetPlugin(pluginInstance, "res/SurgeLogoOnlyWhite.svg"));
#endif
    }
    if (whiteVersion)
        return surgeLogoWhite;
    return surgeLogoBlue;
}
} // namespace SurgeInternal

void SurgeStyle::drawBlueIORect(NVGcontext *vg, float x0, float y0, float w,
                                float h, int direction) {
    nvgBeginPath(vg);
    NVGpaint vGradient;
    vGradient =
        nvgLinearGradient(vg, x0, y0, x0, y0 + h, SurgeStyle::surgeBlueBright(),
                          SurgeStyle::surgeBlue());

    nvgRoundedRect(vg, x0, y0, w, h, 5);
    nvgFillPaint(vg, vGradient);
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0 + 1, w, h - 1, 5);
    nvgStrokeColor(vg, SurgeStyle::surgeBlueVeryBright());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    nvgStrokeColor(vg, SurgeStyle::surgeBlueDark());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
    nvgBeginPath(vg);
}

void SurgeStyle::drawTextBGRect(NVGcontext *vg, float x0, float y0, float w,
                                float h) {
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    NVGpaint gradient =
        nvgLinearGradient(vg, x0, y0, x0, y0 + h, SurgeStyle::textBGBright(),
                          SurgeStyle::textBG());
    nvgFillPaint(vg, gradient);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h - 1, 5);
    nvgStrokeColor(vg, SurgeStyle::textBGVeryBright());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    nvgStrokeColor(vg, SurgeStyle::surgeOrange());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
}

void SurgeStyle::drawPanelBackground(NVGcontext *vg, float w, float h,
                                     std::string displayName, bool narrowMode) {
    int orangeLine = SurgeLayout::orangeLine;
    int fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, w, h);
    nvgFillColor(vg, SurgeStyle::backgroundGray());
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, 0, h );
    nvgLineTo(vg, 0, 0 );
    nvgLineTo(vg, w-1, 0 );
    nvgLineTo(vg, w-1, h);
    nvgStrokeColor(vg, SurgeStyle::backgroundDarkGray());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, orangeLine, w, h - orangeLine);
    nvgFillColor(vg, SurgeStyle::surgeOrange());
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, w-1, orangeLine);
    nvgLineTo(vg, w-1, h-1);
    nvgLineTo(vg, 0, h-1 );
    nvgLineTo(vg, 0, orangeLine );
    nvgStrokeColor(vg, SurgeStyle::surgeOrangeMedium() );
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, 0, orangeLine);
    nvgLineTo(vg, w, orangeLine);
    nvgStrokeColor(vg, SurgeStyle::surgeBlue());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    auto logoSvg = SurgeInternal::getSurgeLogo(false);
    float logoX0 = w / 2;
    
    if (narrowMode)
    {
        logoX0 = 2;
    }
    
    float logoWidth = 10;
    if (logoSvg && logoSvg->handle) {
        // We want the logo to be screw width - 4  high
        float scaleFactor = 1.0 * (SCREW_WIDTH - 4) / logoSvg->handle->height;
        float x0 = logoX0 - (narrowMode ? 0 : logoSvg->handle->width * scaleFactor / 2 );
        logoWidth = logoSvg->handle->width * scaleFactor;
        nvgSave(vg);
        nvgTranslate(vg, x0, 2);
        nvgScale(vg, scaleFactor, scaleFactor);
#if RACK_V1
        rack::svgDraw(vg, logoSvg->handle);
#else
        // This is so unsatisfying but v6 doesn't expose svgDraw as a public API
        // point so fake it with a temp widget
        rack::SVGWidget w;
        w.setSVG(logoSvg);
        w.draw(vg);
#endif
        nvgRestore(vg);
    }
        
    if( ! narrowMode )
    {
        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 14);
        nvgFillColor(vg, SurgeStyle::surgeBlue());
        nvgText(vg, logoX0 - logoWidth / 2 - 3, 0, "Surge", NULL);
    }
    
    nvgBeginPath(vg);
    nvgTextAlign(vg, (narrowMode ? NVG_ALIGN_RIGHT | NVG_ALIGN_TOP : NVG_ALIGN_LEFT | NVG_ALIGN_TOP ) );
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 14);
    nvgFillColor(vg, SurgeStyle::surgeBlue());
    nvgText(vg, (narrowMode ? w - 2 : logoX0 + logoWidth / 2 + 3 ), 0, displayName.c_str(), NULL);
}
