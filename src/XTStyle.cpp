/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "XTStyle.h"
#include "filesystem/import.h"
#include "rack.hpp"
#include "tinyxml/tinyxml.h"
#include "window/Svg.hpp"
namespace sst::surgext_rack::style
{

void XTStyle::initialize()
{
    std::string defaultsDir = rack::asset::user("SurgeXTRack/");
    if (!rack::system::isDirectory(defaultsDir))
        rack::system::createDirectory(defaultsDir);
    std::string defaultsFile = rack::asset::user("SurgeXTRack/default-skin.json");

    json_error_t error;
    json_t *fd{nullptr};
    auto *fptr = std::fopen(defaultsFile.c_str(), "r");
    if (fptr)
    {
        fd = json_loadf(fptr, 0, &error);
        DEFER({ std::fclose(fptr); });
    }
    if (!fd)
    {
#ifdef USING_CARDINAL_NOT_RACK
        setGlobalStyle(rack::settings::darkMode ? DARK : LIGHT);
#else
        setGlobalStyle(MID);
#endif
        setGlobalDisplayRegionColor(ORANGE);
        setGlobalModulationColor(BLUE);
        setGlobalControlValueColor(ORANGE);
        setControlValueColorDistinct(false);
        setShowKnobValuesAtRest(true);
        setShowModulationAnimationOnDisplay(true);
        setShowModulationAnimationOnKnobs(true);
        setGlobalPowerButtonColor(GREEN);
    }
    else
    {
        {
            json_t *defj = json_object_get(fd, "defaultSkin");
            int skinId{-1};
            if (defj)
                skinId = json_integer_value(defj);

            if (skinId >= Style::DARK && skinId <= Style::LIGHT)
            {
                setGlobalStyle((Style)skinId);
            }
            else
            {
                setGlobalStyle(MID);
            }
        }

        {
            json_t *defj = json_object_get(fd, "defaultLightColor");
            int lightColId{-1};
            if (defj)
                lightColId = json_integer_value(defj);

            if (lightColId >= ORANGE && lightColId <= WHITE)
            {
                setGlobalDisplayRegionColor((LightColor)lightColId);
            }
            else
            {
                setGlobalDisplayRegionColor(ORANGE);
            }
        }

        {
            json_t *defj = json_object_get(fd, "defaultModLightColor");
            int lightColId{-1};
            if (defj)
                lightColId = json_integer_value(defj);

            if (lightColId >= ORANGE && lightColId <= WHITE)
            {
                setGlobalModulationColor((LightColor)lightColId);
            }
            else
            {
                setGlobalModulationColor(BLUE);
            }
        }

        {
            json_t *defj = json_object_get(fd, "defaultControlValueColor");
            int lightColId{-1};
            if (defj)
                lightColId = json_integer_value(defj);

            if (lightColId >= ORANGE && lightColId <= WHITE)
            {
                setGlobalControlValueColor((LightColor)lightColId);
            }
            else
            {
                setGlobalControlValueColor(ORANGE);
            }
        }

        {
            json_t *defj = json_object_get(fd, "defaultPowerButtonColor");
            int lightColId{-1};
            if (defj)
                lightColId = json_integer_value(defj);

            if (lightColId >= ORANGE && lightColId <= WHITE)
            {
                setGlobalPowerButtonColor((LightColor)lightColId);
            }
            else
            {
                setGlobalPowerButtonColor(GREEN);
            }
        }

        auto handleBool = [&](auto name, auto op, auto def) {
            json_t *defj = json_object_get(fd, name);
            bool cvd{def};
            if (defj)
                cvd = json_boolean_value(defj);
            op(cvd);
        };
        handleBool("controlValueColorDistinct", setControlValueColorDistinct, false);
        handleBool("showKnobValuesAtRest", setShowKnobValuesAtRest, true);
        handleBool("showModulationAnimationOnKnobs", setShowModulationAnimationOnKnobs, true);
        handleBool("showModulationAnimationOnDisplay", setShowModulationAnimationOnDisplay, true);
        handleBool("showShadows", setShowShadows, true);
        handleBool("waveshaperShowsBothCurves", setWaveshaperShowsBothCurves, false);

        json_decref(fd);
    }
}

static XTStyle::Style defaultGlobalStyle{XTStyle::MID};
static XTStyle::LightColor defaultGlobalDisplayRegionColor{XTStyle::ORANGE};
static XTStyle::LightColor defaultGlobalModulationColor{XTStyle::BLUE};
static XTStyle::LightColor defaultGlobalControlValueColor{XTStyle::ORANGE};
static XTStyle::LightColor defaultGlobalPowerButtonColor{XTStyle::GREEN};
static bool controlValueColorDistinct{false};
static bool showKnobValuesAtRest{true};
static bool showModulationAnimationOnKnobs{true};
static bool showModulationAnimationOnDisplay{true};
static bool showShadows{true};
static bool waveshaperShowsBothCurves{false};

static std::shared_ptr<XTStyle> constructDefaultStyle()
{
    auto res = std::make_shared<XTStyle>();
    res->activeStyle = &defaultGlobalStyle;
    res->activeModulationColor = &defaultGlobalModulationColor;
    res->activeDisplayRegionColor = &defaultGlobalDisplayRegionColor;
    res->activeControlValueColor = &defaultGlobalControlValueColor;
    res->activePowerButtonColor = &defaultGlobalPowerButtonColor;
    return res;
}

void XTStyle::setGlobalStyle(sst::surgext_rack::style::XTStyle::Style s)
{
    if (s != defaultGlobalStyle)
    {
        defaultGlobalStyle = s;
        updateJSON();

        notifyStyleListeners();
    }
}
XTStyle::Style XTStyle::getGlobalStyle() { return defaultGlobalStyle; }

void XTStyle::setGlobalDisplayRegionColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (c != defaultGlobalDisplayRegionColor)
    {
        defaultGlobalDisplayRegionColor = c;
        updateJSON();

        notifyStyleListeners();
    }
}
XTStyle::LightColor XTStyle::getGlobalDisplayRegionColor()
{
    return defaultGlobalDisplayRegionColor;
}

