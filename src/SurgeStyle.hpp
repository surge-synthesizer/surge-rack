/*
** Functions for shared and consistent sytling, including colors, drawing
** gradients, and drawing panel backgrounds
*/
#pragma once
#include "Surge.hpp"
#include "rack.hpp"
#include <map>
#include <vector>
#include <unordered_set>
#include <unordered_map>

// Font dictionary
struct InternalFontMgr {
    static std::map<std::string, int> fontMap;
    static int get(NVGcontext *vg, std::string resName) {
        if (fontMap.find(resName) == fontMap.end()) {
            std::string fontPath = rack::asset::plugin(pluginInstance, resName);

            fontMap[resName] =
                nvgCreateFont(vg, resName.c_str(), fontPath.c_str());
        }
        return fontMap[resName];
    }
};

struct SurgeStyle {
#define GETCOLFN(nm) static NVGcolor nm () { return getColorFromMap( #nm ); } static std::string nm##_KEY() { return #nm; }

    GETCOLFN(panelBackground);
    GETCOLFN(panelBackgroundOutline);
    GETCOLFN(panelFooter);
    GETCOLFN(panelFooterOutline);
    GETCOLFN(panelFooterSeparator);
    GETCOLFN(panelLabel);
    GETCOLFN(panelLabelRule);
    GETCOLFN(panelTitle);
    GETCOLFN(panelSeparator);

    GETCOLFN(switchHandle);
    
    GETCOLFN(ioRegionText);
    GETCOLFN(ioRegionBackgroundGradientStart);
    GETCOLFN(ioRegionBackgroundGradientEnd);
    GETCOLFN(ioRegionBorderHighlight);
    GETCOLFN(ioRegionBorder);

    GETCOLFN(parameterValueText);
    GETCOLFN(parameterNameText);
    GETCOLFN(parameterScrollCenter);
    GETCOLFN(parameterScrollEnd);
    
    GETCOLFN(textBGGradientStart);
    GETCOLFN(textBGGradientEnd);
    GETCOLFN(textBGBorderHighlight);
    GETCOLFN(textBGBorder);


    GETCOLFN( buttonBankSelectedGradientTop);
    GETCOLFN( buttonBankSelectedGradientMid);
    GETCOLFN( buttonBankSelectedGradientEnd);
    GETCOLFN( buttonBankSelectedOutline);

    GETCOLFN( buttonBankSelectedLightGradientTop);
    GETCOLFN( buttonBankSelectedLightGradientEnd);
    GETCOLFN( buttonBankSelectedLightOutline);

    GETCOLFN( buttonBankSelectedLabelGlow);
    GETCOLFN( buttonBankSelectedLabel);

    GETCOLFN(buttonBankUnselectedGradientTop);
    GETCOLFN(buttonBankUnselectedGradientMid);
    GETCOLFN(buttonBankUnselectedGradientEnd);
    GETCOLFN(buttonBankUnselectedOutline);

    GETCOLFN(buttonBankUnselectedLightFill);
    GETCOLFN(buttonBankUnselectedLightOutline);
    GETCOLFN(buttonBankUnselectedLabel);

    GETCOLFN(buttonBankDropShadow);

    static const char *fontFace() {
        return "res/EncodeSansSemiCondensed-Medium.ttf";
    }
    static const char *fontFaceCondensed() {
        return "res/EncodeSansCondensed-Medium.ttf";
    }

    static int fid, fidcond;
    static int fontId(NVGcontext *vg) {
        if( fid < 0 )
            fid = InternalFontMgr::get(vg, fontFace() );
        return fid;
    }
    static int fontIdCondensed(NVGcontext *vg) {
        if( fidcond < 0 )
            fidcond = InternalFontMgr::get(vg, fontFaceCondensed() );
        return fidcond;
    }

    
    static void drawBlueIORect(NVGcontext *vg, float x0, float y0, float w,
                               float h,
                               int direction = 0); // 0 is L->R 1 is R->L
    static void drawTextBGRect(NVGcontext *vg, float x0, float y0, float w,
                               float h);
    static void drawPanelBackground(NVGcontext *vg, float w, float h,
                                    std::string displayName, bool narrowMode);


    /*
      Component Sizes
     */
    static float constexpr portX = 24.6721;
    static float constexpr portY = 24.6721;
    static float constexpr surgeKnobX = 24;
    static float constexpr surgeKnobY = 24;
    static float constexpr surgeRoosterX = 34;
    static float constexpr surgeRoosterY = 34;
    static float constexpr surgeSwitchX = 14;
    static float constexpr surgeSwitchY = 21;

    /*
      Overall spacing
     */
    static float constexpr padMargin = 3;
    static float constexpr padFromEdge = 5;
    static float constexpr padFromTop = SCREW_WIDTH + padMargin;
    static float constexpr orangeLine = 323;
    static float constexpr ioMargin = 7; // distance orange line to blue box

