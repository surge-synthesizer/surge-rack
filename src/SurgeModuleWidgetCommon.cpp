#include "SurgeModuleWidgetCommon.hpp"

namespace sst::surgext_rack::widgets
{

/*
** These are internal only classes
*/

struct SkinSelectItem : rack::ui::MenuItem
{
    style::SurgeStyle::Style s;
    void onAction(const rack::event::Action &e) override { style::SurgeStyle::setCurrentStyle(s); }
};

struct SkinsSubmenuItem : rack::ui::MenuItem
{
    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu *menu = new rack::ui::Menu;

        for (auto sk : {style::SurgeStyle::Style::DARK, style::SurgeStyle::Style::MID,
                        style::SurgeStyle::Style::LIGHT})
        {
            auto it = new SkinSelectItem;
            it->s = sk;
            it->text = style::SurgeStyle::styleName(sk);
            menu->addChild(it);
        }

        return menu;
    }
};

void SurgeModuleWidgetCommon::appendContextMenu(rack::ui::Menu *menu)
{
    menu->addChild(new rack::ui::MenuEntry);

    SkinsSubmenuItem *skins = new SkinsSubmenuItem;
    skins->text = "Skins";
    skins->rightText = RIGHT_ARROW;
    menu->addChild(skins);

    INFO("Append Context Menu");
}
} // namespace sst::surgext_rack::widgets