bool XTStyle::getControlValueColorDistinct() { return controlValueColorDistinct; }
void XTStyle::setControlValueColorDistinct(bool b)
{
    if (b != controlValueColorDistinct)
    {
        controlValueColorDistinct = b;
        updateJSON();
        notifyStyleListeners();
    }
}

bool XTStyle::getShowKnobValuesAtRest() { return showKnobValuesAtRest; }
void XTStyle::setShowKnobValuesAtRest(bool b)
{
    if (b != showKnobValuesAtRest)
    {
        showKnobValuesAtRest = b;
        updateJSON();
        notifyStyleListeners();
    }
}

bool XTStyle::getShowModulationAnimationOnKnobs() { return showModulationAnimationOnKnobs; }
void XTStyle::setShowModulationAnimationOnKnobs(bool b)
{
    if (b != showModulationAnimationOnKnobs)
    {
        showModulationAnimationOnKnobs = b;
        updateJSON();
        notifyStyleListeners();
    }
}

bool XTStyle::getShowModulationAnimationOnDisplay() { return showModulationAnimationOnDisplay; }
void XTStyle::setShowModulationAnimationOnDisplay(bool b)
{
    if (b != showModulationAnimationOnDisplay)
    {
        showModulationAnimationOnDisplay = b;
        updateJSON();
        notifyStyleListeners();
    }
}

bool XTStyle::getShowShadows() { return showShadows; }
void XTStyle::setShowShadows(bool b)
{
    if (b != showShadows)
    {
        showShadows = b;
        updateJSON();
        notifyStyleListeners();
    }
}

