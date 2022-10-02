//
// Created by Paul Walker on 9/20/22.
//

#include "DelayLineByFreq.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::delay::ui
{
struct DelayLineByFreqWidget : widgets::XTModuleWidget,
                               widgets::StandardWidthWithModulationConstants
{
    typedef delay::DelayLineByFreq M;
    DelayLineByFreqWidget(M *module);
};

DelayLineByFreqWidget::DelayLineByFreqWidget(DelayLineByFreqWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "DelayLineByFreq", "other", "TotalBlank");
    addChild(bg);

    int col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = inputRowCenter_MM;
        auto xp = columnCenters_MM[col];
        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    col = 0;
    for (const std::string &s : {"LEFT", "RIGHT", "LEFT", "RIGHT"})
    {
        addChild(makeIORowLabel(col, s, col < 2));
        col++;
    }

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::delay::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDelayLineByFreq =
    rack::createModel<sst::surgext_rack::delay::ui::DelayLineByFreqWidget::M,
                      sst::surgext_rack::delay::ui::DelayLineByFreqWidget>(
        "SurgeXTDelayLineByFreq");
