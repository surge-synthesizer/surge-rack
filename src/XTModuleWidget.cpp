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

#include "XTModuleWidget.h"

namespace sst::surgext_rack::widgets
{

/* fix mingw LTO build */
Label::~Label() {}
ModRingKnob::~ModRingKnob() {}
VerticalSliderModulator::~VerticalSliderModulator() {}

/*
** These are internal only classes
*/

/*
struct SkinSelectItem : rack::ui::MenuItem
{
    style::XTStyle::Style s;
    void onAction(const rack::event::Action &e) override { style::XTStyle::setGlobalStyle(s); }
};

struct SkinsSubmenuItem : rack::ui::MenuItem
{
    rack::ui::Menu *createChildMenu() override
    {
        auto menu = new rack::ui::Menu;

        for (auto sk : {style::XTStyle::Style::DARK, style::XTStyle::Style::MID,
                        style::XTStyle::Style::LIGHT})
        {
            auto it = new SkinSelectItem;
            it->s = sk;
            it->text = style::XTStyle::styleName(sk);
            menu->addChild(it);
        }

        return menu;
    }
};
 */

void skinMenuFor(rack::Menu *p, XTModuleWidget *w)
{
    auto *xtm = static_cast<modules::XTModule *>(w->module);
    if (!xtm)
        return;

    auto glob = xtm->isCoupledToGlobalStyle;
    auto cstyle = glob ? style::XTStyle::getGlobalStyle() : xtm->localStyle;
    for (auto sk :
         {style::XTStyle::Style::DARK, style::XTStyle::Style::MID, style::XTStyle::Style::LIGHT})
    {
        auto m = rack::createMenuItem(style::XTStyle::styleName(sk), CHECKMARK(sk == cstyle),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalStyle(sk);
                                          else
                                              xtm->localStyle = sk;
                                          style::XTStyle::notifyStyleListeners();
                                      });
        p->addChild(m);
    }
}

void lightMenuFor(rack::Menu *p, XTModuleWidget *w)
{
    auto *xtm = static_cast<modules::XTModule *>(w->module);
    if (!xtm)
        return;

    auto glob = xtm->isCoupledToGlobalStyle;
    auto clight =
        glob ? style::XTStyle::getGlobalDisplayRegionColor() : xtm->localDisplayRegionColor;
    for (int ski = style::XTStyle::LightColor::ORANGE; ski <= style::XTStyle::LightColor::WHITE;
         ++ski)
    {
        auto sk = (style::XTStyle::LightColor)ski;
        auto m = rack::createMenuItem(style::XTStyle::lightColorName(sk), CHECKMARK(sk == clight),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalDisplayRegionColor(sk);
                                          else
                                              xtm->localDisplayRegionColor = sk;
                                          style::XTStyle::notifyStyleListeners();
                                      });
        p->addChild(m);
    }
}

void modLightMenuFor(rack::Menu *p, XTModuleWidget *w)
{
    auto *xtm = static_cast<modules::XTModule *>(w->module);
    if (!xtm)
        return;

    auto glob = xtm->isCoupledToGlobalStyle;
    auto clight = glob ? style::XTStyle::getGlobalModulationColor() : xtm->localModulationColor;
    for (int ski = style::XTStyle::LightColor::ORANGE; ski <= style::XTStyle::LightColor::WHITE;
         ++ski)
    {
        auto sk = (style::XTStyle::LightColor)ski;
        auto m = rack::createMenuItem(style::XTStyle::lightColorName(sk), CHECKMARK(sk == clight),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalModulationColor(sk);
                                          else
                                              xtm->localModulationColor = sk;
                                          style::XTStyle::notifyStyleListeners();
                                      });
        p->addChild(m);
    }
}

void knobLightMenuFor(rack::Menu *p, XTModuleWidget *w)
{
    auto *xtm = static_cast<modules::XTModule *>(w->module);
    if (!xtm)
        return;

    p->addChild(rack::createMenuItem(
        "Same as Display Region", CHECKMARK(!style::XTStyle::getControlValueColorDistinct()), []() {
            auto x = style::XTStyle::getControlValueColorDistinct();
            style::XTStyle::setControlValueColorDistinct(!x);
        }));

    if (!style::XTStyle::getControlValueColorDistinct())
    {
        return;
    }
    p->addChild(new rack::ui::MenuSeparator);

    auto glob = xtm->isCoupledToGlobalStyle;
    auto clight = glob ? style::XTStyle::getGlobalControlValueColor() : xtm->localControlValueColor;
    for (int ski = style::XTStyle::LightColor::ORANGE; ski <= style::XTStyle::LightColor::WHITE;
         ++ski)
    {
        auto sk = (style::XTStyle::LightColor)ski;
        auto m = rack::createMenuItem(style::XTStyle::lightColorName(sk), CHECKMARK(sk == clight),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalControlValueColor(sk);
                                          else
                                              xtm->localControlValueColor = sk;
                                          style::XTStyle::notifyStyleListeners();
                                      });
        p->addChild(m);
    }
}