    static float constexpr ioRegionWidth = 105;
    static float constexpr clockPad = 12;
    static float constexpr clockW = portX + 2 * padMargin + clockPad;
    
    

    /*
      Standard IO regiopn locations
     */
    rack::Vec ioPortLocation(bool input,
                             int ctrl,
                             rack::Rect box,
                             int stackVertically = false
        ) { // 0 is L; 1 is R; 2 is gain
        if( stackVertically )
        {
            float x0 = (box.size.x - ioRegionWidth)/2;
            float yRes = orangeLine + 1.5 * ioMargin;
            float xRes = x0 + ioMargin + ctrl * (portX + 4);
            if( input )
            {
                float boxHt = box.size.y - orangeLine - 2 * ioMargin;

                yRes = orangeLine - 0.5 * ioMargin - boxHt;
            }
            return rack::Vec( xRes, yRes );
        }
        else
        {
            int x0 = 0;
            if (!input)
                x0 = box.size.x - ioRegionWidth - 2 * ioMargin;
            
            int padFromEdge = input ? 17 : 5;
            float xRes =
                x0 + ioMargin + padFromEdge + (ctrl * (portX + 4));
            float yRes = orangeLine + 1.5 * ioMargin;
            
            return rack::Vec(xRes, yRes);
        }
    }

    void drawLeftRightInputOutputBackground( NVGcontext *vg, rack::Rect box, std::string inputName = "Input",
                                             bool doIn = true, bool doOut=true ) {
        for (int i = (doIn?0:1); i < (doOut?2:1); ++i) {
            nvgBeginPath(vg);
            int x0 = 0;
            if (i == 1)
                x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

            SurgeStyle::drawBlueIORect(
                vg, x0 + ioMargin, orangeLine + ioMargin,
                ioRegionWidth,
                box.size.y - orangeLine - 2 * ioMargin,
                (i == 0) ? 0 : 1);

            nvgFillColor(vg, ioRegionText());
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 12);
            if (i == 0) {
                nvgSave(vg);
                nvgTranslate(vg, x0 + ioMargin + 2,
                             orangeLine + ioMargin * 1.5);
                nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgRotate(vg, -M_PI / 2);
                nvgText(vg, 0, 0, inputName.c_str(), NULL);
                nvgRestore(vg);
            } else {
                nvgSave(vg);
                nvgTranslate(vg, x0 + ioMargin + ioRegionWidth - 2,
                             orangeLine + ioMargin * 1.5);
                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgRotate(vg, M_PI / 2);
                nvgText(vg, 0, 0, "Output", NULL);
                nvgRestore(vg);
            }
            rack::Vec ll;
            ll = ioPortLocation(i == 0, 0, box);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "L/Mon", NULL);

