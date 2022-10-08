//
// Created by Paul Walker on 9/20/22.
//

#include "DelayLineByFreq.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::delay::ui
{
struct DelayLineByFreqWidget : widgets::XTModuleWidget
{
    typedef delay::DelayLineByFreq M;
    DelayLineByFreqWidget(M *module);
};

DelayLineByFreqWidget::DelayLineByFreqWidget(DelayLineByFreqWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    typedef layout::LayoutEngine<DelayLineByFreqWidget, M::FREQUENCY> engine_t;
    // engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "DelayLineByFreq", "other", "TotalBlank");
    addChild(bg);

    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);
    engine_t::createLeftRightInputLabels(this);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::delay::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDelayLineByFreq =
    rack::createModel<sst::surgext_rack::delay::ui::DelayLineByFreqWidget::M,
                      sst::surgext_rack::delay::ui::DelayLineByFreqWidget>(
        "SurgeXTDelayLineByFreq");
