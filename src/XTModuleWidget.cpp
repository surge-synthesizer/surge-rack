#include "XTModuleWidget.hpp"

namespace sst::surgext_rack::widgets
{

/*
** These are internal only classes
*/

struct SkinSelectItem : rack::ui::MenuItem
{
    style::XTStyle::Style s;
    void onAction(const rack::event::Action &e) override { style::XTStyle::setCurrentStyle(s); }
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

struct LightSelectItem : rack::ui::MenuItem
{
    style::XTStyle::LightColor s;
    void onAction(const rack::event::Action &e) override
    {
        style::XTStyle::setCurrentLightColor(s);
    }
};

struct LightsSubmenuItem : rack::ui::MenuItem
{
    rack::ui::Menu *createChildMenu() override
    {
        auto menu = new rack::ui::Menu;

        for (int sk = style::XTStyle::LightColor::ORANGE; sk <= style::XTStyle::LightColor::RED;
             ++sk)
        {
            auto it = new LightSelectItem;
            it->s = (style::XTStyle::LightColor)sk;
            it->text = style::XTStyle::lightColorName(it->s);
            menu->addChild(it);
        }

        return menu;
    }
};

struct ModLightSelectItem : rack::ui::MenuItem
{
    style::XTStyle::LightColor s;
    void onAction(const rack::event::Action &e) override
    {
        style::XTStyle::setCurrentModLightColor(s);
    }
};

struct ModLightsSubmenuItem : rack::ui::MenuItem
{
    rack::ui::Menu *createChildMenu() override
    {
        auto menu = new rack::ui::Menu;

        for (int sk = style::XTStyle::LightColor::ORANGE; sk <= style::XTStyle::LightColor::RED;
             ++sk)
        {
            auto it = new ModLightSelectItem;
            it->s = (style::XTStyle::LightColor)sk;
            it->text = style::XTStyle::lightColorName(it->s);
            menu->addChild(it);
        }

        return menu;
    }
};

void XTModuleWidget::appendContextMenu(rack::ui::Menu *menu)
{
    menu->addChild(new rack::ui::MenuEntry);

    auto *skins = new SkinsSubmenuItem;
    skins->text = "Skin";
    skins->rightText = RIGHT_ARROW;
    menu->addChild(skins);

    auto *lights = new LightsSubmenuItem;
    lights->text = "LED Colors";
    lights->rightText = RIGHT_ARROW;
    menu->addChild(lights);

    auto *modlights = new ModLightsSubmenuItem;
    modlights->text = "Mod Ring Colors";
    modlights->rightText = RIGHT_ARROW;
    menu->addChild(modlights);
}
} // namespace sst::surgext_rack::widgets