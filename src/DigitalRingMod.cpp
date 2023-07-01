/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "DigitalRingMod.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::digitalrm::ui
{
struct DigitalRingModWidget : widgets::XTModuleWidget
{
    typedef digitalrm::DigitalRingMod M;
    DigitalRingModWidget(M *module);
};

DigitalRingModWidget::DigitalRingModWidget(DigitalRingModWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 6, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "", "other", "blank6hp");
    addChild(bg);

    auto titleLabel = widgets::Label::createWithBaselineBox(
        rack::Vec(0, 0),
        rack::Vec(box.size.x, rack::mm2px(layout::LayoutConstants::mainLabelBaseline_MM)),
        "DIGITAL", layout::LayoutConstants::mainLabelSize_PT);
    titleLabel->tracking = 0.7;
    addChild(titleLabel);

    auto titleLabelLower = widgets::Label::createWithBaselineBox(
        rack::Vec(0, 0),
        rack::Vec(box.size.x, rack::mm2px(layout::LayoutConstants::mainLabelBaseline_MM + 5.2)),
        "RINGMODS", layout::LayoutConstants::mainLabelSize_PT);
    titleLabelLower->tracking = 0.7;
    addChild(titleLabelLower);

    float cols[2]{box.size.x * 0.5f - rack::mm2px(7), box.size.x * 0.5f + rack::mm2px(7)};

    int col = 0;
    for (auto p : {M::INPUT_A_L, M::INPUT_A_R})
    {
        auto yp = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[0]);
        auto xp = cols[col];
        addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
        col++;
    }
    col = 0;
    for (auto p : {M::INPUT_B_L, M::INPUT_B_R})
    {
        auto yp = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[1]);
        auto xp = cols[col];
        addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
        col++;
    }

    auto wb2 = rack::mm2px(layout::LayoutConstants::columnWidth_MM) * 0.5;
    col = 0;
    for (const std::string &s : {std::string("B: LEFT"), std::string("B: RIGHT")})
    {
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM - 15;
        auto lab = widgets::Label::createWithBaselineBox(
            rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)), s);
        addChild(lab);
        col++;
    }

    col = 0;
    for (const std::string &s : {std::string("A: LEFT"), std::string("A: RIGHT")})
    {
        auto bl = layout::LayoutConstants::modulationRowCenters_MM[0] - 6;
        auto lab = widgets::Label::createWithBaselineBox(
            rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)), s);
        addChild(lab);
        col++;
    }

    auto bglcd = widgets::LCDBackground::createWithHeight(60, 6, 39);

    addChild(bglcd);

    {
        auto od = new widgets::OutputDecoration;
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;

        auto pd_MM = 0.5;

        auto c0 = 0, nc = 2;
        od->box.size = rack::Vec(
            rack::mm2px((nc - 0.2) * layout::LayoutConstants::columnWidth_MM + 2 * pd_MM), 42);
        od->box.pos = rack::Vec(
            cols[0] + rack::mm2px((c0 - 0.4) * layout::LayoutConstants::columnWidth_MM - pd_MM),
            rack::mm2px(bl - pd_MM) - 7.2 * 96 / 72);
        od->setup();
        addChild(od);
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
} // namespace sst::surgext_rack::digitalrm::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDigitalRingMods =
    rack::createModel<sst::surgext_rack::digitalrm::ui::DigitalRingModWidget::M,
                      sst::surgext_rack::digitalrm::ui::DigitalRingModWidget>(
        "SurgeXTDigitalRingMod");
