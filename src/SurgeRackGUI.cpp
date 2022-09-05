#include "SurgeRackGUI.hpp"

namespace sst::surgext_rack::widgets
{

/*
** These are internal only classes
*/

struct SkinSelectItem : rack::ui::MenuItem
{
    std::string skin;
    void onAction(const rack::event::Action &e) override
    {
        std::string dir = rack::asset::plugin(pluginInstance, std::string("res/skins/") + skin);
        style::SurgeStyle::loadStyle(dir);
    }
};

struct SkinsSubmenuItem : rack::ui::MenuItem
{
    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu *menu = new rack::ui::Menu;

        for (auto sk : style::SurgeStyle::styleList)
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
} // namespace sst::surgext_rack::widgets