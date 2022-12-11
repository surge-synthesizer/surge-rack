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

#include "QuadAD.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "SurgeXT.h"

namespace sst::surgext_rack::quadad::ui
{
struct QuadADWidget : public widgets::XTModuleWidget
{
    typedef sst::surgext_rack::quadad::QuadAD M;
    QuadADWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_mod_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_mod_params> underKnobs;
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

        auto xtm = static_cast<M *>(module);
        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(rack::createMenuItem("Attack Starts from Zero",
                                            CHECKMARK(xtm->attackFromZero),
                                            [xtm]() { xtm->attackFromZero = true; }));

        menu->addChild(rack::createMenuItem("Attack Starts from Current",
                                            CHECKMARK(!xtm->attackFromZero),
                                            [xtm]() { xtm->attackFromZero = false; }));
    }
};

struct ThreeStateTriggerSwitch : rack::app::Switch, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    float radius;

    ThreeStateTriggerSwitch()
    {
        box.size = rack::mm2px(rack::Vec(3.5, 3.5));
        radius = rack::mm2px(1.1);
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto v) { this->drawBackground(v); });
        bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto v) { this->drawLight(v); });
        addChild(bdw);
        addChild(bdwLight);
    }

    bool hovered{false};
    void onHover(const HoverEvent &e) override
    {
        e.consume(this);
        rack::app::Switch::onHover(e);
    }
    void onEnter(const EnterEvent &e) override
    {
        hovered = true;
        bdw->dirty = true;
        e.consume(this);

        rack::app::Switch::onEnter(e);
    }
    void onLeave(const LeaveEvent &e) override
    {
        hovered = false;
        bdw->dirty = true;
        e.consume(this);

        rack::app::Switch::onLeave(e);
    }

    int val()
    {
        if (!getParamQuantity())
            return 0;
        return std::round(getParamQuantity()->getValue());
    }

    void drawBackground(NVGcontext *vg)
    {
        auto state = val();
        if (state == 0)
        {
            auto col = style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_OFF);
            if (hovered)
            {
                col.r *= 1.2;
                col.g *= 1.2;
                col.b *= 1.2;
            }

            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
            nvgFillColor(vg, col);
            nvgEllipse(vg, box.size.x * 0.5, box.size.y * 0.5, radius, radius);
            nvgFill(vg);
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }
        else
        {
        }
    }

    void drawLight(NVGcontext *vg)
    {

        const float halo = rack::settings::haloBrightness;
        auto state = val();

        if (state == 1)
        {
            auto col = style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_ON);

            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
            nvgFillColor(vg, col);

            auto cx = box.size.x * 0.5;
            auto cy = box.size.y * 0.5;

            auto r = radius * 1.25;
            nvgMoveTo(vg, cx - r, cy - r);
            nvgLineTo(vg, cx + r, cy);
            nvgLineTo(vg, cx - r, cy + r);

            nvgFill(vg);
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }
        else if (state == -1)
        {
            auto col = style()->getColor(style::XTStyle::POWER_BUTTON_LIGHT_ON);

            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PANEL_RULER));
            nvgFillColor(vg, col);

            auto cx = box.size.x * 0.5;
            auto cy = box.size.y * 0.5;

            auto r = radius * 1.25;
            nvgMoveTo(vg, cx + r, cy - r);
            nvgLineTo(vg, cx - r, cy);
            nvgLineTo(vg, cx + r, cy + r);

            nvgFill(vg);
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }
    }

    void onChange(const ChangeEvent &e) override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
        Widget::onChange(e);
    }

    float phalo{0.f};
    void step() override
    {
        const float halo = rack::settings::haloBrightness;
        if (phalo != halo)
        {
            phalo = halo;
            bdw->dirty = true;
            bdwLight->dirty = true;
        }
        Switch::step();
    }
    void onStyleChanged() override {}
};

