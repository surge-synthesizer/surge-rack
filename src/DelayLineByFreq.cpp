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
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 6, rack::app::RACK_GRID_HEIGHT);
  
#if 0
    typedef layout::LayoutEngine<DelayLineByFreqWidget, M::VOCT> engine_t;

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 6, rack::app::RACK_GRID_HEIGHT);
    // auto bg = new widgets::Background(box.size, "DelayLineByFreq", "other", "DelayLineByFreq");
    // addChild(bg);

    {
        auto cx = box.size.x * 0.5;
        auto cy = rack::mm2px(30);
        addParam(rack::createParamCentered<widgets::Knob16>(rack::Vec(cx, cy), module, M::VOCT));
        auto bl = cy + rack::mm2px(8 + 4);
        auto lab = widgets::Label::createWithBaselineBox(rack::Vec(0, bl - rack::mm2px(5)),
                                                         rack::Vec(box.size.x, rack::mm2px(5)),
                                                         "V/OCT OFFSET");
        addChild(lab);
    }
    {
        auto cx = box.size.x * 0.5;
        auto cy = rack::mm2px(30 + 16 + 8);
        auto k =
            rack::createParamCentered<widgets::Knob16>(rack::Vec(cx, cy), module, M::CORRECTION);
        addParam(k);

        auto bl = cy + rack::mm2px(8 + 4);
        auto lab = widgets::Label::createWithBaselineBox(rack::Vec(0, bl - rack::mm2px(5)),
                                                         rack::Vec(box.size.x, rack::mm2px(5)),
                                                         "SAMPLE OFFSET");
        addChild(lab);
    }
    float cols[2]{box.size.x * 0.5f - rack::mm2px(7), box.size.x * 0.5f + rack::mm2px(7)};

    {
        auto yp = layout::LayoutConstants::inputRowCenter_MM - 32;
        auto xp = box.size.x * 0.5;
        addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, rack::mm2px(yp)), module,
                                                          M::INPUT_VOCT));
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM - 32;
        auto lab = widgets::Label::createWithBaselineBox(
            rack::Vec(0, rack::mm2px(bl - 5)), rack::Vec(box.size.x, rack::mm2px(5)), "V/OCT");
        addChild(lab);
    }

    int col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[1]);
        auto xp = cols[col];
        addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
        col++;
    }

    col = 0;
    // for (const std::string &s : {std::string("LEFT"), std::string("RIGHT")})
    {
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM - 16;
        auto lab = widgets::Label::createWithBaselineBox(rack::Vec(0, rack::mm2px(bl - 5)),
                                                         rack::Vec(box.size.x, rack::mm2px(5)),
                                                         "LEFT - IN - RIGHT");
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
    // for (const std::string &s : {std::string("LEFT"), std::string("RIGHT")})
    {
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
        auto lab = widgets::Label::createWithBaselineBox(rack::Vec(0, rack::mm2px(bl - 5)),
                                                         rack::Vec(box.size.x, rack::mm2px(5)),
                                                         "LEFT - OUT - RIGHT");
        addChild(lab);
        col++;
    }

    resetStyleCouplingToModule();
#endif
}
} // namespace sst::surgext_rack::delay::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDelayLineByFreq =
    rack::createModel<sst::surgext_rack::delay::ui::DelayLineByFreqWidget::M,
                      sst::surgext_rack::delay::ui::DelayLineByFreqWidget>(
        "SurgeXTDelayLineByFreq");
