/*
** Functions for shared and consistent sytling, including colors, drawing
** gradients, and drawing panel backgrounds
*/
#pragma once
#include "Surge.hpp"
#include "rack.hpp"
#include <map>

struct SurgeStyle {
    static NVGcolor surgeBlue() { return nvgRGBA(18, 52, 99, 255); }
    static NVGcolor surgeBlueBright() {
        return nvgRGBA(18 * 1.5, 52 * 1.5, 99 * 1.8, 255);
    }
    static NVGcolor surgeBlueVeryBright() {
        return nvgRGBA(18 * 1.8, 52 * 1.8, 99 * 2.1, 255);
    }
    static NVGcolor surgeBlueDark() {
        return nvgRGBA(18 * 0.6, 52 * 0.6, 99 * 0.8, 255);
    }

    static NVGcolor surgeWhite() { return nvgRGBA(255, 255, 255, 255); }
    static NVGcolor surgeOrange() { return nvgRGBA(255, 144, 0, 255); }
    static NVGcolor surgeOrangeMedium() { return nvgRGBA(227, 112, 8, 255); }
    static NVGcolor surgeOrangeDark() { return nvgRGBA(101, 50, 3, 255); }

    static NVGcolor textBG() { return nvgRGBA(27, 28, 32, 255); }
    static NVGcolor textBGBright() { return nvgRGBA(60, 60, 72, 255); }
    static NVGcolor textBGVeryBright() { return nvgRGBA(90, 90, 112, 255); }

    static NVGcolor backgroundDarkGray() { return nvgRGBA(175, 176, 182, 255); }
    static NVGcolor backgroundGray() { return nvgRGBA(205, 206, 212, 255); }
    static NVGcolor backgroundGrayTrans() { return nvgRGBA(205, 206, 212, 0); }
    static NVGcolor backgroundLightGray() {
        return nvgRGBA(215, 216, 222, 255);
    }
    static NVGcolor backgroundLightOrange() {
        return nvgRGBA(239, 210, 172, 255);
    }

    static NVGcolor buttonBoxPressedStroke() {
        return nvgRGB(0xf0, 0x8f, 0x35);
    }
    static NVGcolor buttonBoxPressedFill() {
        return nvgRGB(0xf1, 0xb8, 0x7d);
    }
    static NVGcolor buttonBoxPressedText() {
        return nvgRGB(0x1a, 0x34, 0x60);
    }

    static NVGcolor buttonBoxOpenStroke() {
        return nvgRGB(0xbd, 0xbb, 0xac);
    }
    static NVGcolor buttonBoxOpenFill() {
        return nvgRGB(0xf0, 0xee, 0xdc);
    }
    static NVGcolor buttonBoxOpenText() {
        return nvgRGB(0xc8, 0x7b, 0x2c);
    }

    static NVGcolor buttonBoxContainerStroke() {
        return nvgRGB(0xc8, 0x7b, 0x2c);
    }
    static NVGcolor buttonBoxContainerFill() {
        return nvgRGB(0x37, 0x36, 0x32);
    }

    static const char *fontFace() {
        return "res/EncodeSansSemiCondensed-Medium.ttf";
    }
    static const char *fontFaceCondensed() {
        return "res/EncodeSansCondensed-Medium.ttf";
    }

    static void drawBlueIORect(NVGcontext *vg, float x0, float y0, float w,
                               float h,
                               int direction = 0); // 0 is L->R 1 is R->L
    static void drawTextBGRect(NVGcontext *vg, float x0, float y0, float w,
                               float h);
    static void drawPanelBackground(NVGcontext *vg, float w, float h,
                                    std::string displayName, bool narrowMode);
};

struct SurgeLayout {
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
            fontMap[resName] =
                nvgCreateFont(vg, resName.c_str(), fontPath.c_str());
        }
        return fontMap[resName];
    }
};

#ifndef RACK_V1
using rack::INFO_LEVEL;
#endif

