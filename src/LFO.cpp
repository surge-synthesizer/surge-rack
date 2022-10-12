//
// Created by Paul Walker on 9/20/22.
//

#include "LFO.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::lfo::ui
{
struct LFOWidget : widgets::XTModuleWidget
{
    typedef lfo::LFO M;
    LFOWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, M::n_lfo_params> overlays;
    std::array<widgets::KnobN *, M::n_lfo_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

LFOWidget::LFOWidget(LFOWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    typedef layout::LayoutEngine<LFOWidget, M::RATE> engine_t;
    engine_t::initializeModulationToBlank(this);

    int screwWidth = 21;
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * screwWidth, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "LFO", "other", "LFO");
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

    {
        const auto &col = layout::LayoutConstants::columnCenters_MM;
        const auto row1 = layout::LayoutConstants::rowStart_MM - 6;
        typedef layout::LayoutItem li_t;
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::KNOB9, "RATE", M::RATE, col[0], row1},
                {li_t::KNOB9, "PHASE", M::PHASE, col[1], row1},
                {li_t::KNOB9, "DEFORM", M::DEFORM, col[2], row1},
                {li_t::KNOB9, "AMP", M::AMPLITUDE, col[3], row1},
                li_t::createGrouplabel("WAVEFORM", col[0], row1, 4)
         };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }
    }

    {
        const auto row1 = layout::LayoutConstants::rowStart_MM - 6;
        const auto row2 = layout::LayoutConstants::rowStart_MM + 10;
        float cw = layout::LayoutConstants::columnWidth_MM;
        float c0 = layout::LayoutConstants::columnCenters_MM[3] + cw;
        typedef layout::LayoutItem li_t;
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::KNOB9, "DELAY", M::E_DECAY, c0, row1},
                {li_t::KNOB9, "ATTACK", M::E_ATTACK, c0 + cw, row1},
                {li_t::KNOB9, "HOLD", M::E_HOLD, c0 + 2 * cw, row1},
                {li_t::KNOB9, "DECAY", M::E_DECAY, c0, row2},
                {li_t::KNOB9, "SUSTAIN", M::E_SUSTAIN, c0 + cw, row2},
                {li_t::KNOB9, "RELEASE", M::E_RELEASE, c0 + 2*cw, row2},
                li_t::createGrouplabel("ENVELOPE", c0, row1, 3)
         };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIG", "CLK RATE", "CLK ENV", "PHASE"};
        for (auto p :
             {M::INPUT_TRIGGER, M::INPUT_CLOCK_RATE, M::INPUT_CLOCK_ENV, M::INPUT_PHASE_DIRECT})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::columnCenters_MM[col];
            addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                              module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto lab = engine_t::makeLabelAt(bl, col, labv[col], style::XTStyle::TEXT_LABEL);
            addChild(lab);

            col++;
        }
    }

    auto lcd =
        widgets::LCDBackground::createWithHeight(layout::LayoutConstants::rowStart_MM - 19, 21);
    addChild(lcd);

    {
        int col = 0;
        std::vector<std::string> labv{"OUTLFO", "OUTENV", "OUTWAV"};
        for (auto p : {M::OUTPUT_MIX, M::OUTPUT_ENV, M::OUTPUT_WAVE})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::columnCenters_MM[3] +
                      layout::LayoutConstants::columnWidth_MM * (col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto cx = xp;

            auto boxx0 = cx - layout::LayoutConstants::columnWidth_MM * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::columnWidth_MM, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, labv[col]);
            addChild(lab);

            col++;
        }
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIGP", "TRIGF", "TRIGA"};
        for (auto p : {M::OUTPUT_TRIGPHASE, M::OUTPUT_TRIGF, M::OUTPUT_TRIGA})
        {
            auto yp = layout::LayoutConstants::modulationRowCenters_MM[1];
            auto xp = layout::LayoutConstants::columnCenters_MM[3] +
                      layout::LayoutConstants::columnWidth_MM * (col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = yp - 5;
            auto cx = xp;

            auto boxx0 = cx - layout::LayoutConstants::columnWidth_MM * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::columnWidth_MM, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, labv[col]);
            addChild(lab);

            col++;
        }
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::LFO_MOD_INPUT);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::lfo::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeLFO = rack::createModel<sst::surgext_rack::lfo::ui::LFOWidget::M,
                                               sst::surgext_rack::lfo::ui::LFOWidget>("SurgeXTLFO");