bool XTStyle::getWaveshaperShowsBothCurves() { return waveshaperShowsBothCurves; }
void XTStyle::setWaveshaperShowsBothCurves(bool b)
{
    if (b != waveshaperShowsBothCurves)
    {
        waveshaperShowsBothCurves = b;
        updateJSON();
        notifyStyleListeners();
    }
}

void XTStyle::setGlobalModulationColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (c != defaultGlobalModulationColor)
    {
        defaultGlobalModulationColor = c;
        updateJSON();

        notifyStyleListeners();
    }
}

XTStyle::LightColor XTStyle::getGlobalModulationColor() { return defaultGlobalModulationColor; }

XTStyle::LightColor XTStyle::getGlobalControlValueColor() { return defaultGlobalControlValueColor; }

void XTStyle::setGlobalControlValueColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (c != defaultGlobalControlValueColor)
    {
        defaultGlobalControlValueColor = c;
        updateJSON();

        notifyStyleListeners();
    }
}

XTStyle::LightColor XTStyle::getGlobalPowerButtonColor() { return defaultGlobalPowerButtonColor; }
void XTStyle::setGlobalPowerButtonColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (c != defaultGlobalPowerButtonColor)
    {
        defaultGlobalPowerButtonColor = c;
        updateJSON();

        notifyStyleListeners();
    }
}

void XTStyle::updateJSON()
{
    std::string defaultsDir = rack::asset::user("SurgeXTRack/");
    if (!rack::system::isDirectory(defaultsDir))
        rack::system::createDirectory(defaultsDir);
    std::string defaultsFile = rack::asset::user("SurgeXTRack/default-skin.json");

    json_t *rootJ = json_object();
    json_t *stJ = json_integer(defaultGlobalStyle);
    json_t *lcJ = json_integer(defaultGlobalDisplayRegionColor);
    json_t *lcM = json_integer(defaultGlobalModulationColor);
    json_object_set_new(rootJ, "defaultSkin", stJ);
    json_object_set_new(rootJ, "defaultLightColor", lcJ);
    json_object_set_new(rootJ, "defaultModLightColor", lcM);
    json_object_set_new(rootJ, "defaultControlValueColor",
                        json_integer(defaultGlobalControlValueColor));
    json_object_set_new(rootJ, "defaultPowerButtonColor",
                        json_integer(defaultGlobalPowerButtonColor));
    json_object_set_new(rootJ, "controlValueColorDistinct",
                        json_boolean(controlValueColorDistinct));
    json_object_set_new(rootJ, "showKnobValuesAtRest", json_boolean(showKnobValuesAtRest));
    json_object_set_new(rootJ, "showModulationAnimationOnKnobs",
                        json_boolean(showModulationAnimationOnKnobs));
    json_object_set_new(rootJ, "showModulationAnimationOnDisplay",
                        json_boolean(showModulationAnimationOnDisplay));
    json_object_set_new(rootJ, "showShadows", json_boolean(showShadows));
    json_object_set_new(rootJ, "waveshaperShowsBothCurves",
                        json_boolean(waveshaperShowsBothCurves));
    FILE *f = std::fopen(defaultsFile.c_str(), "w");
    if (f)
    {
        json_dumpf(rootJ, f, JSON_INDENT(2));
        std::fclose(f);
    }
    json_decref(rootJ);
}
const std::shared_ptr<XTStyle> &StyleParticipant::style()
{
    if (!stylePtr)
        stylePtr = constructDefaultStyle();

    return stylePtr;
}
void StyleParticipant::attachToGlobalStyle()
{
    style()->activeStyle = &defaultGlobalStyle;
    style()->activeModulationColor = &defaultGlobalModulationColor;
    style()->activeDisplayRegionColor = &defaultGlobalDisplayRegionColor;
    style()->activeControlValueColor = &defaultGlobalControlValueColor;
    style()->activePowerButtonColor = &defaultGlobalPowerButtonColor;
}
void StyleParticipant::attachTo(style::XTStyle::Style *s, style::XTStyle::LightColor *display,
                                style::XTStyle::LightColor *modulation,
                                style::XTStyle::LightColor *control,
                                style::XTStyle::LightColor *power)
{
    style()->activeStyle = s;
    style()->activeDisplayRegionColor = display;
    style()->activeModulationColor = modulation;
    style()->activeControlValueColor = control;
    style()->activePowerButtonColor = power;
}