            ll = ioPortLocation(i == 0, 1, box);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "R", NULL);

            ll = ioPortLocation(i == 0, 2, box);
            ll.y = box.size.y - ioMargin - 1.5;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "Gain", NULL);
        }
    }

    void drawStackedInputOutputBackground( NVGcontext *vg, rack::Rect box, std::string inputName = "Input" ) {
        for (int i = 0; i < 2; ++i) {
            nvgBeginPath(vg);
            float x0 = ( box.size.x - ioRegionWidth ) / 2;
            float y0 = orangeLine + ioMargin;
            float boxHt = box.size.y - orangeLine - 2 * ioMargin;
            if( i == 0 )
                y0 = orangeLine - boxHt - ioMargin;
            
            SurgeStyle::drawBlueIORect(
                vg, x0, y0,
                ioRegionWidth,
                boxHt,
                (i == 0) ? 0 : 1);

            nvgFillColor(vg, ioRegionText());
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 12);
            nvgSave(vg);
            nvgTranslate(vg, x0 + ioRegionWidth - 2, y0 + ioMargin * 0.5 );
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgRotate(vg, M_PI / 2);
            if (i == 0) {
                nvgText(vg, 0, 0, inputName.c_str(), NULL);
            } else {
                nvgText(vg, 0, 0, "Output", NULL);
            }
            nvgRestore(vg);
            rack::Vec ll;
            ll = ioPortLocation(i == 0, 0, box, true);
            ll.y += boxHt - ioMargin;
            ll.x += 24.6721 / 2;
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "L/Mon", NULL);

            ll = ioPortLocation(i == 0, 1, box, true);
            ll.y += boxHt - ioMargin;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "R", NULL);

            ll = ioPortLocation(i == 0, 2, box, true);
            ll.y += boxHt - ioMargin;
            ll.x += 24.6721 / 2;
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, ll.x, ll.y, "Gain", NULL);
        }
    }

    void clockBackground(NVGcontext *vg, float x, float y, bool vertical = false) {
        if( vertical )
        {
            drawBlueIORect(vg, x, y, portX + 2 * padMargin, portY + 2 * padMargin + clockPad);

            nvgBeginPath(vg);
            nvgSave(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 10);
            nvgFillColor(vg, ioRegionText() );
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER );
            nvgText(vg, x + + portX / 2 + padMargin, y + padMargin, "bpm cv", NULL );
            nvgRestore(vg);
        }
        else
        {
            drawBlueIORect(vg, x, y, portX + 2 * padMargin + clockPad, portY + 2 * padMargin);
            
            nvgBeginPath(vg);
            nvgSave(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 10);
            nvgTranslate(vg, x + padMargin + portX + clockPad - 10, y );
            nvgRotate( vg, M_PI/2 );
            nvgFillColor(vg, ioRegionText() );
            nvgTextAlign(vg, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT );
            nvgText(vg, padMargin, 0, "bpm cv", NULL );
            nvgRestore(vg);
        }
        

    }
    
    float fxGroupLabel( NVGcontext *vg, float y, const char* label, rack::Rect box ) {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 14);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT );
        nvgFillColor( vg, panelLabel());
        nvgText( vg, padFromEdge, y, label, NULL );

        float bounds[4];
        nvgTextBounds( vg, padFromEdge, y, label, NULL, bounds );
        float xp = bounds[ 2 ] + padMargin;

        float a, d, h;
        nvgTextMetrics( vg, &a, &d, &h );
        float yp = y + h + d;
        
        nvgBeginPath(vg);
        nvgMoveTo(vg, xp, yp );
        nvgLineTo(vg, box.size.x - padFromEdge, yp );
        nvgStrokeColor( vg, panelLabelRule() );
        nvgStrokeWidth( vg, 1 );
        nvgStroke( vg );
        return y + h;
    }

    void centerRuledLabel( NVGcontext *vg, float x0, float y0, float w, const char* label, int size = 14 ) {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, size);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER );
        nvgFillColor( vg, panelLabel());
        nvgText( vg, x0 + w/2, y0, label, NULL );

        float bounds[4];
        nvgTextBounds( vg, x0 + w/2, y0, label, NULL, bounds );

        float a, d, h;
        nvgTextMetrics( vg, &a, &d, &h );
        float yp = y0 + h + d;

        nvgBeginPath(vg);
        nvgMoveTo(vg, x0, yp);
        nvgLineTo(vg, bounds[0] - padMargin, yp);
        nvgStrokeColor( vg, panelLabelRule() );
        nvgStrokeWidth( vg, 1 );
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgMoveTo(vg, bounds[2] + padMargin, yp);
        nvgLineTo(vg, x0 + w, yp);
        nvgStrokeColor( vg, panelLabelRule() );
        nvgStrokeWidth( vg, 1 );
        nvgStroke(vg);

    }

    void simpleLabel( NVGcontext *vg, float x, float y, const char* label, int al = NVG_ALIGN_LEFT | NVG_ALIGN_TOP, int fs = 12 ) {
        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, fs);
        nvgFillColor(vg, panelLabel() );
        nvgTextAlign(vg, al );
        nvgText( vg, x, y, label, NULL );
    }

    void dropRightLine(NVGcontext *vg, float x0, float y0, float x1, float y1) {
        int radius = 3;
        nvgBeginPath(vg);
        nvgMoveTo(vg, x0, y0);
        nvgLineTo(vg, x0, y1-radius);
        nvgArcTo(vg, x0, y1, x0+radius, y1, radius);
        nvgLineTo(vg, x1, y1);
        nvgStrokeColor(vg, panelLabelRule());
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
    }


    /*
    ** Since styles are dynamic, we want to be able to be notified when they change
    */
    struct StyleListener {
        virtual void styleHasChanged() = 0;
    };
    static void addStyleListener( StyleListener *l ) {
        listeners.insert(l);
    }
    static void removeStyleListener( StyleListener *l ) {
        listeners.erase(l);
    }
    static void notifyStyleListeners() {
        for( auto l : listeners )
            l->styleHasChanged();
    }

    static void loadStyle(std::string styleXml = "");

    static std::string currentStyle;
    static std::vector<std::string> styleList;
    static std::unordered_set<StyleListener *> listeners;

    static std::unordered_map<std::string, NVGcolor> colorMap;
    static std::unordered_map<std::string, std::string> assets;
    
    /*
    ** These are the logical colors we need
    */
    static inline NVGcolor getColorFromMap( std::string nm ) {
        // This is here so we can do a super efficient version later but for now
        auto it = colorMap.find(nm);
        if( it == colorMap.end() )
        {
            rack::WARN( "Lookup failed for color '%s'", nm.c_str() );
            return nvgRGB( 255, 0, 0 );
        }
        return it->second;
    }
    
    static inline std::string getAssetPath( std::string nm ) {
        // This is here so we can do a super efficient version later but for now
        auto it = assets.find(nm);
        if( it == assets.end() )
        {
            rack::WARN( "Lookup failed for asset '%s'", nm.c_str() );
            return "";
        }
        return it->second;
    }
    

};
