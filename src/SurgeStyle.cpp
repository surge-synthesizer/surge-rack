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
namespace SurgeInternal
{
#ifdef RACK_V1
    typedef std::shared_ptr<rack::Svg> svg_t;
#else
    typedef std::shared_ptr<rack::SVG> svg_t;
#endif    
    static svg_t surgeLogo = nullptr;
    static svg_t getSurgeLogo()
    {
        if( surgeLogo == nullptr )
        {
#if RACK_V1
            surgeLogo = rack::APP->window->loadSvg(rack::asset::plugin(
                                                      pluginInstance, "res/SurgeLogoOnlyWhite.svg"));
#else
            surgeLogo = rack::SVG::load(
                rack::assetPlugin(pluginInstance, "res/SurgeLogoOnlyWhite.svg"));
#endif

        }
        return surgeLogo;
    }
}

void SurgeStyle::drawBlueIORect(NVGcontext *vg, float x0, float y0, float w, float h, int direction)
{
    INFO( "DBIOR" );
    nvgBeginPath(vg);
    NVGpaint sideGradient;
    switch(direction)
    {
    case 1:
        sideGradient = nvgLinearGradient(vg,
                                         x0, y0, x0, y0 + w,
                                         SurgeStyle::surgeBlue(),
                                         SurgeStyle::surgeBlueBright());
        break;
    case 0:
    default:
        sideGradient = nvgLinearGradient(vg,
                                         x0, y0, x0, y0 + w,
                                         SurgeStyle::surgeBlueBright(),
                                         SurgeStyle::surgeBlue());
        break;
    }
    nvgRoundedRect(vg,x0,y0,w,h, 5);
    nvgFillPaint(vg, sideGradient);
    nvgFill(vg);
    nvgStrokeColor(vg, SurgeStyle::surgeWhite());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

}

void SurgeStyle::drawTextBGRect(NVGcontext *vg, float x0, float y0, float w, float h)
{
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    NVGpaint gradient =
        nvgLinearGradient(vg, x0, y0, x0, y0 + h,
                          SurgeStyle::color2Bright(), SurgeStyle::color2());
    nvgFillPaint(vg, gradient);
    nvgFill(vg);
    nvgStrokeColor(vg, SurgeStyle::surgeOrange());
    nvgStroke(vg);
}

void SurgeStyle::drawPanelBackground(NVGcontext *vg, float w, float h, std::string displayName)
{
    INFO( "DPBG" );
    int orangeLine = SurgeLayout::orangeLine;
    int fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, w, h);
    nvgFillColor(vg, SurgeStyle::backgroundGray());
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0, orangeLine, w, h - orangeLine);
    nvgFillColor(vg, SurgeStyle::surgeOrange());
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, 0, orangeLine);
    nvgLineTo(vg, w, orangeLine);
    nvgStrokeColor(vg, SurgeStyle::surgeBlue());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
            
    nvgBeginPath(vg);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    nvgFillColor(vg, SurgeStyle::surgeWhite());
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 9);

    char version[1024];
    snprintf(version, 1023, "%s: %s.%s.%s",
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
    nvgText(vg, w/2, h - 2, version, NULL);

    nvgBeginPath(vg);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    nvgFillColor(vg, SurgeStyle::surgeOrange2());
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 30);
    nvgText(vg, w/2 + 1, h - 25 + 1, displayName.c_str(), NULL);

    nvgBeginPath(vg);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    nvgFillColor(vg, SurgeStyle::surgeWhite());
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 30);
    nvgText(vg, w/2, h - 25, displayName.c_str(), NULL);

    auto logoSvg = SurgeInternal::getSurgeLogo();
    if( logoSvg && logoSvg->handle )
    {
        // We want the logo to be 1.5 * screw width high
        float scaleFactor = 1.5 * SCREW_WIDTH / logoSvg->handle->height;
        float x0 = w/2 - logoSvg->handle->width * scaleFactor / 2;
        nvgSave(vg);
        nvgTranslate(vg, x0, 2);
        nvgScale(vg, scaleFactor, scaleFactor);
#if RACK_V1        
        rack::svgDraw(vg, logoSvg->handle);
#else
        // This is so unsatisfying but v6 doesn't expose svgDraw as a public API point so fake it with a temp widget
        rack::SVGWidget w;
        w.setSVG(logoSvg);
        w.draw(vg);
#endif        
        nvgRestore(vg);
    }

}