const NVGcolor XTStyle::getColor(sst::surgext_rack::style::XTStyle::Colors c)
{
    switch (c)
    {

    case PANEL_RULER:
    case KNOB_RING:
    {
        switch (*activeStyle)
        {
        case DARK:
            return nvgRGB(82, 82, 82);
        case MID:
            return nvgRGB(40, 40, 40);
        case LIGHT:
            return nvgRGB(194, 194, 194);
        }
    }

    case KNOB_MOD_PLUS:
    case MOD_BUTTON_LIGHT_ON:
        return lightColorColor(*activeModulationColor);
    case KNOB_MOD_MINUS:
        return nvgRGB(180, 180, 220);
    case KNOB_MOD_MARK:
        return nvgRGB(255, 255, 255);

    case LED_PANEL:
        return nvgRGB(0x11, 0x11, 0x11);
    case LED_BORDER:
        return nvgRGB(0x00, 0x00, 0x00);
    case LED_HIGHLIGHT:
    {
        switch (*activeStyle)
        {
        case DARK:
            return nvgRGB(0x4d, 0x4d, 0x4d);
        case MID:
            return nvgRGB(0x72, 0x72, 0x72);
        case LIGHT:
            return nvgRGB(0xFA, 0xFA, 0xFA);
        }
    }

    case PLOT_MARKS:
        return nvgRGB(60, 60, 60);

    case POWER_BUTTON_LIGHT_ON:
        return lightColorColor(*activePowerButtonColor);

    case MOD_BUTTON_LIGHT_OFF:
    case POWER_BUTTON_LIGHT_OFF:
        return nvgRGB(0x82, 0x82, 0x82);

    case TEXT_LABEL:
    {
        switch (*activeStyle)
        {
        case DARK:
        case MID:
            return nvgRGB(201, 201, 201);
        case LIGHT:
            return nvgRGB(82, 82, 82);
        }
    }

    case TEXT_LABEL_OUTPUT:
        return nvgRGB(201, 201, 201);

    case PLOT_CONTROL_VALUE_FG:
        return nvgRGB(0, 0, 0);

    case KNOB_RING_VALUE:
    case SLIDER_RING_VALUE:
    {
        if (!getShowKnobValuesAtRest())
            return nvgRGBA(0, 0, 0, 0);

        auto col = *activeDisplayRegionColor;
        if (getControlValueColorDistinct())
            col = *activeControlValueColor;
        if (col == WHITE && *activeStyle == LIGHT)
        {
            // Special case - white ring on light background
            if (c == SLIDER_RING_VALUE)
                return nvgRGB(150, 150, 150);
            return nvgRGB(0x33, 0x33, 0x33);
        }

        return lightColorColor(col);
    }

    case PLOT_CURVE:
    case PLOT_CONTROL_TEXT:
    case PLOT_CONTROL_VALUE_BG:
    {
        return lightColorColor(*activeDisplayRegionColor);
    }

    case SHADOW_BASE:
    {
        switch (*activeStyle)
        {
        case DARK:
            return nvgRGBAf(0.f, 0.f, 0.f, 0.25f);
        case MID:
            return nvgRGBAf(0.f, 0.f, 0.f, 0.15f);
        case LIGHT:
            return nvgRGBAf(0.f, 0.f, 0.f, 0.10f);
        }
    }

    case SHADOW_OVER_GRADEND:
        return nvgRGBA(0, 0, 0, 0);
    case SHADOW_OVER_GRADSTART:
    {
        switch (*activeStyle)
        {
        case DARK:
            return nvgRGBAf(0.f, 0.f, 0.f, 0.60f);
        case MID:
            return nvgRGBAf(0.f, 0.f, 0.f, 0.45f);
        case LIGHT:
            return nvgRGBAf(0.f, 0.f, 0.f, 0.20f);
        }
    }

    case OUTPUTBG_START:
    {
        switch (*activeStyle)
        {
        case DARK:
        case LIGHT:
            return nvgRGB(0x52, 0x52, 0x52);
        default:
            return nvgRGB(0x27, 0x27, 0x29);
        }
    }

    case OUTPUTBG_END:
    {
        switch (*activeStyle)
        {
        case DARK:
        case LIGHT:
            return nvgRGB(0x52, 0x52, 0x52);
        default:
            return nvgRGB(0x1E, 0x1E, 0x20);
        }
    }
    }

    return nvgRGB(255, 0, 0);
}