void powerLightMenuFor(rack::Menu *p, XTModuleWidget *w)
{
    auto *xtm = static_cast<modules::XTModule *>(w->module);
    if (!xtm)
        return;

    auto glob = xtm->isCoupledToGlobalStyle;
    auto clight = glob ? style::XTStyle::getGlobalPowerButtonColor() : xtm->localPowerButtonColor;
    for (int ski = style::XTStyle::LightColor::ORANGE; ski <= style::XTStyle::LightColor::WHITE;
         ++ski)
    {
        auto sk = (style::XTStyle::LightColor)ski;
        auto m = rack::createMenuItem(style::XTStyle::lightColorName(sk), CHECKMARK(sk == clight),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalPowerButtonColor(sk);
                                          else
                                              xtm->localPowerButtonColor = sk;
                                          style::XTStyle::notifyStyleListeners();
                                      });
        p->addChild(m);
    }
}

void colorsMenuFor(rack::Menu *menu, XTModuleWidget *w)
{
    menu->addChild(
        rack::createSubmenuItem("Display Area", "", [w](auto *x) { lightMenuFor(x, w); }));
    menu->addChild(rack::createSubmenuItem("Knob and Slider Values", "",
                                           [w](auto *x) { knobLightMenuFor(x, w); }));
    menu->addChild(
        rack::createSubmenuItem("Modulations", "", [w](auto *x) { modLightMenuFor(x, w); }));
    menu->addChild(
        rack::createSubmenuItem("Power Buttons", "", [w](auto *x) { powerLightMenuFor(x, w); }));

    auto shad = style::XTStyle::getShowShadows();
    menu->addChild(rack::createMenuItem("Show Knob Shadows", CHECKMARK(shad),
                                        [shad]() { style::XTStyle::setShowShadows(!shad); }));
}

void valueDisplayMenuFor(rack::Menu *menu, XTModuleWidget *w)
{
    auto addBoolTog = [&](auto name, auto g, auto s) {
        auto v = g();
        menu->addChild(rack::createMenuItem(name, CHECKMARK(v), [=]() { s(!v); }));
    };
    addBoolTog("Knob Value Rings", style::XTStyle::getShowKnobValuesAtRest,
               style::XTStyle::setShowKnobValuesAtRest);
    addBoolTog("Knob Modulation Animations", style::XTStyle::getShowModulationAnimationOnKnobs,
               style::XTStyle::setShowModulationAnimationOnKnobs);
    addBoolTog("Display Modulation Animations", style::XTStyle::getShowModulationAnimationOnDisplay,
               style::XTStyle::setShowModulationAnimationOnDisplay);
}

void XTModuleWidget::appendContextMenu(rack::ui::Menu *menu)
{
    auto xtm = static_cast<modules::XTModule *>(module);
    appendModuleSpecificMenu(menu);
    menu->addChild(new rack::ui::MenuSeparator);
#ifndef USING_CARDINAL_NOT_RACK
    auto globalItem =
        rack::createMenuItem("Use Global Style", CHECKMARK(module && xtm->isCoupledToGlobalStyle),
                             [this]() { toggleCoupleToGlobalStyle(); });
    menu->addChild(globalItem);
    menu->addChild(rack::createSubmenuItem("Skin", "", [this](auto *x) { skinMenuFor(x, this); }));
#endif
    menu->addChild(
        rack::createSubmenuItem("Colors", "", [this](auto *x) { colorsMenuFor(x, this); }));
    menu->addChild(rack::createSubmenuItem("Value Displays", "",
                                           [this](auto *x) { valueDisplayMenuFor(x, this); }));
}

void XTModuleWidget::resetStyleCouplingToModule()
{
    bool couple{true};
    auto xtm = static_cast<modules::XTModule *>(module);

    if (xtm)
        couple = xtm->isCoupledToGlobalStyle;

    if (xtm && couple)
    {
        xtm->localStyle = style::XTStyle::getGlobalStyle();
        xtm->localDisplayRegionColor = style::XTStyle::getGlobalDisplayRegionColor();
        xtm->localModulationColor = style::XTStyle::getGlobalModulationColor();
        xtm->localControlValueColor = style::XTStyle::getGlobalControlValueColor();
        xtm->localPowerButtonColor = style::XTStyle::getGlobalPowerButtonColor();
    }

    std::function<void(rack::Widget *)> rec;
    rec = [xtm, couple, &rec](auto *w) {
        auto sp = dynamic_cast<style::StyleParticipant *>(w);
        if (sp)
        {
            if (couple)
            {
                sp->attachToGlobalStyle();
            }
            else
            {
                sp->attachTo(&xtm->localStyle, &xtm->localDisplayRegionColor,
                             &xtm->localModulationColor, &xtm->localControlValueColor,
                             &xtm->localPowerButtonColor);
            }
        }
        for (auto c : w->children)
        {
            rec(c);
        }
    };
    rec(this);

    style::XTStyle::notifyStyleListeners();
}

void XTModuleWidget::toggleCoupleToGlobalStyle()
{
    if (!module)
        return;
    auto xtm = static_cast<modules::XTModule *>(module);
    xtm->isCoupledToGlobalStyle = !xtm->isCoupledToGlobalStyle;
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::widgets
