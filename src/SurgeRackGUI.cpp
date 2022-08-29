#include "SurgeRackGUI.hpp"

std::map<std::string, int> InternalFontMgr::fontMap;

/*
** These are internal only classes
*/

struct SkinSelectItem : rack::ui::MenuItem
{
    std::string skin;
    void onAction(const rack::event::Action &e) override
    {
        std::string dir = rack::asset::plugin(pluginInstance, std::string("res/skins/") + skin);
        SurgeStyle::loadStyle(dir);
    }
};

struct SkinsSubmenuItem : rack::ui::MenuItem
{
    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu *menu = new rack::ui::Menu;

        for (auto sk : SurgeStyle::styleList)
        {
            auto it = new SkinSelectItem;
            it->skin = sk;
            it->text = sk;
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
