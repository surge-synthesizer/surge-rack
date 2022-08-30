#include "SurgeStyle.hpp"
#include "filesystem/import.h"
#include "rack.hpp"
#include "tinyxml/tinyxml.h"
#include "window/Svg.hpp"

namespace logger = rack::logger;
using rack::appGet;

int SurgeStyle::fid = -1;
int SurgeStyle::fidcond = -1;

/*
** These are purposefully not exposed
*/
namespace SurgeInternal
{

typedef std::shared_ptr<rack::Svg> svg_t;

static svg_t surgeLogoWhite = nullptr;
static svg_t getSurgeLogo()
{
    if (surgeLogoWhite == nullptr)
    {
        surgeLogoWhite =
            APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/newSurgeLogo.svg"));
    }
    return surgeLogoWhite;
}
} // namespace SurgeInternal

std::unordered_set<SurgeStyle::StyleListener *> SurgeStyle::listeners;
std::unordered_map<std::string, NVGcolor> SurgeStyle::colorMap;
std::unordered_map<std::string, std::string> SurgeStyle::assets;
std::string SurgeStyle::currentStyle = "";
std::vector<std::string> SurgeStyle::styleList;

void SurgeStyle::drawBlueIORect(NVGcontext *vg, float x0, float y0, float w, float h, int direction)
{
    nvgBeginPath(vg);
    NVGpaint vGradient;
    vGradient = nvgLinearGradient(vg, x0, y0, x0, y0 + h, ioRegionBackgroundGradientStart(),
                                  ioRegionBackgroundGradientEnd());

    nvgRoundedRect(vg, x0, y0, w, h, 5);
    nvgFillPaint(vg, vGradient);
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0 + 1, w, h - 1, 5);
    nvgStrokeColor(vg, ioRegionBorderHighlight());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    nvgStrokeColor(vg, ioRegionBorder());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
    nvgBeginPath(vg);
}

void SurgeStyle::drawTextBGRect(NVGcontext *vg, float x0, float y0, float w, float h)
{
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    NVGpaint gradient = nvgLinearGradient(vg, x0, y0, x0, y0 + h, SurgeStyle::textBGGradientStart(),
                                          SurgeStyle::textBGGradientEnd());
    nvgFillPaint(vg, gradient);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h - 1, 5);
    nvgStrokeColor(vg, SurgeStyle::textBGBorderHighlight());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x0, y0, w, h, 5);
    nvgStrokeColor(vg, SurgeStyle::textBGBorder());
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
}

void SurgeStyle::drawPanelBackground(NVGcontext *vg, float w, float h, std::string displayName,
                                     bool narrowMode)
{
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, w, h);
    nvgFillColor(vg, SurgeStyle::panelBackground());
    nvgFill(vg);

    auto logoSvg = SurgeInternal::getSurgeLogo();

    auto hn = logoSvg->handle;
    auto pt = panelTitle();
    for (auto s = hn->shapes; s; s = s->next)
    {
        s->fill.color = (255 << 24) + (((int)(pt.b * 255)) << 16) + (((int)(pt.g * 255)) << 8) +
                        (int)(pt.r * 255);
    }
    float logoX0 = w / 2;

    if (narrowMode)
    {
        logoX0 = 2;
    }

    float logoWidth = 10;
    if (logoSvg && logoSvg->handle)
    {
        // We want the logo to be screw width - 4  high
        float scaleFactor = 1.0 * (SCREW_WIDTH - 4) / logoSvg->handle->height;
        float x0 = logoX0 - (narrowMode ? 0 : logoSvg->handle->width * scaleFactor / 2);
        logoWidth = logoSvg->handle->width * scaleFactor;
        nvgSave(vg);
        nvgTranslate(vg, x0, 2);
        nvgScale(vg, scaleFactor, scaleFactor);
        // FIXME - we want to replace the color in this logo with the panelTitle
        // color
        rack::svgDraw(vg, logoSvg->handle);
        nvgRestore(vg);
    }

    if (!narrowMode)
    {
        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 14);
        nvgFillColor(vg, SurgeStyle::panelTitle());
        nvgText(vg, logoX0 - logoWidth / 2 - 3, 0, "SurgeXT", NULL);
    }

    nvgBeginPath(vg);
    nvgTextAlign(vg,
                 (narrowMode ? NVG_ALIGN_RIGHT | NVG_ALIGN_TOP : NVG_ALIGN_LEFT | NVG_ALIGN_TOP));
    nvgFontFaceId(vg, fontId(vg));
    nvgFontSize(vg, 14);
    nvgFillColor(vg, SurgeStyle::panelTitle());
    nvgText(vg, (narrowMode ? w - 2 : logoX0 + logoWidth / 2 + 3), 0, displayName.c_str(), NULL);
}