NVGcolor XTStyle::lightColorColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    switch (c)
    {
    case ORANGE:
        return nvgRGB(0xFF, 0x90, 0x00);
    case YELLOW:
        return nvgRGB(255, 214, 0);
    case GREEN:
        return nvgRGB(114, 234, 101);
    case AQUA:
        return nvgRGB(19, 236, 196);
    case BLUE:
        return nvgRGB(26, 167, 255);
    case PURPLE:
        return nvgRGB(158, 130, 243);
    case PINK:
        return nvgRGB(255, 82, 163);
    case WHITE:
        return nvgRGB(235, 235, 235);
    case RED:
        return nvgRGB(240, 67, 67);
    }

    return nvgRGB(255, 0, 1);
}

std::unordered_set<StyleParticipant *> XTStyle::listeners;

std::string XTStyle::styleName(sst::surgext_rack::style::XTStyle::Style s)
{
    switch (s)
    {
    case DARK:
        return "Dark";
    case MID:
        return "Medium";
    case LIGHT:
        return "Light";
    }
    return "ERROR";
}

std::string XTStyle::lightColorName(sst::surgext_rack::style::XTStyle::LightColor c)
{
    switch (c)
    {
    case ORANGE:
        return "Orange";
    case YELLOW:
        return "Yellow";
    case GREEN:
        return "Green";
    case AQUA:
        return "Aqua";
    case BLUE:
        return "Blue";
    case PURPLE:
        return "Purple";
    case PINK:
        return "Pink";
    case WHITE:
        return "High Contrast";
    case RED:
        return "Red";
    }
    return "ERROR";
}

std::string XTStyle::skinAssetDir()
{
    switch (*activeStyle)
    {
    case DARK:
        return "res/xt/dark";
    case MID:
        return "res/xt/mid";
    case LIGHT:
        return "res/xt/light";
    }
    return "error";
}

void XTStyle::notifyStyleListeners()
{
    for (auto l : listeners)
        l->onStyleChanged();
}

#define TEST_FONTS 0

static const char *fontFace()
{
#if TEST_FONTS
    return "res/xt/fonts/test-fonts/UglyTypist.ttf";
#else
    return "res/xt/fonts/quicksand/Quicksand-Regular.ttf";
#endif
}
static const char *fontFaceBold()
{
#if TEST_FONTS
    return "res/xt/fonts/test-fonts/PlayfairDisplay.ttf";
#else
    return "res/xt/fonts/quicksand/Quicksand-Bold.ttf";
#endif
}

int XTStyle::fontId(NVGcontext *vg)
{
    auto fontPath = rack::asset::plugin(pluginInstance, fontFace());
    auto font = APP->window->loadFont(fontPath);
    return font->handle;
}

int XTStyle::fontIdBold(NVGcontext *vg)
{
    auto fontPath = rack::asset::plugin(pluginInstance, fontFaceBold());
    auto font = APP->window->loadFont(fontPath);
    return font->handle;
}
} // namespace sst::surgext_rack::style
