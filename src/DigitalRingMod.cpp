/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
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
        rack::Vec(box.size.x, rack::mm2px(layout::LayoutConstants::mainLabelBaseline_MM)), "CXOR",
        layout::LayoutConstants::mainLabelSize_PT);
    titleLabel->tracking = 0.7;
    addChild(titleLabel);

    float cols[2]{box.size.x * 0.5f - rack::mm2px(7), box.size.x * 0.5f + rack::mm2px(7)};

    for (int inst = 0; inst < 2; ++inst)
    {
        auto inOff = inst == 0 ? 0 : 4;
        auto outOff = inst == 0 ? 0 : 2;

        auto yShiftMM = inst == 0 ? -58 : 0;
        auto yShift = rack::mm2px(yShiftMM);

        int col = 0;
        for (auto p : {M::INPUT_0_A_L + inOff, M::INPUT_0_A_R + inOff})
        {
            auto yp = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[0]) + yShift;
            auto xp = cols[col];
            addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
            col++;
        }
        col = 0;
        for (auto p : {M::INPUT_0_B_L + inOff, M::INPUT_0_B_R + inOff})
        {
            auto yp = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[1]) + yShift;
            auto xp = cols[col];
            addInput(rack::createInputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
            col++;
        }

        auto wb2 = rack::mm2px(layout::LayoutConstants::columnWidth_MM) * 0.5;
        col = 0;
        for (const std::string &s : {std::string("B: LEFT"), std::string("B: RIGHT")})
        {
            auto bl = layout::LayoutConstants::inputLabelBaseline_MM - 15 + yShiftMM;
            auto lab = widgets::Label::createWithBaselineBox(
                rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)),
                s);
            addChild(lab);
            col++;
        }

        col = 0;
        for (const std::string &s : {std::string("A: LEFT"), std::string("A: RIGHT")})
        {
            auto bl = layout::LayoutConstants::modulationRowCenters_MM[0] - 6 + yShiftMM;
            auto lab = widgets::Label::createWithBaselineBox(
                rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)),
                s);
            addChild(lab);
            col++;
        }

        {
            auto od = new widgets::OutputDecoration;
            auto bl = layout::LayoutConstants::inputLabelBaseline_MM + yShiftMM;

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
        for (auto p : {M::OUTPUT_0_L + outOff, M::OUTPUT_0_R + outOff})
        {
            auto yp = rack::mm2px(layout::LayoutConstants::inputRowCenter_MM) + yShift;
            auto xp = cols[col];
            addOutput(rack::createOutputCentered<widgets::Port>(rack::Vec(xp, yp), module, p));
            col++;
        }

        col = 0;
        for (const std::string &s : {std::string("LEFT"), std::string("RIGHT")})
        {
            auto bl = layout::LayoutConstants::inputLabelBaseline_MM + yShiftMM;
            auto lab = widgets::Label::createWithBaselineBox(
                rack::Vec(cols[col] - wb2, rack::mm2px(bl - 5)), rack::Vec(2 * wb2, rack::mm2px(5)),
                s, layout::LayoutConstants::labelSize_pt, style::XTStyle::TEXT_LABEL_OUTPUT);
            addChild(lab);
            col++;
        }

        auto ys = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[0]) + yShift - 48;
        auto pxdiff = 5.0;
        auto bglcd = widgets::LCDBackground::createAtYPosition(ys, 17, 6, pxdiff);
        addChild(bglcd);

        auto xpos = rack::mm2px(widgets::LCDBackground::posx_MM) - pxdiff;

        auto wsz = bglcd->box.size;
        wsz.y -= rack::mm2px(2 * widgets::LCDBackground::padY_MM);
        auto wid =
            widgets::PlotAreaMenuItem::create(rack::Vec(xpos, ys), wsz, module, M::TYPE_0 + inst);
        wid->upcaseDisplay = false;
        wid->centerDisplay = true;
        wid->onShowMenu = [wid]() {
            wid->destroyTooltip();
            wid->createContextMenu();
        };
        addChild(wid);

        if (inst == 1)
        {
            auto x = box.size.x / 2;
            auto y = rack::mm2px(layout::LayoutConstants::modulationRowCenters_MM[0]) + yShift;

            auto light = rack::createParamCentered<widgets::ActivateKnobSwitch>(rack::Vec(x, y),
                                                                                module, M::LINK_01);
            addChild(light);
        }
    }
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::digitalrm::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDigitalRingMods =
    rack::createModel<sst::surgext_rack::digitalrm::ui::DigitalRingModWidget::M,
                      sst::surgext_rack::digitalrm::ui::DigitalRingModWidget>(
        "SurgeXTDigitalRingMod");