#if !defined(TINYXML_SAFE_TO_ELEMENT)
#define TINYXML_SAFE_TO_ELEMENT(expr) ((expr) ? (expr)->ToElement() : NULL)
#endif

void SurgeStyle::loadStyle(std::string styleXml)
{
    // FIXME - factor this all out a bit
    std::string defaultsDir = rack::asset::user("SurgeRack/");
    if (!rack::system::isDirectory(defaultsDir))
        rack::system::createDirectory(defaultsDir);
    std::string defaultsFile = rack::asset::user("SurgeRack/default-skin.json");

    if (styleXml == "" && currentStyle == "")
    {
        INFO("Loading default style");
        json_error_t error;
        json_t *fd = json_load_file(defaultsFile.c_str(), 0, &error);
        if (!fd)
        {
            styleXml = rack::asset::plugin(pluginInstance, "res/skins/Classic.xml");
        }
        else
        {
            json_t *defj = json_object_get(fd, "defaultSkin");
            if (defj)
                styleXml = json_string_value(defj);
            INFO("styleXML is now %s", styleXml.c_str());
            json_decref(fd);
        }
    }
    else if (styleXml == "") // implicit and currentStyle != ""
    {
        // I want to use the default; and I have a current style; so I'm all
        // fine
        return;
    }

    if (styleList.empty())
    {
        INFO("Loading styles");
        std::string dir = rack::asset::plugin(pluginInstance, "res/skins");
        for (auto &d : fs::directory_iterator(fs::path(dir)))
        {
            styleList.push_back(d.path().generic_string().c_str() + dir.length() +
                                1); // +1 for trailing /
        }
    }

    if (currentStyle == styleXml)
        return;

    currentStyle = styleXml;

    INFO("Loading skin '%s'", currentStyle.c_str());

    TiXmlDocument doc;
    if (!doc.LoadFile(styleXml.c_str()) || doc.Error())
    {
        styleXml = rack::asset::plugin(pluginInstance, "res/skins/Classic.xml");
        doc.LoadFile(styleXml.c_str());
    }
    TiXmlElement *skin = TINYXML_SAFE_TO_ELEMENT(doc.FirstChild("surge-rack-skin"));
    if (skin == nullptr)
    {
        WARN("Unable to find surge-rack-skin in file '%s'", currentStyle.c_str());
        return;
    }

    TiXmlElement *cols = TINYXML_SAFE_TO_ELEMENT(skin->FirstChild("colors"));
    if (cols)
    {
        colorMap.clear();
        for (auto child = cols->FirstChild(); child; child = child->NextSibling())
        {
            auto *lkid = TINYXML_SAFE_TO_ELEMENT(child);
            if (lkid && strcmp(lkid->Value(), "color") == 0)
            {
                auto name = lkid->Attribute("name");
                auto hex = lkid->Attribute("hex");
                if (name && hex)
                {
                    int r, g, b;
                    sscanf(hex + 1, "%02x%02x%02x", &r, &g, &b);
                    colorMap[name] = nvgRGB(r, g, b);
                }
            }
        }
    }

    TiXmlElement *asxml = TINYXML_SAFE_TO_ELEMENT(skin->FirstChild("assets"));
    if (asxml)
    {
        INFO("Found Assets");
        assets.clear();
        for (auto child = asxml->FirstChild(); child; child = child->NextSibling())
        {
            auto *lkid = TINYXML_SAFE_TO_ELEMENT(child);
            if (lkid && strcmp(lkid->Value(), "asset") == 0)
            {
                auto name = lkid->Attribute("name");
                auto path = lkid->Attribute("path");
                if (name && path)
                {
                    assets[name] = path;
                }
            }
        }
    }

    // Update the default file
    json_t *rootJ = json_object();
    json_t *stJ = json_string(styleXml.c_str());
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
