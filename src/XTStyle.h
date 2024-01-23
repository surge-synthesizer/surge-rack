/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
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

#ifndef SURGE_XT_RACK_SRC_XTSTYLE_H
#define SURGE_XT_RACK_SRC_XTSTYLE_H
#include "SurgeXT.h"
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
    Style *activeStyle{nullptr};
    static std::string styleName(Style s);
    static void setGlobalStyle(Style s);
    static Style getGlobalStyle();
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
        RED,
        WHITE // If you change this as last, change the ranges in XTMW and XGS.cpp
    };
    LightColor *activeModulationColor{nullptr}, *activeDisplayRegionColor{nullptr},
        *activeControlValueColor{nullptr}, *activePowerButtonColor{nullptr};

    static void setGlobalDisplayRegionColor(LightColor c);
    static LightColor getGlobalDisplayRegionColor();

    static void setGlobalModulationColor(LightColor c);
    static LightColor getGlobalModulationColor();

    static void setGlobalControlValueColor(LightColor c);
    static LightColor getGlobalControlValueColor();
    static bool getControlValueColorDistinct();
    static void setControlValueColorDistinct(bool b);

    static void setGlobalPowerButtonColor(LightColor c);
    static LightColor getGlobalPowerButtonColor();

    static bool getShowKnobValuesAtRest();
    static void setShowKnobValuesAtRest(bool b);

    static bool getShowModulationAnimationOnKnobs();
    static void setShowModulationAnimationOnKnobs(bool b);

    static bool getShowModulationAnimationOnDisplay();
    static void setShowModulationAnimationOnDisplay(bool b);

    static bool getShowShadows();
    static void setShowShadows(bool b);

    static bool getWaveshaperShowsBothCurves();
    static void setWaveshaperShowsBothCurves(bool b);

    static std::string lightColorName(LightColor c);
    static NVGcolor lightColorColor(LightColor c);

    enum Colors
    {
        KNOB_RING,
        KNOB_MOD_PLUS,
        KNOB_MOD_MINUS,
        KNOB_MOD_MARK,
        KNOB_RING_VALUE,
        SLIDER_RING_VALUE,

        PANEL_RULER,

        PLOT_CURVE,
        PLOT_MARKS,

        MOD_BUTTON_LIGHT_ON,
        MOD_BUTTON_LIGHT_OFF,

        POWER_BUTTON_LIGHT_ON,
        POWER_BUTTON_LIGHT_OFF,

        TEXT_LABEL,
        TEXT_LABEL_OUTPUT,

        PLOT_CONTROL_TEXT,
        PLOT_CONTROL_VALUE_BG,
        PLOT_CONTROL_VALUE_FG,

        LED_PANEL,
        LED_BORDER,
        LED_HIGHLIGHT,

        SHADOW_BASE,
        SHADOW_OVER_GRADSTART,
        SHADOW_OVER_GRADEND,

        OUTPUTBG_START,
        OUTPUTBG_END
    };
    const NVGcolor getColor(Colors c);

    std::string skinAssetDir();
    int fontId(NVGcontext *vg);
    int fontIdBold(NVGcontext *vg);

    friend struct StyleParticipant;
    static void notifyStyleListeners();

  private:
    static std::unordered_set<StyleParticipant *> listeners;
    static void addStyleListener(StyleParticipant *l) { listeners.insert(l); }
    static void removeStyleListener(StyleParticipant *l) { listeners.erase(l); }
    static void updateJSON();
};

struct StyleParticipant
{
    StyleParticipant();
    virtual ~StyleParticipant();
    virtual void onStyleChanged() = 0;

    const std::shared_ptr<XTStyle> &style();

    void attachToGlobalStyle();
    void attachTo(style::XTStyle::Style *, style::XTStyle::LightColor *,
                  style::XTStyle::LightColor *, style::XTStyle::LightColor *,
                  style::XTStyle::LightColor *);

    std::shared_ptr<XTStyle> stylePtr{nullptr};
};

inline StyleParticipant::StyleParticipant() { XTStyle::addStyleListener(this); }
inline StyleParticipant::~StyleParticipant() { XTStyle::removeStyleListener(this); }

} // namespace sst::surgext_rack::style
#endif // SCXT_SRC_XTSTYLE_H
