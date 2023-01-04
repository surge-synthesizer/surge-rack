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

#include "Delay.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::delay::ui
{
struct DelayWidget : widgets::XTModuleWidget
{
    typedef delay::Delay M;
    DelayWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_delay_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_delay_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
        addClockMenu<Delay>(menu);

        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(rack::createMenuLabel("Feedback Clipping"));
        addSelectionMenu(menu, module->paramQuantities[M::CLIP_MODE_PARAM],
                         {{"Transparent (Potentially Unbounded)", M::ClipMode::TRANSPARENT},
                          {"Softclip Feedback (Surge Default)", M::ClipMode::SOFTCLIP_FEEDBACK},
                          {"Softclip Input + Feedback", M::ClipMode::SOFTCLIP_FULL_SIGNAL},
                          {"Softclip Output", M::ClipMode::SOFTCLIP_OUTPUT},
                          {"Hardclip Output @ 10V", M::ClipMode::HARDCLIP_OUTPUT}});
    }

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }
};

struct TimeDisplay : rack::widget::TransparentWidget, style::StyleParticipant
{
    Delay *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};

    static TimeDisplay *create(Delay *module)
    {
        auto res = new TimeDisplay();
        res->box.pos.x = rack::mm2px(widgets::LCDBackground::contentPosX_MM);
        res->box.pos.y = rack::mm2px(widgets::LCDBackground::contentPosY_MM);
        res->box.size.x =
            rack::RACK_GRID_WIDTH * 12 - 2 * rack::mm2px(widgets::LCDBackground::contentPosX_MM);
        res->box.size.y = rack::mm2px(4.5);

        res->module = module;
        res->setup();

        return res;
    }

    void setup()
    {
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0),
                                                      rack::Vec(this->box.size.x, this->box.size.y),
                                                      [this](auto *v) { this->drawDelays(v); });
        this->addChild(bdw);
    }

    std::string ll{"1.00 s"}, lr{"0.25 s"};
    void step() override
    {
        if (module)
        {
            auto l = module->paramQuantities[Delay::TIME_L]->getDisplayValueString();
            auto r = module->paramQuantities[Delay::TIME_R]->getDisplayValueString();
            if (ll != l || lr != r)
                bdw->dirty = true;
            ll = l;
            lr = r;
        }
        rack::widget::TransparentWidget::step();
    }
    void drawDelays(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
        nvgMoveTo(vg, box.size.x * 0.5, rack::mm2px(widgets::LCDBackground::padY_MM));
        nvgLineTo(vg, box.size.x * 0.5, box.size.y - rack::mm2px(widgets::LCDBackground::padY_MM));
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        nvgText(vg, 0, box.size.y * 0.5, ll.c_str(), nullptr);

        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        nvgText(vg, box.size.x, box.size.y * 0.5, lr.c_str(), nullptr);
    }

    void onStyleChanged() override { bdw->dirty = true; }
};
DelayWidget::DelayWidget(DelayWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    typedef layout::LayoutEngine<DelayWidget, M::TIME_L, M::INPUT_CLOCK> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "DELAY", "fx", "BlankNoDisplay");
    addChild(bg);

    auto col = std::vector<float>();
    for (int i = 0; i < 4; ++i)
        col.push_back(layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::columnWidth_MM * i);

    const auto row1 = layout::LayoutConstants::rowStart_MM;
    const auto row2 = row1 - layout::LayoutConstants::labeledGap_MM;
    const auto row2S = row2 - layout::LayoutConstants::labeledGap_MM - 2;

    const auto endOfPanel = row2S - 6;

    typedef layout::LayoutItem li_t;
    // fixme use the enums
    // clang-format off
    std::vector<li_t> layout = {
        {li_t::KNOB14, "LEFT", M::TIME_L, layout::LayoutConstants::bigCol0-3, row2S - 2.5f},
        {li_t::KNOB14, "RIGHT", M::TIME_R, layout::LayoutConstants::bigCol1+3, row2S - 2.5f},
        {li_t::KNOB9, "FINE", M::TIME_S, (col[2]+col[1]) * 0.5f, row2S},

        {li_t::KNOB9, "DEPTH", M::FEEDBACK, col[0], row2},
        {li_t::KNOB9, "CROSS", M::CROSSFEED, col[1], row2},
        li_t::createGrouplabel("FEEDBACK", col[0], row2, 2),

        {li_t::KNOB9, "LO", M::LOCUT, col[2], row2},
        {li_t::KNOB9, "HI", M::HICUT, col[3], row2},
        li_t::createGrouplabel("EQ", col[2], row2, 2),

        {li_t::PORT, "CLOCK", M::INPUT_CLOCK, col[0], row1},

        {li_t::KNOB9, "RATE", M::MODRATE, col[1], row1},
        {li_t::KNOB9, "DEPTH", M::MODDEPTH, col[2], row1},
        li_t::createGrouplabel("MOD", col[1], row1, 2),

        {li_t::KNOB9, "MIX", M::MIX, col[3], row1},

        li_t::createPresetLCDArea(),
    };
    // clang-format on

    for (const auto &lay : layout)
    {
        engine_t::layoutItem(this, lay, "Delay");
    }

    auto t = TimeDisplay::create(module);
    addChild(t);

    engine_t::addModulationSection(this, M::n_mod_inputs, M::DELAY_MOD_INPUT);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);
    engine_t::createLeftRightInputLabels(this);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::delay::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeDelay =
    rack::createModel<sst::surgext_rack::delay::ui::DelayWidget::M,
                      sst::surgext_rack::delay::ui::DelayWidget>("SurgeXTDelay");
