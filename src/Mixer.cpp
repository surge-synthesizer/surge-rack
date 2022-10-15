//
// Created by Paul Walker on 9/20/22.
//

#include "Mixer.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::mixer::ui
{
struct MixerWidget : widgets::XTModuleWidget
{
    typedef mixer::Mixer M;
    MixerWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, M::n_mixer_params> overlays;
    std::array<widgets::KnobN *, M::n_mixer_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

MixerWidget::MixerWidget(MixerWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    typedef layout::LayoutEngine<MixerWidget, M::OSC1_LEV> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "MIXER", "other", "Mixer");
    addChild(bg);

    std::vector<std::string> labels{"OSC1", "OSC2", "OSC3",  "NOISE",
                                    "R1X2", "R2X3", "COLOR", "GAIN"};
    int kr{0}, kc{0};
    for (int i = M::OSC1_LEV; i <= M::GAIN; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - 58 - (1 - kr) * 18;
        auto xc = layout::LayoutConstants::columnCenters_MM[kc];

        auto lay = layout::LayoutItem();
        lay.type = layout::LayoutItem::KNOB9;
        lay.parId = i;
        lay.label = labels[i];
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
        lcd->noModuleText = "Mixer";
    addChild(lcd);
    for (int i = 0; i < 6; ++i)
    {
        std::string lab = "O" + std::to_string(i + 1);
        if (i == 3)
            lab = "N";
        if (i == 4)
            lab = "1X2";
        if (i == 5)
            lab = "2x3";

        auto pm = M::OSC1_MUTE + i;
        auto ps = M::OSC1_SOLO + i;
        auto sxm = widgets::LCDBackground::posx_MM + padlcd;
        auto wm = rack::app::RACK_GRID_WIDTH * 12 * 25.4 / 75 - 2 * sxm;

        auto dx = wm / 6.0;

        auto hm = eolcd - solcd - 2 * padlcd;
        auto dy = hm / 5.0;
        auto y0 = solcd + padlcd;

        auto sz = rack::mm2px(rack::Vec(dx - 0.5, dy + 1));
        auto pLab = rack::mm2px(rack::Vec(sxm + i * dx + 0.25, y0 - 2));
        addChild(widgets::Label::createWithBaselineBox(pLab, sz, lab,
                                                       layout::LayoutConstants::labelSize_pt,
                                                       style::XTStyle::PLOT_CONTROL_TEXT));

        auto pmute = rack::mm2px(rack::Vec(sxm + i * dx + 0.25, y0 + dy - 1));
        addParam(widgets::PlotAreaSwitch::create(pmute, sz, "M", module, pm));

        auto psolo = rack::mm2px(rack::Vec(sxm + i * dx + 0.25, y0 + 2 * dy - 1));
        addParam(widgets::PlotAreaSwitch::create(psolo, sz, "S", module, ps));
    }

    auto portSpacing = layout::LayoutConstants::inputRowCenter_MM -
                       layout::LayoutConstants::modulationRowCenters_MM[1];

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MIXER_MOD_INPUT, -portSpacing);

    kr = 0;
    kc = 0;
    for (int i = M::INPUT_OSC1_L; i <= M::INPUT_OSC3_R; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - (1 - kr) * portSpacing;
        auto xc = layout::LayoutConstants::columnCenters_MM[kc];

        auto lay = layout::LayoutItem();
        lay.type = layout::LayoutItem::PORT;
        lay.parId = i;
        lay.label = "";
        lay.xcmm = xc;
        lay.ycmm = yc;
        engine_t::layoutItem(this, lay, "Mixer");

        if (kc % 2 == 0)
        {
            auto bl = layout::LayoutConstants::inputLabelBaseline_MM - (1 - kr) * portSpacing;
            auto ll =
                std::string("L - OSC") + std::to_string((i - M::INPUT_OSC1_L) / 2 + 1) + " - R";
            auto laylab = engine_t::makeSpanLabelAt(bl, kc, ll, 2);
            this->addChild(laylab);
        }

        kc++;
        if (kc == 4)
        {
            kr++;
            kc = 0;
        }
    }

    auto yp = layout::LayoutConstants::inputRowCenter_MM;
    auto xp = layout::LayoutConstants::columnCenters_MM[2];
    addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module,
                                                        M::OUTPUT_L));

    xp = layout::LayoutConstants::columnCenters_MM[3];
    addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module,
                                                        M::OUTPUT_R));

    auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
    auto laylab =
        engine_t::makeSpanLabelAt(bl, 2, "L - OUT - R", 2, style::XTStyle::TEXT_LABEL_OUTPUT);
    this->addChild(laylab);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::mixer::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeMixer =
    rack::createModel<sst::surgext_rack::mixer::ui::MixerWidget::M,
                      sst::surgext_rack::mixer::ui::MixerWidget>("SurgeXTMixer");
