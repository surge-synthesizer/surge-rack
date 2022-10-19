//
// Created by Paul Walker on 9/20/22.
//

#include "DelayLineByFreq.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
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

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 6, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "", "other", "blank6hp");
    addChild(bg);

    auto titleLabel = widgets::Label::createWithBaselineBox(
        rack::Vec(0, 0),
        rack::Vec(box.size.x, rack::mm2px(layout::LayoutConstants::mainLabelBaseline_MM)), "TUNED",
        layout::LayoutConstants::mainLabelSize_PT);
    titleLabel->tracking = 0.7;
    addChild(titleLabel);

    auto titleLabelLower = widgets::Label::createWithBaselineBox(
        rack::Vec(0, 0),
        rack::Vec(box.size.x, rack::mm2px(layout::LayoutConstants::mainLabelBaseline_MM + 5.2)),
        "DELAY", layout::LayoutConstants::mainLabelSize_PT);
    titleLabelLower->tracking = 0.7;
    addChild(titleLabelLower);

    {
        auto cx = box.size.x * 0.5;
        auto cy = rack::mm2px(28);
        addParam(rack::createParamCentered<widgets::Knob14>(rack::Vec(cx, cy), module, M::VOCT));
        auto bl = cy + rack::mm2px(7 + 4);
        auto lab = widgets::Label::createWithBaselineBox(rack::Vec(0, bl - rack::mm2px(5)),
                                                         rack::Vec(box.size.x, rack::mm2px(5)),
                                                         "V/OCT OFFSET");
        addChild(lab);
    }
    {
        auto cx = box.size.x * 0.5;
        auto cy = rack::mm2px(28 + 16 + 8);
        auto k =
            rack::createParamCentered<widgets::Knob14>(rack::Vec(cx, cy), module, M::CORRECTION);
        addParam(k);

        auto bl = cy + rack::mm2px(7 + 4);
        auto lab = widgets::Label::createWithBaselineBox(rack::Vec(0, bl - rack::mm2px(5)),
                                                         rack::Vec(box.size.x, rack::mm2px(5)),
                                                         "SAMPLE OFFSET");
        addChild(lab);
    }

    {
        auto yp = layout::LayoutConstants::inputRowCenter_MM - 40;
        auto xp = box.size.x * 0.5;
        addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, rack::mm2px(yp)), module,
                                                          M::INPUT_VOCT));
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM - 26.5;
        auto lab = widgets::Label::createWithBaselineBox(
            rack::Vec(0, rack::mm2px(bl - 5)), rack::Vec(box.size.x, rack::mm2px(5)), "PITCH");
        addChild(lab);
    }

    float cols[2]{box.size.x * 0.5f - rack::mm2px(7), box.size.x * 0.5f + rack::mm2px(7)};

    int col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[1]);
        auto xp = cols[col];
        addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
        col++;
    }

    auto wb2 = rack::mm2px(layout::LayoutConstants::columnWidth_MM) * 0.5;
    col = 0;
    for (const std::string &s : {std::string("LEFT"), std::string("RIGHT")})
    {
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM - 15;
        auto lab = widgets::Label::createWithBaselineBox(
            rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)), s);
        addChild(lab);
        col++;
    }

    col = 0;
    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = rack::mm2px(layout::LayoutConstants::inputRowCenter_MM);
        auto xp = cols[col];
        addOutput(rack::createOutputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
        col++;
    }

    col = 0;
    for (const std::string &s : {std::string("LEFT"), std::string("RIGHT")})
    {
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
        auto lab = widgets::Label::createWithBaselineBox(
            rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)), s,
            layout::LayoutConstants::labelSize_pt, style::XTStyle::TEXT_LABEL_OUTPUT);
        addChild(lab);
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
