#include "Surge.hpp"
#include "rack.hpp"

struct SurgeUpgrade : rack::Module
{
};
struct SurgeUpgradeWidget : rack::ModuleWidget {
    typedef SurgeUpgrade M;
    SurgeUpgradeWidget(M *module);
};

SurgeUpgradeWidget::SurgeUpgradeWidget(SurgeUpgradeWidget::M *module)
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 12, RACK_HEIGHT);
    auto s = new rack::SvgWidget();
    s->box.pos = rack::Vec(0,0);
    s->box.size = box.size;
    auto svg = rack::Svg::load(rack::asset::plugin(pluginInstance, "res/SurgeXT-upgrade.svg"));
    if (svg) s->setSvg(svg);
    addChild(s);
}

rack::Model *modelSurgeUpgrade =
    rack::createModel<SurgeUpgradeWidget::M, SurgeUpgradeWidget>("SurgeUpgradeToXT");
