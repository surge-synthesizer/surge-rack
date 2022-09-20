//
// Created by Paul Walker on 9/20/22.
//

#include "Mixer.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::mixer::ui
{
struct MixerWidget : widgets::XTModuleWidget, widgets::VCOVCFConstants
{
    typedef mixer::Mixer M;
    MixerWidget(M *module);
};

MixerWidget::MixerWidget(MixerWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "Mixer", "other", "TotalBlank");
    addChild(bg);
}
} // namespace sst::surgext_rack::mixer::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeMixer =
    rack::createModel<sst::surgext_rack::mixer::ui::MixerWidget::M,
                      sst::surgext_rack::mixer::ui::MixerWidget>("SurgeXTMixer");
