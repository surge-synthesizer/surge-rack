//
// Created by Paul Walker on 9/20/22.
//

#include "LFO.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::lfo::ui
{
struct LFOWidget : widgets::XTModuleWidget
{
    typedef lfo::LFO M;
    LFOWidget(M *module);
};

LFOWidget::LFOWidget(LFOWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    int screwWidth = 12;
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * screwWidth, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "LFO", "other", "TotalBlank");
    addChild(bg);
#if 0
    const float columnWidth_MM = 14;
    const float lastRow_MM = 114.5;
    const float modRow_MM = lastRow_MM - (100.16 - 85.32);
    const float firstColumnCenter = 9.48;

    for (int i = 0; i < 4; ++i)
    {
        auto xp = firstColumnCenter;
        auto yp = lastRow_MM - i * 14;
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module,
                                                          M::INPUT_TRIGGER + i));
    }

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        int cc = columnWidth_MM * (i + 1) + firstColumnCenter;
        auto *k = rack::createWidgetCentered<widgets::ModToggleButton>(
            rack::mm2px(rack::Vec(cc, modRow_MM)));
        addChild(k);
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(cc, lastRow_MM)),
                                                          module, M::LFO_MOD_INPUT + i));
    }

    const float knobRowMM = modRow_MM - 22;
    float c0 = firstColumnCenter + columnWidth_MM * 0.5 + 8;
    std::vector<std::string> lab{"RATE", "PHASE", "DEFORM", "AMPLITUDE"};
    for (int i = 0; i < 4; ++i)
    {
        auto ctr = rack::Vec(c0 + (i)*17, knobRowMM);
        auto *k = rack::createParamCentered<widgets::Knob12>(rack::mm2px(ctr), module, i + M::RATE);
        addChild(k);

        ctr.y += 8;
        ctr.x -= 10;
        auto sz = rack::Vec(20, 5);
        auto *r = widgets::Label::createWithBaselineBox(rack::mm2px(ctr), rack::mm2px(sz), lab[i]);
        addChild(r);
    }

    auto lcdbg = widgets::LCDBackground::createWithHeight(knobRowMM - 13, screwWidth);
    addChild(lcdbg);

    float s0 = c0 + 3 * 17 + 6 + 8;
    for (int i = 0; i < 6; ++i)
    {
        auto x = s0 + 7.5 * i;
        auto y = knobRowMM - 8;
        auto w = 5;
        auto h = lastRow_MM - y;
        auto dr =
            widgets::DebugRect::create(rack::mm2px(rack::Vec(x, y)), rack::mm2px(rack::Vec(w, h)));
        addChild(dr);
    }

    for (int i = 0; i < 4; ++i)
    {
        float xp = box.size.x - rack::mm2px(firstColumnCenter);
        float yp = rack::mm2px(lastRow_MM - i * 14);
        addOutput(rack::createOutputCentered<widgets::Port>(rack::Vec(xp, yp), module,
                                                            M::OUTPUT_MIX + i));
    }
#endif

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::lfo::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeLFO = rack::createModel<sst::surgext_rack::lfo::ui::LFOWidget::M,
                                               sst::surgext_rack::lfo::ui::LFOWidget>("SurgeXTLFO");
