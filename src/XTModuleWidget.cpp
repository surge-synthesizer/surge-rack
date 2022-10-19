/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "XTModuleWidget.h"

namespace sst::surgext_rack::widgets
{

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
    auto clight = glob ? style::XTStyle::getGlobalLightColor() : xtm->localLightColor;
    for (int ski = style::XTStyle::LightColor::ORANGE; ski <= style::XTStyle::LightColor::RED;
         ++ski)
    {
        auto sk = (style::XTStyle::LightColor)ski;
        auto m = rack::createMenuItem(style::XTStyle::lightColorName(sk), CHECKMARK(sk == clight),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalLightColor(sk);
                                          else
                                              xtm->localLightColor = sk;
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
    auto clight = glob ? style::XTStyle::getGlobalModLightColor() : xtm->localModLightColor;
    for (int ski = style::XTStyle::LightColor::ORANGE; ski <= style::XTStyle::LightColor::RED;
         ++ski)
    {
        auto sk = (style::XTStyle::LightColor)ski;
        auto m = rack::createMenuItem(style::XTStyle::lightColorName(sk), CHECKMARK(sk == clight),
                                      [xtm, glob, sk]() {
                                          if (glob)
                                              style::XTStyle::setGlobalModLightColor(sk);
                                          else
                                              xtm->localModLightColor = sk;
                                          style::XTStyle::notifyStyleListeners();
                                      });
        p->addChild(m);
    }
}

void XTModuleWidget::appendContextMenu(rack::ui::Menu *menu)
{
    auto xtm = static_cast<modules::XTModule *>(module);
    appendModuleSpecificMenu(menu);
    menu->addChild(new rack::ui::MenuSeparator);
    auto globalItem =
        rack::createMenuItem("Use Global Style", CHECKMARK(module && xtm->isCoupledToGlobalStyle),
                             [this]() { toggleCoupleToGlobalStyle(); });
    menu->addChild(globalItem);
    menu->addChild(rack::createSubmenuItem("Skin", "", [this](auto *x) { skinMenuFor(x, this); }));
    menu->addChild(
        rack::createSubmenuItem("LED Color", "", [this](auto *x) { lightMenuFor(x, this); }));
    menu->addChild(rack::createSubmenuItem("Modulation Color", "",
                                           [this](auto *x) { modLightMenuFor(x, this); }));
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
        xtm->localLightColor = style::XTStyle::getGlobalLightColor();
        xtm->localModLightColor = style::XTStyle::getGlobalModLightColor();
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
                sp->attachTo(&xtm->localStyle, &xtm->localLightColor, &xtm->localModLightColor);
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