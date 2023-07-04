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

#include "ModMatrix.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::modmatrix::ui
{
struct ModMatrixWidget : widgets::XTModuleWidget
{
    typedef modmatrix::ModMatrix M;
    ModMatrixWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_matrix_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_matrix_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }
};

struct MatrixDisplay : rack::Widget, style::StyleParticipant
{
    typedef ModMatrix M;
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};

    ModMatrix *module{nullptr};
    static MatrixDisplay *create(const rack::Vec &pos, const rack::Vec &size, ModMatrix *m)
    {
        auto res = new MatrixDisplay();
        res->box.pos = pos;
        res->box.size = size;
        res->module = m;

        res->bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), size,
                                                           [res](auto vg) { res->drawWidget(vg); });
        res->addChild(res->bdw);

        for (int i = 0; i < M::n_mod_inputs; ++i)
        {
            res->connectedIn[i] = false;
            for (int j = 0; j < M::n_matrix_params; ++j)
            {
                res->connectedOut[i] = false;
                res->depths[i][j] = 0.f;
            }
        }

        return res;
    }

    bool connectedIn[M::n_mod_inputs];
    bool connectedOut[M::n_matrix_params];
    float depths[M::n_mod_inputs][M::n_matrix_params];

    void step() override
    {
        if (!module)
            return;

        bool dirty{false};
        for (int i = 0; i < M::n_mod_inputs; ++i)
        {
            auto c = module->inputs[M::MATRIX_MOD_INPUT + i].isConnected();
            dirty = dirty || (c != connectedIn[i]);
            connectedIn[i] = c;
            for (int j = 0; j < M::n_matrix_params; ++j)
            {
                if (i == 0)
                {
                    auto c = module->outputs[M::OUTPUT_0 + j].isConnected();
                    dirty = dirty || (c == connectedOut[j]);
                    connectedOut[j] = c;
                }
                auto tp = M::modulatorIndexFor(j + M::TARGET0, i);
                auto f = module->params[tp].getValue();
                dirty = dirty || (f != depths[i][j]);
                depths[i][j] = f;
            }
        }

        if (dirty)
            bdw->dirty = true;
    }

    void drawWidget(NVGcontext *vg)
    {
        if (!module)
        {
            return;
        }

        auto dy = box.size.y / (M::n_mod_inputs + 1);
        auto dx = box.size.x / (M::n_matrix_params + 1);

        // column headers
        for (int i = 0; i < M::n_matrix_params; ++i)
        {
            auto p = rack::Vec(dx * (i + 1.5), dy * 0.5);
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            if (connectedOut[i])
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
            else
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));

            nvgText(vg, p.x, p.y, std::to_string(i + 1).c_str(), nullptr);
            nvgFill(vg);
        }

        // row headers
        for (int i = 0; i < M::n_mod_inputs; ++i)
        {
            auto p = rack::Vec(dx * 0.5, dy * (i + 1.5));
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            if (connectedIn[i])
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
            else
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgText(vg, p.x, p.y, std::to_string(i + 1).c_str(), nullptr);
            nvgFill(vg);
        }

        for (int i = 0; i < M::n_mod_inputs; ++i)
        {
            for (int j = 0; j < M::n_matrix_params; ++j)
            {
                if (connectedIn[i] && connectedOut[j])
                {
                    auto d = depths[i][j];
                    if (d == 0)
                    {
                        nvgBeginPath(vg);
                        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
                        nvgRect(vg, (j + 1.5) * dx - 0.25, (i + 1) * dy, 0.5, dy);
                        nvgFill(vg);
                    }
                    else if (d < 0)
                    {
                        nvgBeginPath(vg);
                        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
                        nvgRect(vg, (j + 1.5) * dx + dx * d * 0.5, (i + 1) * dy, -dx * d * 0.5, dy);
                        nvgFill(vg);
                    }
                    else
                    {
                        nvgBeginPath(vg);
                        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
                        nvgRect(vg, (j + 1.5) * dx, (i + 1) * dy, dx * d * 0.5, dy);
                        nvgFill(vg);
                    }
                }

                nvgBeginPath(vg);
                nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
                nvgRect(vg, (j + 1) * dx, (i + 1) * dy, dx, dy);
                nvgStrokeWidth(vg, 0.5);
                nvgStroke(vg);
            }
        }
    }

    void onStyleChanged() override { bdw->dirty = true; }
};

ModMatrixWidget::ModMatrixWidget(ModMatrixWidget::M *module) : XTModuleWidget()
{
    setModule(module);

    typedef layout::LayoutEngine<ModMatrixWidget, M::TARGET0> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "MOD MATRIX", "other", "Matrix");
    addChild(bg);

    int kr{0}, kc{0};
    for (int i = M::TARGET0; i < M::TARGET0 + M::n_matrix_params; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - 58 - (1 - kr) * 18;
        auto xc = layout::LayoutConstants::firstColumnCenter_MM +
                  layout::LayoutConstants::columnWidth_MM * kc;

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
    {
        lcd->noModuleText = "Mod Matrix";
        lcd->noModuleSize = 30;
    }
    addChild(lcd);

    auto sxm = widgets::LCDBackground::posx_MM + padlcd;
    auto wm = rack::app::RACK_GRID_WIDTH * 12 * 25.4 / 75 - 2 * sxm - 1;
    auto hm = eolcd - solcd - 2 * padlcd - 1;

    auto mmd = MatrixDisplay::create(rack::mm2px(rack::Vec(sxm, solcd + padlcd)),
                                     rack::mm2px(rack::Vec(wm, hm)), module);
    addChild(mmd);

    auto portSpacing = layout::LayoutConstants::inputRowCenter_MM -
                       layout::LayoutConstants::modulationRowCenters_MM[1];

    engine_t::addModulationSection(this, M::n_mod_inputs, M::MATRIX_MOD_INPUT, -portSpacing);

    {
        auto od = new widgets::OutputDecoration;
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
        auto pd_MM = 0.5;
        auto nc = 4;
        auto c0 = 0;
        od->box.size = rack::Vec(
            rack::mm2px((nc - 0.2) * layout::LayoutConstants::columnWidth_MM + 2 * pd_MM), 84);
        od->box.pos =
            rack::Vec(rack::mm2px(layout::LayoutConstants::firstColumnCenter_MM +
                                  (c0 - 0.4) * layout::LayoutConstants::columnWidth_MM - pd_MM),
                      rack::mm2px(bl - portSpacing - pd_MM) - 7.2 * 96 / 72);
        od->setup();
        addChild(od);
    }

    kr = 0;
    kc = 0;
    for (int i = M::OUTPUT_0; i < M::OUTPUT_0 + M::n_matrix_params; ++i)
    {
        auto yc = layout::LayoutConstants::inputRowCenter_MM - (1 - kr) * portSpacing;
        auto xc = layout::LayoutConstants::firstColumnCenter_MM +
                  layout::LayoutConstants::columnWidth_MM * kc;

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
