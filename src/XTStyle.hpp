/*
** Functions for shared and consistent sytling, including colors, drawing
** gradients, and drawing panel backgrounds
*/
#pragma once
#include "SurgeXT.hpp"
#include "rack.hpp"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace logger = rack::logger;

namespace sst::surgext_rack::style
{

struct StyleParticipant;

struct XTStyle
{
    enum Style
    {
        DARK = 10001, // just so it's not a 0 in the JSON
        MID,
        LIGHT
    } *
        activeStyle{nullptr};
    static std::string styleName(Style s);
    static void setGlobalStyle(Style s);
    static void initialize();

    enum LightColor
    {
        ORANGE = 900001, // must be first
        YELLOW,
        GREEN,
        AQUA,
        BLUE,
        PURPLE,
        PINK,
        RED // must be last
    } *
        activeModLight{nullptr},
        *activeLight{nullptr};

    static std::string lightColorName(LightColor c);
    static void setGlobalLightColor(LightColor c);
    static void setGlobalModLightColor(LightColor c);
    static NVGcolor lightColorColor(LightColor c);

    enum Colors
    {
        KNOB_RING,
        KNOB_MOD_PLUS,
        KNOB_MOD_MINUS,
        KNOB_MOD_MARK,
        KNOB_RING_VALUE,

        PLOT_CURVE,
        PLOT_MARKS,

        MOD_BUTTON_LIGHT_ON,
        MOD_BUTTON_LIGHT_OFF,

        TEXT_LABEL,
        TEXT_LABEL_OUTPUT,

        PLOT_CONTROL_TEXT,
        PLOT_CONTROL_VALUE_BG,
        PLOT_CONTROL_VALUE_FG,

        LED_PANEL,
        LED_BORDER,
        LED_HIGHLIGHT
    };
    const NVGcolor getColor(Colors c);

    std::string skinAssetDir();
    int fontId(NVGcontext *vg);
    int fontIdBold(NVGcontext *vg);

    friend struct StyleParticipant;

  private:
    static std::unordered_set<StyleParticipant *> listeners;
    static void addStyleListener(StyleParticipant *l) { listeners.insert(l); }
    static void removeStyleListener(StyleParticipant *l) { listeners.erase(l); }
    static void notifyStyleListeners();
    static void updateJSON();
};

struct StyleParticipant
{
    StyleParticipant();
    ~StyleParticipant();
    virtual void onStyleChanged() = 0;

    const std::shared_ptr<XTStyle> &style();

    std::shared_ptr<XTStyle> stylePtr{nullptr};
};

inline StyleParticipant::StyleParticipant() { XTStyle::addStyleListener(this); }
inline StyleParticipant::~StyleParticipant() { XTStyle::removeStyleListener(this); }

} // namespace sst::surgext_rack::style