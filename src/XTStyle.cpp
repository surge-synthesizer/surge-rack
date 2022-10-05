#include "XTStyle.hpp"
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
    json_t *fd = json_load_file(defaultsFile.c_str(), 0, &error);
    if (!fd)
    {
        setGlobalStyle(MID);
        setGlobalLightColor(ORANGE);
        setGlobalModLightColor(BLUE);
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

            if (lightColId >= ORANGE && lightColId <= RED)
            {
                setGlobalLightColor((LightColor)lightColId);
            }
            else
            {
                setGlobalLightColor(ORANGE);
            }
        }

        {
            json_t *defj = json_object_get(fd, "defaultModLightColor");
            int lightColId{-1};
            if (defj)
                lightColId = json_integer_value(defj);

            if (lightColId >= ORANGE && lightColId <= RED)
            {
                setGlobalModLightColor((LightColor)lightColId);
            }
            else
            {
                setGlobalModLightColor(BLUE);
            }
        }
        json_decref(fd);
    }
}

// Font dictionary
struct InternalFontMgr
{
    static std::map<std::string, int> fontMap;
    static int get(NVGcontext *vg, std::string resName)
    {
        if (fontMap.find(resName) == fontMap.end())
        {
            std::string fontPath = rack::asset::plugin(pluginInstance, resName);

            fontMap[resName] = nvgCreateFont(vg, resName.c_str(), fontPath.c_str());
        }
        return fontMap[resName];
    }
};

static XTStyle::Style defaultGlobalStyle{XTStyle::MID};
static XTStyle::LightColor defaultGlobalLightColor{XTStyle::ORANGE};
static XTStyle::LightColor defaultGlobalModLightColor{XTStyle::BLUE};
static std::shared_ptr<XTStyle> constructDefaultStyle()
{
    auto res = std::make_shared<XTStyle>();
    res->activeStyle = &defaultGlobalStyle;
    res->activeModLight = &defaultGlobalModLightColor;
    res->activeLight = &defaultGlobalLightColor;
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

void XTStyle::setGlobalLightColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (c != defaultGlobalLightColor)
    {
        defaultGlobalLightColor = c;
        updateJSON();

        notifyStyleListeners();
    }
}
XTStyle::LightColor XTStyle::getGlobalLightColor() { return defaultGlobalLightColor; }

void XTStyle::setGlobalModLightColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (c != defaultGlobalModLightColor)
    {
        defaultGlobalModLightColor = c;
        updateJSON();

        notifyStyleListeners();
    }
}

XTStyle::LightColor XTStyle::getGlobalModLightColor() { return defaultGlobalModLightColor; }

void XTStyle::updateJSON()
{
    std::string defaultsDir = rack::asset::user("SurgeXTRack/");
    if (!rack::system::isDirectory(defaultsDir))
        rack::system::createDirectory(defaultsDir);
    std::string defaultsFile = rack::asset::user("SurgeXTRack/default-skin.json");

    json_t *rootJ = json_object();
    json_t *stJ = json_integer(defaultGlobalStyle);
    json_t *lcJ = json_integer(defaultGlobalLightColor);
    json_t *lcM = json_integer(defaultGlobalModLightColor);
    json_object_set_new(rootJ, "defaultSkin", stJ);
    json_object_set_new(rootJ, "defaultLightColor", lcJ);
    json_object_set_new(rootJ, "defaultModLightColor", lcM);
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
    style()->activeModLight = &defaultGlobalModLightColor;
    style()->activeLight = &defaultGlobalLightColor;
}
void StyleParticipant::attachTo(style::XTStyle::Style *s, style::XTStyle::LightColor *l,
                                style::XTStyle::LightColor *m)
{
    style()->activeStyle = s;
    style()->activeLight = l;
    style()->activeModLight = m;
}

const NVGcolor XTStyle::getColor(sst::surgext_rack::style::XTStyle::Colors c)
{
    switch (c)
    {
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
        return lightColorColor(*activeModLight);
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

    case MOD_BUTTON_LIGHT_OFF:
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
    case PLOT_CURVE:
    case PLOT_CONTROL_TEXT:
    case PLOT_CONTROL_VALUE_BG:
    {
        return lightColorColor(*activeLight);
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
    case RED:
        return nvgRGB(240, 67, 67);
    }
}
std::map<std::string, int> InternalFontMgr::fontMap;

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
    static int fid{-1};
    if (fid < 0)
        fid = InternalFontMgr::get(vg, fontFace());
    return fid;
}

int XTStyle::fontIdBold(NVGcontext *vg)
{
    static int fid{-1};
    if (fid < 0)
        fid = InternalFontMgr::get(vg, fontFaceBold());
    return fid;
}
} // namespace sst::surgext_rack::style
