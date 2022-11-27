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

#include "EGxVCA.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "SurgeXT.h"

namespace sst::surgext_rack::egxvca::ui
{
struct EGxVCAWidget : public widgets::XTModuleWidget
{
    typedef sst::surgext_rack::egxvca::EGxVCA M;
    EGxVCAWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, n_fx_params> overlays;
    std::array<widgets::ModulatableKnob *, n_fx_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;

        menu->addChild(new rack::ui::MenuSeparator);
        /*
         * Clock entries
         */
        auto xtm = static_cast<M *>(module);

        typedef modules::ClockProcessor<EGxVCA> cp_t;
        menu->addChild(new rack::ui::MenuSeparator);
        auto t = xtm->clockProc.clockStyle;
        menu->addChild(
            rack::createMenuItem("Clock in QuarterNotes", CHECKMARK(t == cp_t::QUARTER_NOTE),
                                 [xtm]() { xtm->clockProc.clockStyle = cp_t::QUARTER_NOTE; }));

        menu->addChild(
            rack::createMenuItem("Clock in BPM CV", CHECKMARK(t == cp_t::BPM_VOCT),
                                 [xtm]() { xtm->clockProc.clockStyle = cp_t::BPM_VOCT; }));
    }
};

struct EnvCurveWidget : rack::Widget, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwCurve{nullptr};
    EGxVCA *module{nullptr};
    EnvCurveWidget(const rack::Vec &pos, const rack::Vec &size, EGxVCA *md)
    {
        box.pos = pos;
        box.size = size;
        module = md;

        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                      [this](auto vg) { drawBg(vg); });
        addChild(bdw);

        bdwCurve = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), size, [this](auto vg) { drawCurve(vg); });
        addChild(bdwCurve);
    }

    void drawBg(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgStrokeColor(vg, nvgRGB(255, 0, 0));
        nvgStroke(vg);
    }
    void drawCurve(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(255, 0, 0));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 10);

        nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, "Curve Soon", nullptr);
        nvgStroke(vg);
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwCurve->dirty = true;
    }
};

struct ResponseMeterWidget : rack::Widget, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwCurve{nullptr};
    EGxVCA *module{nullptr};
    ResponseMeterWidget(const rack::Vec &pos, const rack::Vec &size, EGxVCA *md)
    {
        box.pos = pos;
        box.size = size;
        module = md;

        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                      [this](auto vg) { drawBg(vg); });
        addChild(bdw);

        bdwCurve = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), size, [this](auto vg) { drawCurve(vg); });
        addChild(bdwCurve);
    }

    void drawBg(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgStrokeColor(vg, nvgRGB(0, 255, 0));
        nvgStroke(vg);
    }
    void drawCurve(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(0, 255, 0));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 10);

        nvgText(vg, box.size.x * 0.5, box.size.y * 0.4, "Meters", nullptr);
        nvgText(vg, box.size.x * 0.5, box.size.y * 0.6, "Soon", nullptr);
        nvgStroke(vg);
    }

    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwCurve->dirty = true;
    }
};

