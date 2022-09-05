#include "SurgeModuleWidgetCommon.hpp"

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
        rack::ui::Menu *menu = new rack::ui::Menu;

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