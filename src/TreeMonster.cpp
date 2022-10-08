//
// Created by Paul Walker on 9/20/22.
//

#include "TreeMonster.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::treemonster::ui
{
struct TreeMonsterWidget : widgets::XTModuleWidget
{
    typedef treemonster::TreeMonster M;
    TreeMonsterWidget(M *module);
};

TreeMonsterWidget::TreeMonsterWidget(TreeMonsterWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "TreeMonster", "other", "TotalBlank");
    addChild(bg);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::treemonster::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeTreeMonster =
    rack::createModel<sst::surgext_rack::treemonster::ui::TreeMonsterWidget::M,
                      sst::surgext_rack::treemonster::ui::TreeMonsterWidget>("SurgeXTTreeMonster");
