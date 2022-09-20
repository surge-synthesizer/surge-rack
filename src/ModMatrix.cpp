//
// Created by Paul Walker on 9/20/22.
//

#include "ModMatrix.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::modmatrix::ui
{
struct ModMatrixWidget : widgets::XTModuleWidget, widgets::VCOVCFConstants
{
    typedef modmatrix::ModMatrix M;
    ModMatrixWidget(M *module);
};

ModMatrixWidget::ModMatrixWidget(ModMatrixWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "ModMatrix", "other", "TotalBlank");
    addChild(bg);
}
} // namespace sst::surgext_rack::modmatrix::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeModMatrix =
    rack::createModel<sst::surgext_rack::modmatrix::ui::ModMatrixWidget::M,
                      sst::surgext_rack::modmatrix::ui::ModMatrixWidget>("SurgeXTModMatrix");
