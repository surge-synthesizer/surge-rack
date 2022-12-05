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

#include "QuadLFO.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "SurgeXT.h"

namespace sst::surgext_rack::quadlfo::ui
{
struct QuadLFOWidget : public widgets::XTModuleWidget
{
    typedef sst::surgext_rack::quadlfo::QuadLFO M;
    QuadLFOWidget(M *module);

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
        addClockMenu<QuadLFO>(menu);
    }
};

struct QuadWavePicker : rack::Widget, style::StyleParticipant
{
    QuadLFO *module{nullptr};
    int idx{0};
    static QuadWavePicker *create(const rack::Vec &pos, const rack::Vec &size, QuadLFO *module,
                                  int i)
    {
        auto res = new QuadWavePicker;
        res->box.size = size;
        res->box.pos = pos;
        res->module = module;
        res->idx = i;

        return res;
    }

    float up{rack::mm2px(4)};

    void draw(const DrawArgs &args) override
    {
        if (!module)
            return;

        // FIXME - hack to quick merge
        auto vg = args.vg;
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        auto uni = module->paramQuantities[QuadLFO::BIPOLAR_0 + idx]->getValue() < 0.5;
        auto shp = module->paramQuantities[QuadLFO::SHAPE_0 + idx]->getDisplayValueString();

        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 7.3 * 96 / 72);
        nvgText(vg, up * 0.5, up * 0.5, uni ? "0+" : "+/-", nullptr);
        nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, shp.c_str(), nullptr);
    }

    void onButton(const ButtonEvent &e) override
    {
        if (module && e.action == GLFW_PRESS)
        {
            if (e.pos.x < up && e.pos.y < up)
            {
                auto pq = module->paramQuantities[QuadLFO::BIPOLAR_0 + idx];
                pq->setValue(!pq->getValue());
            }
            else
            {
                auto pq = module->paramQuantities[QuadLFO::SHAPE_0 + idx];
                auto *sq = dynamic_cast<rack::engine::SwitchQuantity *>(pq);

                if (sq)
                {

                    auto menu = rack::createMenu();
                    menu->addChild(rack::createMenuLabel("Shape"));
                    menu->addChild(new rack::ui::MenuSeparator);
                    float minValue = pq->getMinValue();
                    int index = (int)std::floor(pq->getValue() - minValue);
                    int numStates = sq->labels.size();
                    for (int i = 0; i < numStates; i++)
                    {
                        std::string label = sq->labels[i];
                        menu->addChild(rack::createMenuItem(label, CHECKMARK(index == i),
                                                            [pq, i]() { pq->setValue(i); }));
                    }
                }
            }
            e.consume(this);
        }
    }
    void onStyleChanged() override {}
};
QuadLFOWidget::QuadLFOWidget(sst::surgext_rack::quadlfo::ui::QuadLFOWidget::M *module)
{
    setModule(module);
    typedef layout::LayoutEngine<QuadLFOWidget, M::RATE_0, M::CLOCK_IN> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "QUAD LFO", "other", "FourOuts");
    addChild(bg);
    bg->addAlpha();

    auto portSpacing = 0.f;

    const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1] - portSpacing;
    const auto row2 = layout::LayoutConstants::vcoRowCenters_MM[0] - portSpacing;
    const auto row3 = layout::LayoutConstants::vcoRowCenters_MM[0] - portSpacing - (row1 - row2);

    typedef layout::LayoutItem li_t;
    engine_t::layoutItem(this, li_t::createLCDArea(row3 - rack::mm2px(2.5)), "QUAD LFO");

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MOD_INPUT_0);

    for (int i = 0; i < M::n_lfos; ++i)
    {
        const auto col = layout::LayoutConstants::firstColumnCenter_MM +
                         i * layout::LayoutConstants::columnWidth_MM;
        // fixme use the enums
        // clang-format off
        std::vector<li_t> layout = {
          {li_t::KNOB9, "RATE", M::RATE_0 + i, col, row3},
          {li_t::KNOB9, "DEFORM", M::DEFORM_0 + i, col, row2},

          {li_t::PORT, "TRIG", M::TRIGGER_0 + i, col, row1},
        };
        // clang-format on

        // todo dynamic labels like layout[n].dynamicLabel=true .dynLabelFn etc

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "QuadLFO");
        }
    }

    {
        auto presetHeight = 5.5;
        auto labelHeight = 5.0;
        auto topx = widgets::LCDBackground::posx;
        auto topy = widgets::LCDBackground::posy;
        auto width = box.size.x - 2 * widgets::LCDBackground::posx;
        auto height = rack::mm2px(row3 - rack::mm2px(2.5)) - widgets::LCDBackground::posy;
        auto preset = widgets::DebugRect::create(rack::Vec(topx, topy),
                                                 rack::Vec(width, rack::mm2px(presetHeight)));
        preset->fill = nvgRGBA(255, 0, 255, 120);
        addChild(preset);

        topy += rack::mm2px(presetHeight);
        height -= rack::mm2px(presetHeight);
        auto qw = width / M::n_lfos;
        for (int i = 0; i < M::n_lfos; ++i)
        {
            auto bh = height - rack::mm2px(labelHeight);
            auto ql = QuadWavePicker::create(rack::Vec(topx + qw * i, topy), rack::Vec(qw, bh),
                                             module, i);
            addChild(ql);

            auto ll = widgets::DebugRect::create(rack::Vec(topx + qw * i, topy + bh),
                                                 rack::Vec(qw, rack::mm2px(labelHeight)));
            ll->fill = nvgRGBA(255, 0, 120, 120);
            addChild(ll);
        }
    }

    int kc = 0;
    for (int i = M::OUTPUT_0; i < M::OUTPUT_0 + M::n_lfos; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM;
        auto xc = layout::LayoutConstants::firstColumnCenter_MM +
                  layout::LayoutConstants::columnWidth_MM * kc;

        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xc, yc)), module, i));

        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
        auto lab = engine_t::makeLabelAt(bl, kc, "LFO" + std::to_string(i - M::OUTPUT_0 + 1),
                                         style::XTStyle::TEXT_LABEL_OUTPUT);
        addChild(lab);
        kc++;
    }
    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::quadlfo::ui

rack::Model *modelQuadLFO =
    rack::createModel<sst::surgext_rack::quadlfo::ui::QuadLFOWidget::M,
                      sst::surgext_rack::quadlfo::ui::QuadLFOWidget>("SurgeXTQuadLFO");
