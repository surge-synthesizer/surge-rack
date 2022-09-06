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
    };
    static std::string styleName(Style s);
    static void setCurrentStyle(Style s);
    static void initialize();

    enum Colors
    {
        KNOB_RING,
        KNOB_MOD_PLUS,
        KNOB_MOD_MINUS,
        KNOB_MOD_MARK,

        PLOT_CURVE,
        PLOT_MARKS,

        MOD_BUTTON_LIGHT_ON,
        MOD_BUTTON_LIGHT_OFF,

        TEXT_LABEL,
        TEXT_LABEL_OUTPUT,

        PLOT_CONTROL_TEXT,
        PLOT_CONTROL_VALUE_BG,
        PLOT_CONTROL_VALUE_FG,
    };
    const NVGcolor getColor(Colors c);

    std::string skinAssetDir();
    int fontId(NVGcontext *vg);
    int fontIdBold(NVGcontext *vg);

    friend class StyleParticipant;

  private:
    static std::unordered_set<StyleParticipant *> listeners;
    static void addStyleListener(StyleParticipant *l) { listeners.insert(l); }
    static void removeStyleListener(StyleParticipant *l) { listeners.erase(l); }
    static void notifyStyleListeners();
};

struct StyleParticipant
{
    StyleParticipant();
    ~StyleParticipant();
    virtual void onStyleChanged() = 0;

    const std::shared_ptr<XTStyle> &style();
};

inline StyleParticipant::StyleParticipant() { XTStyle::addStyleListener(this); }
inline StyleParticipant::~StyleParticipant() { XTStyle::removeStyleListener(this); }

} // namespace sst::surgext_rack::style