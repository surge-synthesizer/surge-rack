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

#include "Mixer.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::mixer::ui
{
struct MixerWidget : widgets::XTModuleWidget
{
    typedef mixer::Mixer M;
    MixerWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_mixer_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_mixer_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    virtual void meterChannelMenu(rack::Menu *p, M *m)
    {
        if (!m)
            return;

        for (int c = 0; c < m->polyChannelCount(); ++c)
        {
            p->addChild(rack::createMenuItem(std::string("Channel ") + std::to_string(c + 1),
                                             CHECKMARK(c == m->vuChannel),
                                             [m, c]() { m->vuChannel = c; }));
        }
    }

    virtual void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (!module)
            return;
        auto m = static_cast<M *>(module);

        auto pc = m->polyChannelCount();
        if (pc > 1)
        {
            menu->addChild(new rack::ui::MenuSeparator);
            menu->addChild(rack::createSubmenuItem("Meter Channel", "",
                                                   [this, m](auto *x) { meterChannelMenu(x, m); }));
        }
    }

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }
};

struct VUWidget : rack::TransparentWidget, style::StyleParticipant
{
    Mixer *module{nullptr};
    VUWidget() {}
    static VUWidget *create(rack::Vec pos, rack::Vec size, Mixer *module)
    {
        auto res = new VUWidget();
        res->box.size = size;
        res->box.pos = pos;
        res->module = module;

        return res;
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 0)
            return;
        if (!module)
            return;
        auto vg = args.vg;
        auto ll = module->vuLevel[0];
        auto lr = module->vuLevel[1];

        ll = std::clamp(ll / 6.f, 0.f, 1.f);
        lr = std::clamp(lr / 6.f, 0.f, 1.f);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 1, ll * box.size.x, box.size.y * 0.5 - 2);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, 0, box.size.y * 0.5 + 1, lr * box.size.x, box.size.y * 0.5 - 2);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
        nvgFill(vg);

        int nLines = 24;
        for (int i = 0; i < nLines; ++i)
        {
            float x = box.size.x * i * 1.f / nLines;
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::LED_PANEL));
            nvgStrokeWidth(vg, 0.5);
            nvgMoveTo(vg, x, 0);
            nvgLineTo(vg, x, box.size.y);
            nvgStroke(vg);
        }
    }

    void onStyleChanged() override {}
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

    std::vector<std::string> labels{"IN 1",   "IN 2",   "IN 3",  "NOISE",
                                    "RM 1x2", "RM 2x3", "COLOR", "GAIN"};
    int kr{0}, kc{0};
    for (int i = M::OSC1_LEV; i <= M::GAIN; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - 58 - (1 - kr) * 18;
        auto xc = layout::LayoutConstants::firstColumnCenter_MM +
                  layout::LayoutConstants::columnWidth_MM * kc;

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
    {
        lcd->noModuleText = "Mixer";
        lcd->noModuleSize = 30;
    }
    addChild(lcd);
    if (module)
    {
        for (int i = 0; i < 6; ++i)
        {
            std::string lab = "IN " + std::to_string(i + 1);
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

            if (i == 0)
            {
                auto pvu = rack::mm2px(rack::Vec(sxm + i * dx + 0.25, y0 + 3 * dy + 1));
                auto vusz = rack::mm2px(rack::Vec(wm - 1, 2 * dy - 1));
                auto vu = VUWidget::create(pvu, vusz, module);
                addChild(vu);
            }
        }
    }
    auto portSpacing = layout::LayoutConstants::inputRowCenter_MM -
                       layout::LayoutConstants::modulationRowCenters_MM[1];

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MIXER_MOD_INPUT, -portSpacing);

    kr = 0;
    kc = 0;
    for (int i = M::INPUT_OSC1_L; i <= M::INPUT_OSC3_R; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - (1 - kr) * portSpacing;
        auto xc = layout::LayoutConstants::firstColumnCenter_MM +
                  layout::LayoutConstants::columnWidth_MM * kc;

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
                std::string("L - IN ") + std::to_string((i - M::INPUT_OSC1_L) / 2 + 1) + " - R";
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

    engine_t::addSingleOutputStripBackground(this);

    auto yp = layout::LayoutConstants::inputRowCenter_MM;
    auto xp =
        layout::LayoutConstants::firstColumnCenter_MM + layout::LayoutConstants::columnWidth_MM * 2;
    addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module,
                                                        M::OUTPUT_L));

    xp =
        layout::LayoutConstants::firstColumnCenter_MM + layout::LayoutConstants::columnWidth_MM * 3;
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
