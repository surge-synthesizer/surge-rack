//
// Created by Paul Walker on 8/29/22.
//

#include "SurgeVCF.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"

#include "XTWidgets.h"

namespace sst::surgext_rack::vcf::ui
{
struct SurgeVCFWidget : widgets::XTModuleWidget
{
    typedef SurgeVCF M;
    SurgeVCFWidget(M *module);

    void moduleBackground(NVGcontext *vg) {}
};

SurgeVCFWidget::SurgeVCFWidget(SurgeVCFWidget::M *module) : XTModuleWidget() { setModule(module); }
} // namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeVCF =
    rack::createModel<sst::surgext_rack::vcf::ui::SurgeVCFWidget::M,
                      sst::surgext_rack::vcf::ui::SurgeVCFWidget>("SurgeXTVCF");
