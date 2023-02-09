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

    void polyMenu(rack::Menu *p, M *m)
    {
        if (!m)
            return;
        p->addChild(rack::createMenuLabel("Polyphony"));
        p->addChild(new rack::ui::MenuSeparator);
        int cp = m->forcePolyphony;
        p->addChild(rack::createMenuItem("Follow Trigger Input", CHECKMARK(-1 == cp),
                                         [m]() { m->forcePolyphony = -1; }));
        for (int i = 1; i <= 16; ++i)
        {
            p->addChild(rack::createMenuItem(std::to_string(i), CHECKMARK(i == cp),
                                             [m, i]() { m->forcePolyphony = i; }));
        }
    }

    void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
        auto m = static_cast<M *>(module);
        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(
            rack::createSubmenuItem("Polyphony", "", [this, m](auto *x) { polyMenu(x, m); }));

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
    widgets::BufferedDrawFunctionWidget *bdw{nullptr}, *bdwLight{nullptr};
    static QuadWavePicker *create(const rack::Vec &pos, const rack::Vec &size, QuadLFO *module,
                                  int i)
    {
        auto res = new QuadWavePicker;
        res->box.size = size;
        res->box.pos = pos;
        res->module = module;
        res->idx = i;

        res->bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                           [res](auto v) { res->drawBG(v); });
        res->addChild(res->bdw);

        res->bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), size, [res](auto v) { res->drawLight(v); });
        res->addChild(res->bdwLight);

        for (auto b : {QuadLFO::SHAPE_0, QuadLFO::RATE_0, QuadLFO::DEFORM_0, QuadLFO::BIPOLAR_0})
        {
            res->dirtyChecks.push_back(widgets::DirtyHelper<QuadLFO>());
            res->dirtyChecks.back().module = module;
            res->dirtyChecks.back().par = b + res->idx;
            if (b == QuadLFO::RATE_0 || b == QuadLFO::DEFORM_0)
                res->dirtyChecks.back().isModulated = true;
        }
        res->dirtyChecks.push_back(widgets::DirtyHelper<QuadLFO>());
        res->dirtyChecks.back().module = module;
        res->dirtyChecks.back().par = QuadLFO::INTERPLAY_MODE;

        for (int i = 0; i < QuadLFO::n_lfos; ++i)
        {
            for (auto b : {QuadLFO::RATE_0, QuadLFO::DEFORM_0})
            {
                res->spreadDirtyChecks.emplace_back(widgets::DirtyHelper<QuadLFO>());
                res->spreadDirtyChecks.back().module = module;
                res->spreadDirtyChecks.back().par = b + i;
                res->spreadDirtyChecks.back().isModulated = true;
            }
        }

        if (module)
            res->lfo = std::make_unique<QuadLFO::lfoSource_t>(module->storage.get());
        return res;
    }

    float labelHeight{rack::mm2px(4.5)};
    void drawBG(NVGcontext *vg)
    {
        if (!module)
            return;

        if (idx != 3)
        {
            nvgBeginPath(vg);
            nvgMoveTo(vg, box.size.x, 0);
            nvgLineTo(vg, box.size.x, box.size.y - 1);
            nvgStrokeWidth(vg, 0.75);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::Colors::PLOT_MARKS));
            nvgStroke(vg);
        }

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, 0);
        nvgLineTo(vg, box.size.x, 0);
        nvgStrokeWidth(vg, 0.75);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::Colors::PLOT_MARKS));
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, box.size.y - labelHeight);
        nvgLineTo(vg, box.size.x, box.size.y - labelHeight);
        nvgStrokeWidth(vg, 0.75);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::Colors::PLOT_MARKS));
        nvgStroke(vg);

        nvgBeginPath(vg);
        auto txtColor = style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT);
        nvgFillColor(vg, txtColor);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        auto dv = module->paramQuantities[QuadLFO::RATE_0 + idx]->getDisplayValueString();
        dv = temposync_support::abbreviateSurgeTemposyncLabel(dv);
        nvgText(vg, this->box.size.x * 0.5, this->box.size.y - labelHeight * 0.5, dv.c_str(),
                nullptr);

        float pushX = rack::mm2px(0.87);
        float pushY = rack::mm2px(0.7);
        float sz = rack::mm2px(2.5);
        float gapX = rack::mm2px(0.5);
        float gapY = rack::mm2px(0.7);
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
        nvgMoveTo(vg, box.size.x - sz + gapX - pushX, gapY + pushY);
        nvgLineTo(vg, box.size.x - gapX - pushX, gapY + pushY);
        nvgLineTo(vg, box.size.x - sz * 0.5 - pushX, sz - gapY + pushY);
        nvgFill(vg);
        nvgStroke(vg);
    }

    std::unique_ptr<QuadLFO::lfoSource_t> lfo;
    void drawLight(NVGcontext *vg)
    {
        if (!module)
            return;

        auto ip = (int)std::round(module->paramQuantities[QuadLFO::INTERPLAY_MODE]->getValue());

        auto s = (int)std::round(module->paramQuantities[QuadLFO::SHAPE_0 + idx]->getValue());
        // auto r = (int)std::round(module->paramQuantities[QuadLFO::RATE_0 + idx]->getValue());

        /*
         * We want N cycles on display and that sets rate. So
         */
        int cycles = 1;
        bool zeroEndpoints{false};
        if (s == QuadLFO::lfoSource_t::SMOOTH_NOISE || s == QuadLFO::lfoSource_t::SH_NOISE)
            cycles = 4;

        if (s == QuadLFO::lfoSource_t::PULSE || s == QuadLFO::lfoSource_t::RANDOM_TRIGGER)
            zeroEndpoints = true;

        float yshift = rack::mm2px(3.9);
        float ypad = rack::mm2px(2.1);
        float xoff = rack::mm2px(1.5);

        // How many samples do we want
        int oversample = 4;
        float dx = 1.0 / oversample;
        int samples = (int)(box.size.x - 2 * xoff) * oversample; // 4 to let us scale up to 400%

        // so how much time do we need
        auto time = samples * module->storage->samplerate_inv;
        auto freq = 1.0 / time;
        auto r = log2(freq * BLOCK_SIZE_INV * cycles);

        auto d = module->modAssist.values[QuadLFO::DEFORM_0 + idx][0];

        lfo->attackForDisplay(s);
        nvgBeginPath(vg);

        if (idx != 0 && ip == QuadLFO::PHASE_OFFSET)
        {
            auto dph = QuadLFO::RateQuantity::phaseRateScale(
                module->modAssist.values[QuadLFO::RATE_0 + idx][0]);
            lfo->applyPhaseOffset(dph);
        }
        if (idx != 0 && ip == QuadLFO::QUADRATURE)
        {
            lfo->applyPhaseOffset(idx * 0.25);
            lfo->setAmplitude(module->modAssist.values[QuadLFO::RATE_0 + idx][0]);
        }
        if (ip == QuadLFO::SPREAD)
        {
            d = module->spreadDeform(idx, 0);
            auto a = module->spreadAmp(idx, 0);
            auto p = module->spreadPhase(idx, 0);
            lfo->applyPhaseOffset(p);
            lfo->setAmplitude(a);
        }
        lfo->process_block(r, d, s);
        if (zeroEndpoints)
            nvgMoveTo(vg, xoff, (box.size.y - yshift - ypad - labelHeight) * 0.5 + yshift);
        for (int i = 0; i < samples - (zeroEndpoints); ++i)
        {
            lfo->process_block(r, d, s);
            auto v = lfo->outputBlock[0];
            float x = i * dx + xoff;
            if (s == QuadLFO::lfoSource_t::RANDOM_TRIGGER)
            {
                v = ((x - xoff) < (box.size.x - xoff) * 0.25) ? 1 : -1;
            }
            v = (v * 0.5 + 0.5);

            float y = (1 - v) * (box.size.y - yshift - ypad - labelHeight) + yshift;
            if (i == 0 && !zeroEndpoints)
                nvgMoveTo(vg, x, y);
            else
                nvgLineTo(vg, x, y);
        }

        if (zeroEndpoints)
            nvgLineTo(vg, ((samples - zeroEndpoints) * dx) + xoff,
                      (box.size.y - yshift - ypad - labelHeight) * 0.5 + yshift);
        nvgStrokeColor(vg, style()->getColor(style::XTStyle::Colors::PLOT_CURVE));
        nvgStrokeWidth(vg, 1.25);
        nvgStroke(vg);
    }

    std::vector<widgets::DirtyHelper<QuadLFO>> dirtyChecks;
    std::vector<widgets::DirtyHelper<QuadLFO>> spreadDirtyChecks;
    std::string lastLab{"none"};
    void step() override
    {
        if (module)
        {
            auto isD = false;
            for (auto &d : dirtyChecks)
                isD = isD || d.dirty();

            auto ip = (int)std::round(module->paramQuantities[QuadLFO::INTERPLAY_MODE]->getValue());
            if (ip == QuadLFO::SPREAD)
            {
                for (auto &d : spreadDirtyChecks)
                {
                    isD = isD || d.dirty();
                }
            }

            auto dv = module->paramQuantities[QuadLFO::RATE_0 + idx]->getDisplayValueString();
            isD = isD || (dv != lastLab);
            lastLab = dv;
            if (isD)
            {
                bdw->dirty = true;
                bdwLight->dirty = true;
            }
        }
        rack::Widget::step();
    }
    void onButton(const ButtonEvent &e) override
    {
        if (module && e.action == GLFW_PRESS)
        {
            if (e.pos.y > box.size.y - labelHeight)
            {
                if (module->tempoSynced && idx == 0 &&
                    module->paramQuantities[QuadLFO::INTERPLAY_MODE]->getValue() !=
                        QuadLFO::INDEPENDENT)
                {
                    auto min =
                        QuadLFO::RateQuantity::independentRateScale(0) + QuadLFO::temposyncOffset;
                    auto max =
                        QuadLFO::RateQuantity::independentRateScale(1) + QuadLFO::temposyncOffset;

                    auto subm = [min, max, this](auto *menu, float off) {
                        for (float f = min; f <= max; ++f)
                        {
                            auto tsv = f + off;
                            auto l = temposync_support::temposyncLabel(tsv, true);
                            menu->addChild(rack::createMenuItem(l, "", [this, tsv]() {
                                auto v = tsv - QuadLFO::temposyncOffset;
                                module->paramQuantities[QuadLFO::RATE_0]->setValue(
                                    QuadLFO::RateQuantity::independentRateScaleInv(v));
                            }));
                        }
                    };

                    auto menu = rack::createMenu();
                    menu->addChild(rack::createMenuLabel("TempoSync Value"));
                    menu->addChild(new rack::MenuSeparator);
                    menu->addChild(
                        rack::createSubmenuItem("Notes", "", [=](auto *x) { subm(x, 0); }));
                    menu->addChild(rack::createSubmenuItem(
                        "Dotted", "", [=](auto *x) { subm(x, log2(1.333333333)); }));
                    menu->addChild(rack::createSubmenuItem("Triplets", "",
                                                           [=](auto *x) { subm(x, log2(1.5)); }));
                }
                e.consume(this);
                return;
            }

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

                if (index != QuadLFO::lfoSource_t::RANDOM_TRIGGER)
                {
                    menu->addChild(new rack::ui::MenuSeparator);
                    auto bpq = module->paramQuantities[QuadLFO::BIPOLAR_0 + idx];
                    auto bv = bpq->getValue() > 0.5;
                    menu->addChild(rack::createMenuItem("Unipolar (0/10v)", CHECKMARK(!bv),
                                                        [bpq]() { bpq->setValue(0); }));

                    menu->addChild(rack::createMenuItem("Bipolar (-5/+5v)", CHECKMARK(bv),
                                                        [bpq]() { bpq->setValue(1); }));
                }
            }
            e.consume(this);
        }
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwLight->dirty = true;
    }
};
QuadLFOWidget::QuadLFOWidget(sst::surgext_rack::quadlfo::ui::QuadLFOWidget::M *module)
{
    setModule(module);
    typedef layout::LayoutEngine<QuadLFOWidget, M::RATE_0> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * 12, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "QUAD LFO", "other", "FourOuts");
    addChild(bg);

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

          {li_t::PORT, "RESET", M::TRIGGER_0 + i, col, row1},
        };
        // clang-format on

        // todo dynamic labels like layout[n].dynamicLabel=true .dynLabelFn etc
        layout[0].dynamicLabel = true;
        layout[0].dynLabelFn = [i](modules::XTModule *m) -> std::string {
            auto ql = dynamic_cast<QuadLFO *>(m);
            if (ql)
            {
                return ql->getRatePanelLabel(i);
            }
            return {"ERR"};
        };
        layout[1].dynamicLabel = true;
        layout[1].dynLabelFn = [i](modules::XTModule *m) -> std::string {
            auto ql = dynamic_cast<QuadLFO *>(m);
            if (ql)
            {
                return ql->getDeformPanelLabel(i);
            }
            return {"ERR"};
        };
        layout[2].dynamicLabel = true;
        layout[2].dynLabelFn = [i](modules::XTModule *m) -> std::string {
            auto ql = dynamic_cast<QuadLFO *>(m);
            if (ql)
            {
                return ql->getTriggerPanelLabel(i);
            }
            return {"ERR"};
        };

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "QuadLFO");
        }
    }

    {
        auto presetHeight = 4.5;
        auto topx = widgets::LCDBackground::posx;
        auto topy = widgets::LCDBackground::posy;
        auto width = box.size.x - 2 * widgets::LCDBackground::posx;
        auto height = rack::mm2px(row3 - rack::mm2px(2.5)) - widgets::LCDBackground::posy;

        auto preset = new widgets::SteppedParamAsPresetJog;
        preset->box.pos = rack::Vec(topx, topy);
        preset->box.size = rack::Vec(width, rack::mm2px(presetHeight));
        preset->module = module;
        preset->paramId = QuadLFO::INTERPLAY_MODE;
        preset->setup();
        addChild(preset);

        topy += rack::mm2px(presetHeight);
        height -= rack::mm2px(presetHeight);
        auto qw = width / M::n_lfos;
        for (int i = 0; i < M::n_lfos; ++i)
        {
            auto ql = QuadWavePicker::create(rack::Vec(topx + qw * i, topy), rack::Vec(qw, height),
                                             module, i);
            addChild(ql);
        }
    }

    engine_t::addSingleOutputStripBackground(this, 0, 4);

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
