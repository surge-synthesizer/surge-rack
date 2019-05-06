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
    static NVGcolor surgeOrange2() { return nvgRGBA(101, 50, 3, 255); }
    static NVGcolor surgeOrange3() { return nvgRGBA(227, 112, 8, 255); }

    static NVGcolor textBG() { return nvgRGBA(27, 28, 32, 255); }
    static NVGcolor textBGBright() { return nvgRGBA(60, 60, 72, 255); }
    static NVGcolor textBGVeryBright() { return nvgRGBA(90, 90, 112, 255); }

    static NVGcolor backgroundDarkGray() { return nvgRGBA(175, 176, 182, 255); }
    static NVGcolor backgroundGray() { return nvgRGBA(205, 206, 212, 255); }
    static NVGcolor backgroundLightGray() {
        return nvgRGBA(215, 216, 222, 255);
    }
    static NVGcolor backgroundLightOrange() {
        return nvgRGBA(239, 210, 172, 255);
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
