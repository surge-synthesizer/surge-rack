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
        setCurrentStyle(MID);
        setCurrentLightColor(ORANGE);
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
                setCurrentStyle((Style)skinId);
            }
            else
            {
                setCurrentStyle(MID);
            }
        }

        {
            json_t *defj = json_object_get(fd, "defaultLightColor");
            int lightColId{-1};
            if (defj)
                lightColId = json_integer_value(defj);

            if (lightColId >= ORANGE && lightColId <= GREEN)
            {
                setCurrentLightColor((LightColor)lightColId);
            }
            else
            {
                setCurrentLightColor(ORANGE);
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

static XTStyle::Style currentStyle{XTStyle::MID};
static XTStyle::LightColor currentLightColor{XTStyle::ORANGE};
static std::shared_ptr<XTStyle> currentStyleP;
void XTStyle::setCurrentStyle(sst::surgext_rack::style::XTStyle::Style s)
{
    if (!currentStyleP)
        currentStyleP = std::make_shared<XTStyle>();
    if (s != currentStyle)
    {
        currentStyle = s;
        updateJSON();

        notifyStyleListeners();
    }
}

void XTStyle::setCurrentLightColor(sst::surgext_rack::style::XTStyle::LightColor c)
{
    if (!currentStyleP)
        currentStyleP = std::make_shared<XTStyle>();
    if (c != currentLightColor)
    {
        currentLightColor = c;
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
    json_t *stJ = json_integer(currentStyle);
    json_t *lcJ = json_integer(currentLightColor);
    json_object_set_new(rootJ, "defaultSkin", stJ);
    json_object_set_new(rootJ, "defaultLightColor", lcJ);
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
    if (!currentStyleP)
        currentStyleP = std::make_shared<XTStyle>();

    return currentStyleP;
}

const NVGcolor XTStyle::getColor(sst::surgext_rack::style::XTStyle::Colors c)
{
    switch (c)
    {
    case KNOB_RING:
    {
        switch (currentStyle)
        {
        case DARK:
            return nvgRGB(82, 82, 82);
        case MID:
            return nvgRGB(40, 40, 40);
        case LIGHT:
            return nvgRGB(194, 194, 194);
        }
    }
    case KNOB_MOD_MINUS:
        return nvgRGB(180, 180, 220);
    case KNOB_MOD_MARK:
        return nvgRGB(255, 255, 255);

    case PLOT_MARKS:
        return nvgRGB(60, 60, 60);

    case MOD_BUTTON_LIGHT_OFF:
        return nvgRGB(0x82, 0x82, 0x82);

    case TEXT_LABEL:
    {
        switch (currentStyle)
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

    case KNOB_MOD_PLUS:
    case PLOT_CURVE:
    case PLOT_CONTROL_TEXT:
    case PLOT_CONTROL_VALUE_BG:
    case MOD_BUTTON_LIGHT_ON:
    {
        switch (currentLightColor)
        {
        case ORANGE:
            return nvgRGB(0xFF, 0x90, 0x00);
        case BLUE:
            return nvgRGB(0x70, 0x70, 0xFF);
        case RED:
            return nvgRGB(0xFF, 0x30, 0x30);
        case GREEN:
            return nvgRGB(0x20, 0xFF, 0x20);
        }
    }
    }

    return nvgRGB(255, 0, 0);
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
    case BLUE:
        return "Blue";
    case GREEN:
        return "Green";
    case RED:
        return "Red";
    }
    return "ERROR";
}

std::string XTStyle::skinAssetDir()

{
    switch (currentStyle)
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
