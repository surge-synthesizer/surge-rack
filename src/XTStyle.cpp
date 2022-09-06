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
        setCurrentStyle(DARK);
    }
    else
    {
        json_t *defj = json_object_get(fd, "defaultSkin");
        int skinId{-1};
        if (defj)
            skinId = json_integer_value(defj);

        if (skinId >= Style::DARK && skinId <= Style::LIGHT)
        {
            setCurrentStyle((Style)skinId);
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

static XTStyle::Style currentStyle{XTStyle::DARK};
static std::shared_ptr<XTStyle> currentStyleP;
void XTStyle::setCurrentStyle(sst::surgext_rack::style::XTStyle::Style s)
{
    if (!currentStyleP)
        currentStyleP = std::make_shared<XTStyle>();
    if (s != currentStyle)
    {
        currentStyle = s;

        std::string defaultsDir = rack::asset::user("SurgeXTRack/");
        if (!rack::system::isDirectory(defaultsDir))
            rack::system::createDirectory(defaultsDir);
        std::string defaultsFile = rack::asset::user("SurgeXTRack/default-skin.json");

        json_t *rootJ = json_object();
        json_t *stJ = json_integer(currentStyle);
        json_object_set_new(rootJ, "defaultSkin", stJ);
        FILE *f = std::fopen(defaultsFile.c_str(), "w");
        if (f)
        {
            json_dumpf(rootJ, f, JSON_INDENT(2));
            std::fclose(f);
        }
        json_decref(rootJ);

        notifyStyleListeners();
    }
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
    case KNOB_MOD_PLUS:
        return nvgRGB(0xFF, 0x90, 0x00);
    case KNOB_MOD_MINUS:
        return nvgRGB(180, 180, 220);
    case KNOB_MOD_MARK:
        return nvgRGB(255, 255, 255);

    case PLOT_CURVE:
        return nvgRGB(0xFF, 0x90, 0x00);
    case PLOT_MARKS:
        return nvgRGB(60, 60, 60);

    case MOD_BUTTON_LIGHT_ON:
        return nvgRGB(0xFF, 0x90, 0x00);
    case MOD_BUTTON_LIGHT_OFF:
        return nvgRGB(0x82, 0x82, 0x82);
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

static const char *fontFace() { return "res/xt/fonts/quicksand/Quicksand-Regular.ttf"; };
static const char *fontFaceBold() { return "res/xt/fonts/quicksand/Quicksand-Bold.ttf"; };

int XTStyle::fontId(NVGcontext *vg)
{
    static int fid;
    if (fid < 0)
        fid = InternalFontMgr::get(vg, fontFace());
    return fid;
}

int XTStyle::fontIdBold(NVGcontext *vg)
{
    static int fid;
    if (fid < 0)
        fid = InternalFontMgr::get(vg, fontFaceBold());
    return fid;
}
} // namespace sst::surgext_rack::style