EGxVCAWidget::EGxVCAWidget(sst::surgext_rack::egxvca::ui::EGxVCAWidget::M *module)
{
    setModule(module);
    typedef layout::LayoutEngine<EGxVCAWidget, EGxVCAWidget::M::LEVEL, EGxVCAWidget::M::CLOCK_IN>
        engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "EG x VCA", "fx", "BlankNoDisplay");
    addChild(bg);
    bg->addAlpha();

    auto col = std::vector<float>();
    for (int i = 0; i < 4; ++i)
        col.push_back(layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::columnWidth_MM * i);

    const auto col0 = layout::LayoutConstants::firstColumnCenter_MM;
    const auto col1 =
        layout::LayoutConstants::firstColumnCenter_MM + layout::LayoutConstants::columnWidth_MM;
    const auto col2 =
        layout::LayoutConstants::firstColumnCenter_MM + 2 * layout::LayoutConstants::columnWidth_MM;
    const auto col3 =
        layout::LayoutConstants::firstColumnCenter_MM + 3 * layout::LayoutConstants::columnWidth_MM;

    const auto dSlider = layout::LayoutConstants::columnWidth_MM * 0.5f;

    const auto sliderStart = col1 + layout::LayoutConstants::columnWidth_MM * 0.25f;
    const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1];
    const auto row2 = layout::LayoutConstants::vcoRowCenters_MM[0];
    const auto row3 = layout::LayoutConstants::vcoRowCenters_MM[0] - (row1 - row2);
    const auto rowS = (row2 + row3) * 0.5f;

    typedef layout::LayoutItem li_t;
    // fixme use the enums
    // clang-format off
    std::vector<li_t> layout = {
        {li_t::KNOB12, "LEVEL", M::LEVEL, col0, rowS},
        {li_t::KNOB9, "PAN", M::PAN, col1, row3},
        {li_t::KNOB9, "RESP", M::RESPONSE, col1, row2},

        {li_t::PORT, "GATE", M::GATE_IN, col0, row1},
        {li_t::PORT, "RETRIG", M::RETRIG_IN, col1, row1},
        {li_t::PORT, "CLOCK", M::CLOCK_IN, col2, row1},
        {li_t::OUT_PORT, "ENV", M::ENV_OUT, col3, row1},

        {li_t::VSLIDER_25, "A", M::EG_A, sliderStart + 1.f * dSlider, rowS},
        {li_t::VSLIDER_25, "D", M::EG_D, sliderStart + 2.f * dSlider, rowS},
        {li_t::VSLIDER_25, "S", M::EG_S, sliderStart + 3.f * dSlider, rowS},
        {li_t::VSLIDER_25, "R", M::EG_R, sliderStart + 4.f * dSlider, rowS},

        li_t::createLCDArea(row3 - rack::mm2px(2.5))
    };
    // clang-format on

    for (const auto &lay : layout)
    {
        engine_t::layoutItem(this, lay, "EGxVCA");
    }

    auto posx = widgets::LCDBackground::posx;
    {
        auto adp =
            rack::Vec(rack::app::RACK_GRID_WIDTH * 6, rack::mm2px(widgets::LCDBackground::posy_MM));
        auto ads = rack::Vec((rack::app::RACK_GRID_WIDTH * 12 - 2 * posx) * 0.5 - rack::mm2px(1.5),
                             rack::mm2px(6));
        auto andig = widgets::PlotAreaToggleClick::create(adp, ads, module, M::ANALOG_OR_DIGITAL);
        addChild(andig);
    }
    {
        auto adp = rack::Vec(posx + rack::mm2px(1.5), rack::mm2px(widgets::LCDBackground::posy_MM));
        auto ads = rack::Vec((rack::app::RACK_GRID_WIDTH * 12 - 2 * posx) * 0.5 - rack::mm2px(1.5),
                             rack::mm2px(6));
        auto mode = widgets::PlotAreaToggleClick::create(adp, ads, module, M::ADSR_OR_DAHD);
        mode->alignLeft = true;
        addChild(mode);
    }

    auto respWidth_MM = 14;
    auto posyCurve = rack::mm2px(widgets::LCDBackground::posy_MM + 5);
    auto w = rack::app::RACK_GRID_WIDTH * 12 - 2 * posx - rack::mm2px(respWidth_MM + 1);
    auto h = row3 - rack::mm2px(1.5 + 4) + widgets::LCDBackground::posy;
    auto envc =
        new EnvCurveWidget(rack::Vec(posx + rack::mm2px(1), posyCurve), rack::Vec(w, h), module);
    addChild(envc);

    auto respc =
        new ResponseMeterWidget(rack::Vec(posx + rack::mm2px(1) + w + rack::mm2px(0.5), posyCurve),
                                rack::Vec(rack::mm2px(respWidth_MM), h), module);
    addChild(respc);

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);
    engine_t::createLeftRightInputLabels(this);
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::egxvca::ui

rack::Model *modelEGxVCA =
    rack::createModel<sst::surgext_rack::egxvca::ui::EGxVCAWidget::M,
                      sst::surgext_rack::egxvca::ui::EGxVCAWidget>("SurgeXTEGxVCA");