QuadADWidget::QuadADWidget(sst::surgext_rack::quadad::ui::QuadADWidget::M *module)
{
    setModule(module);
    typedef layout::LayoutEngine<QuadADWidget, M::ATTACK_0> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "QUAD AD", "other", "FourOuts");
    addChild(bg);
    bg->addAlpha();

    /*auto portSpacing = layout::LayoutConstants::inputRowCenter_MM -
                       layout::LayoutConstants::modulationRowCenters_MM[1];*/
    auto portSpacing = 0.f;

    const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1] - portSpacing;
    const auto row2 = layout::LayoutConstants::vcoRowCenters_MM[0] - portSpacing;
    const auto row3 = layout::LayoutConstants::vcoRowCenters_MM[0] - portSpacing - (row1 - row2);

    typedef layout::LayoutItem li_t;
    engine_t::layoutItem(this, li_t::createLCDArea(row3 - rack::mm2px(2.5)), "QUAD AD");

    for (int i = 0; i < M::n_ads; ++i)
    {
        const auto col = layout::LayoutConstants::firstColumnCenter_MM +
                         i * layout::LayoutConstants::columnWidth_MM;
        // fixme use the enums
        // clang-format off
        std::vector<li_t> layout = {
          {li_t::KNOB9, "ATTACK", M::ATTACK_0 + i, col, row3},
          {li_t::KNOB9, "DECAY", M::DECAY_0 + i, col, row2},

          {li_t::PORT, "TRIG", M::TRIGGER_0 + i, col, row1},
        };
        // clang-format on
        layout[2].dynamicLabel = true;
        layout[2].dynLabelFn = [i](modules::XTModule *m) -> std::string {
            if (!m)
                return {"TRIG"};
            auto xtm = static_cast<M *>(m);
            auto isg = xtm->params[M::ADAR_0 + i].getValue() > 0.5;
            if (isg)
                return {"GATE"};
            return {"TRIG"};
        };

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "QuadVCA");
        }

        auto cp = rack::mm2px(rack::Vec(col + layout::LayoutConstants::columnWidth_MM * 0.5, row1));
        auto trigLight =
            rack::createParamCentered<ThreeStateTriggerSwitch>(cp, module, M::LINK_TRIGGER_0 + i);
        addChild(trigLight);

        auto lcdw = rack::app::RACK_GRID_WIDTH * 12 - widgets::LCDBackground::posx * 2;
        auto w = lcdw / 4.0;

        {
            auto yAD = rack::mm2px(widgets::LCDBackground::posy_MM - 1.2);
            auto x = widgets::LCDBackground::posx + w * i;
            auto h = rack::mm2px(5);
            auto mode = widgets::PlotAreaToggleClick::create(rack::Vec(x, yAD), rack::Vec(w, h),
                                                             module, M::MODE_0 + i);
            mode->align = widgets::PlotAreaToggleClick::CENTER;
            addChild(mode);
        }
        {
            auto yAD = rack::mm2px(widgets::LCDBackground::posy_MM - 1.2 + 5);
            auto x = widgets::LCDBackground::posx + w * i;
            auto h = rack::mm2px(5);
            auto mode = widgets::PlotAreaToggleClick::create(rack::Vec(x, yAD), rack::Vec(w, h),
                                                             module, M::ADAR_0 + i);
            mode->align = widgets::PlotAreaToggleClick::CENTER;
            addChild(mode);
        }

        {
            auto yAD = rack::mm2px(row3) - rack::mm2px(13);
            auto x = widgets::LCDBackground::posx + w * i;
            auto lw = w * 0.5;
            auto h = rack::mm2px(5.5);
            auto A = widgets::PlotAreaToggleClick::create(rack::Vec(x, yAD), rack::Vec(lw, h),
                                                          module, M::A_SHAPE_0 + i);
            A->align = widgets::PlotAreaToggleClick::CENTER;
            addChild(A);
            auto D = widgets::PlotAreaToggleClick::create(rack::Vec(x + lw, yAD), rack::Vec(lw, h),
                                                          module, M::D_SHAPE_0 + i);
            D->align = widgets::PlotAreaToggleClick::CENTER;
            addChild(D);
        }
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0, -portSpacing);

    int kc = 0;
    for (int i = M::OUTPUT_0; i < M::OUTPUT_0 + M::n_ads; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM;
        auto xc = layout::LayoutConstants::firstColumnCenter_MM +
                  layout::LayoutConstants::columnWidth_MM * kc;

        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xc, yc)), module, i));

        if (i != M::OUTPUT_0 + M::n_ads - 1)
        {
            auto cp =
                rack::mm2px(rack::Vec(xc + layout::LayoutConstants::columnWidth_MM * 0.5, yc));
            auto sumLight = rack::createParamCentered<widgets::ActivateKnobSwitch>(
                cp, module, M::LINK_ENV_0 + i);
            addChild(sumLight);
        }
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
        auto lab = engine_t::makeLabelAt(bl, kc, "ENV" + std::to_string(i - M::OUTPUT_0 + 1),
                                         style::XTStyle::TEXT_LABEL_OUTPUT);
        addChild(lab);
        kc++;
    }
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::quadad::ui
// namespace sst::surgext_rack::quadad::ui

rack::Model *modelQuadAD =
    rack::createModel<sst::surgext_rack::quadad::ui::QuadADWidget::M,
                      sst::surgext_rack::quadad::ui::QuadADWidget>("SurgeXTQuadAD");
