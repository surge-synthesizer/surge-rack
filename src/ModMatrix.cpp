//
// Created by Paul Walker on 9/20/22.
//

#include "ModMatrix.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::modmatrix::ui
{
struct ModMatrixWidget : widgets::XTModuleWidget
{
    typedef modmatrix::ModMatrix M;
    ModMatrixWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, M::n_matrix_params> overlays;
    std::array<widgets::KnobN *, M::n_matrix_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

ModMatrixWidget::ModMatrixWidget(ModMatrixWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    typedef layout::LayoutEngine<ModMatrixWidget, M::TARGET0> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "MOD MATRIX", "other", "Mixer");
    addChild(bg);

    int kr{0}, kc{0};
    for (int i = M::TARGET0; i < M::TARGET0 + M::n_matrix_params; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - 58 - (1 - kr) * 18;
        auto xc = layout::LayoutConstants::columnCenters_MM[kc];

        auto lay = layout::LayoutItem();
        lay.type = layout::LayoutItem::KNOB9;
        lay.parId = i;
        lay.label = std::to_string(i - M::TARGET0 + 1);
        lay.xcmm = xc;
        lay.ycmm = yc;

        kc++;
        if (kc == 4)
        {
            kr++;
            kc = 0;
        }

        engine_t::layoutItem(this, lay, "Mixer");
    }

    auto solcd = widgets::LCDBackground::posy_MM;
    auto eolcd = layout::LayoutConstants::inputRowCenter_MM - 58 - 18 - 8;
    auto padlcd = 1;
    auto lcd = widgets::LCDBackground::createWithHeight(eolcd);
    if (!module)
        lcd->noModuleText = "Mod Matrix";
    addChild(lcd);

    auto portSpacing = layout::LayoutConstants::inputRowCenter_MM -
                       layout::LayoutConstants::modulationRowCenters_MM[1];

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MATRIX_MOD_INPUT, -portSpacing);

    kr = 0;
    kc = 0;
    for (int i = M::OUTPUT_0; i < M::OUTPUT_0 + M::n_matrix_params; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - (1 - kr) * portSpacing;
        auto xc = layout::LayoutConstants::columnCenters_MM[kc];

        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xc, yc)), module, i));

        auto bl = layout::LayoutConstants::inputLabelBaseline_MM - (1 - kr) * portSpacing;
        auto lab = engine_t::makeLabelAt(bl, kc, std::to_string(i - M::TARGET0 + 1),
                                         style::XTStyle::TEXT_LABEL_OUTPUT);
        addChild(lab);
        kc++;
        if (kc == 4)
        {
            kr++;
            kc = 0;
        }
    }
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::modmatrix::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeModMatrix =
    rack::createModel<sst::surgext_rack::modmatrix::ui::ModMatrixWidget::M,
                      sst::surgext_rack::modmatrix::ui::ModMatrixWidget>("SurgeXTModMatrix");
