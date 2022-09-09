#include "VCO.hpp"
#include "VCOConfig.hpp"
#include "XTWidgets.h"

#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"

namespace sst::surgext_rack::vco::ui
{
template <int oscType> struct VCOWidget : public  widgets::XTModuleWidget,
                        widgets::VCOVCFConstants
{
    typedef VCO<oscType> M;
    VCOWidget(M *module);

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, 8> overlays;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

template <int oscType>
struct OSCPlotWidget : public rack::widget::TransparentWidget, style::StyleParticipant
{
    typename VCOWidget<oscType>::M *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwPlot{nullptr};
    void setup(typename VCOWidget<oscType>::M *m)
    {
        module = m;
        if (module)
        {
            storage = module->storage.get();
            oscdata = &(storage->getPatch().scene[0].osc[1]); // this is tne no-mod storage
        }
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0,0), box.size, [this](auto *vg) { drawPlotBackground(vg);});
        addChild(bdw);

        bdwPlot = new widgets::BufferedDrawFunctionWidgetOnLayer(rack::Vec(0,0), box.size, [this](auto *vg) { drawPlot(vg);});
        addChild(bdwPlot);
    }

    void step() override
    {
        if (!module)
            return;

        if (isDirty())
        {
            recalcPath();
            bdwPlot->dirty = true;
        }
        rack::widget::Widget::step();
    }

    virtual void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwPlot->dirty = true;
    }

    static OSCPlotWidget<oscType> *create(rack::Vec pos, rack::Vec size,
                                          typename VCOWidget<oscType>::M *module)
    {
        auto *res = rack::createWidget<OSCPlotWidget<oscType>>(pos);

        res->box.pos = pos;
        res->box.size = size;

        res->setup(module);

        return res;
    }

    bool firstDirty{false};
    int dirtyCount{0};
    int sumDeact{-1};
    bool isDirty()
    {
        if (!firstDirty)
        {
            firstDirty = true;
            return true;
        }

        bool dval{false};
        if (module)
        {
            auto lSumDeact = 0;
            for (int i = 0; i < n_osc_params; i++)
            {
                dval = dval || (tp[oscdata->p[i].param_id_in_scene].i != oscdata->p[i].val.i);
                lSumDeact += oscdata->p[i].deactivated * ( 1 << i );
            }

            if (lSumDeact != sumDeact)
            {
                sumDeact = lSumDeact;
                dval = true;
            }
        }
        return dval;
    }

    pdata tp[n_scene_params];
    OscillatorStorage *oscdata{nullptr};
    SurgeStorage *storage{nullptr};
    unsigned char oscbuffer alignas(16)[oscillator_buffer_size];

    Oscillator *setupOscillator()
    {
        tp[oscdata->pitch.param_id_in_scene].f = 0;

        for (int i = 0; i < n_osc_params; i++)
        {
            tp[oscdata->p[i].param_id_in_scene].i = oscdata->p[i].val.i;
        }

        auto res = spawn_osc(oscdata->type.val.i, storage, oscdata, tp, oscbuffer);
        res->init_ctrltypes();
        return res;
    }

    std::vector<std::pair<float, float>> oscPath;
    void recalcPath()
    {
        auto xp = box.size.x;
        auto yp = box.size.y;

        oscPath.clear();
        auto osc = setupOscillator();
        const float ups = 3.0, invups = 1.0 / ups;

        float disp_pitch_rs =
            12.f * std::log2f((700.f * (storage->samplerate / 48000.f)) / 440.f) + 69.f;

        osc->init(disp_pitch_rs, true, true);

        int block_pos{BLOCK_SIZE_OS + 1};
        for (int i = 0; i < xp * ups; ++i)
        {
            if (block_pos >= BLOCK_SIZE_OS)
            {
                osc->process_block(disp_pitch_rs);
                block_pos = 0;
            }

            float yc = (-osc->output[block_pos] * 0.47 + 0.5) * yp;
            oscPath.emplace_back(i * invups, yc);
            block_pos++;
        }

        osc->~Oscillator();
    }

    void drawPlotBackground(NVGcontext *vg)
    {
        // This will go in layer 0
        int nSteps = 9;
        int mid = (nSteps - 1) / 2;
        float dy = box.size.y * 1.f / (nSteps-1);

        float nX = std::ceil(box.size.x / dy);
        float dx = box.size.x / nX;

        auto markCol = style()->getColor(style::XTStyle::PLOT_MARKS);
        for (int yd = 0; yd < nSteps; yd++)
        {
            if (yd == mid)
                continue;
            float y = yd * dy;
            float x = 0;

            while (x <= box.size.x)
            {
                nvgBeginPath(vg);
                nvgFillColor(vg, markCol);
                nvgEllipse(vg, x, y, 0.5, 0.5);
                nvgFill(vg);
                x += dx;
            }
        }
        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, box.size.y * 0.5);
        nvgLineTo(vg, box.size.x, box.size.y * 0.5);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, box.size.y);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);


        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, 0);
        nvgLineTo(vg, box.size.x, 0);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
    }

    void drawPlot(NVGcontext *vg)
    {
        if (!oscPath.empty())
        {
            nvgSave(vg);
            nvgScissor(vg, 0, 0.5, box.size.x, box.size.y-1);
            nvgBeginPath(vg);
            bool first{true};
            for (const auto &[x, y] : oscPath)
            {
                if (first)
                {
                    nvgMoveTo(vg, x, y);
                }
                else
                {
                    nvgLineTo(vg, x, y);
                }
                first = false;
            }
            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            nvgStrokeColor(vg, col);
            nvgStrokeWidth(vg, 1.25);
            nvgStroke(vg);

            col.a = 0.1;
            nvgStrokeColor(vg, col);
            nvgStrokeWidth(vg, 3);
            nvgStroke(vg);
            nvgRestore(vg);
        }

        if (!module)
        {
            // Draw the module name here for preview goodness
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, 30);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgText(vg, box.size.x * 0.5,
                    box.size.y * 0.5,
                    osc_type_names[oscType], nullptr);
        }
    }
};

template <int oscType>
VCOWidget<oscType>::VCOWidget(VCOWidget<oscType>::M *module)
    : XTModuleWidget()
{
    setModule(module);

    for (auto &ob : overlays)
        for (auto &o : ob)
            o = nullptr;

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);

    std::string panelLabel = std::string(M::name) + " VCO";
    for (auto &q : panelLabel)
        q = std::toupper(q);

    auto bg = new widgets::Background(box.size, panelLabel, "vco", "BlankVCO");
    addChild(bg);

    auto t = plotStartY;
    auto h = plotH;
    addChild(OSCPlotWidget<oscType>::create(rack::Vec(plotStartX, plotStartY),
                                            rack::Vec(plotW, plotH), module));

    float underX = plotStartX + 2;
    auto oct = widgets::LabeledPlotAreaControl::create(rack::Vec(underX, underPlotStartY),
                                              rack::Vec(34, underPlotH), "OCT",
                                              module,
                                              M::OCTAVE_SHIFT);
    oct->formatLabel = [](float f, const std::string &s)
    {
        std::string r = s;
        if (f > 0)
            r = "+" + r;
        return r;
    };
    addChild(oct);
    underX += 34 + 2;

    if constexpr (VCOConfig<oscType>::supportsUnison())
    {
        auto oct = widgets::LabeledPlotAreaControl::create(
            rack::Vec(underX, underPlotStartY), rack::Vec(32, underPlotH), "UNI", module, M::OSC_CTRL_PARAM_0 + 6);
        addChild(oct);
        underX += 32 + 2;
    }

    if (VCOConfig<oscType>::rightMenuParamId() >= 0)
    {
        auto restSz = plotW - underX + plotStartX;
        auto plt = widgets::PlotAreaMenuItem::create(
            rack::Vec(underX, underPlotStartY), rack::Vec(restSz, underPlotH), module, M::OSC_CTRL_PARAM_0 + VCOConfig<oscType>::rightMenuParamId());
        addChild(plt);
    }



    const auto &knobConfig = VCOConfig<oscType>::getKnobs();
    auto idx = 0;
    int row = 0, col  = 0;
    for (const auto k : knobConfig)
    {
        auto pid = k.id;
        auto label = k.name;

        if (k.type == VCOConfig<oscType>::KnobDef::Type::PARAM)
        {
            auto uxp = columnCenters_MM[col];
            auto uyp = rowCenters_MM[row];
            auto baseKnob =
                rack::createParamCentered<widgets::Knob9>(rack::mm2px(rack::Vec(uxp, uyp)), module, pid);
            addParam(baseKnob);
            for (int m = 0; m < M::n_mod_inputs; ++m)
            {
                auto radius = rack::mm2px(baseKnob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
                int id = M::modulatorIndexFor(pid, m);
                auto *k = widgets::ModRingKnob::createCentered(rack::mm2px(rack::Vec(uxp, uyp)),
                                                       radius, module, id);
                overlays[idx][m] = k;
                k->setVisible(false);
                k->underlyerParamWidget = baseKnob;
                baseKnob->modRings.insert(k);
                addChild(k);
            }

            // This is a little inefficient but
            for (const auto &[lidx, lid] : VCOConfig<oscType>::getLightsOnKnobsTo())
            {
                if (lidx == idx)
                {
                    addParam(rack::createParamCentered<widgets::ActivateKnobSwitch>(
                        rack::Vec(baseKnob->box.pos.x + baseKnob->box.size.x + 2.5,
                                  baseKnob->box.pos.y + 3),
                        module, M::ARBITRARY_SWITCH_0 + lid));
                }
            }
        }
        else if (k.type == VCOConfig<oscType>::KnobDef::Type::INPUT)
        {
            auto uxp = columnCenters_MM[col];
            auto uyp = rowCenters_MM[row] + verticalPortOffset_MM;
            addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module, k.id));
        }

        if (k.type != VCOConfig<oscType>::KnobDef::Type::BLANK)
        {
            if (k.colspan == 1)
            {
                addChild(makeLabel(row, col, k.name));
            }
            else if (k.colspan == 2)
            {
                auto cx = (columnCenters_MM[col] + columnCenters_MM[col+1]) * 0.5;
                auto bl = labelBaselines_MM[row];

                auto boxx0 = cx - columnWidth_MM;
                auto boxy0 = bl - 5;

                auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
                auto s0 = rack::mm2px(rack::Vec(columnWidth_MM * 2, 5));

                auto lab = widgets::Label::createWithBaselineBox(p0, s0, label);
                addChild(lab);
            }
        }
        idx++;
        col++;
        if (idx == 4)
        {
           col = 0;
           row ++;
        }
    }

    for (int i=0; i<M::n_mod_inputs; ++i)
    {
        addChild(makeLabel(2, i, std::string("MOD ") + std::to_string(i + 1)));
    }

    col = 0;
    row = 3;

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto uxp = columnCenters_MM[i];
        auto uyp = rowCenters_MM[2];

        auto *k =
            rack::createWidgetCentered<widgets::ModToggleButton>(rack::mm2px(rack::Vec(uxp, uyp)));
        toggles[i] = k;
        k->onToggle = [this, toggleIdx = i](bool isOn) {
            for (const auto &t : toggles)
                if (t)
                    t->setState(false);
            for (const auto &ob : overlays)
                for (const auto &o : ob)
                    if (o)
                        o->setVisible(false);
            if (isOn)
            {
                toggles[toggleIdx]->setState(true);
                for (const auto &ob : overlays)
                    if (ob[toggleIdx])
                    {
                        ob[toggleIdx]->setVisible(true);
                        ob[toggleIdx]->bdw->dirty = true;
                    }
            }
        };

        addChild(k);

        uyp = rowCenters_MM[3];

        addInput(rack::createInputCentered<widgets::Port>(
            rack::mm2px(rack::Vec(uxp, uyp)), module,
            M::OSC_MOD_INPUT + i));

    }

    col = 0;
    for (auto p : { M::PITCH_CV, M::RETRIGGER} )
    {
        auto yp = rowCenters_MM[4];
        auto xp = columnCenters_MM[col];
        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col ++;
    }

    for (auto p : { M::OUTPUT_L, M::OUTPUT_R} )
    {
        auto yp = rowCenters_MM[4];
        auto xp = columnCenters_MM[col];
        addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col ++;
    }

    col =0;
    for(const std::string &s : { "V/OCT", "TRIG", "LEFT", "RIGHT"})
    {
        addChild(makeLabel(3, col, s, ( col < 2 ? style::XTStyle::TEXT_LABEL : style::XTStyle::TEXT_LABEL_OUTPUT)));
        col++;
    }
}

} // namespace sst::surgext_rack::vco::ui

namespace vcoui = sst::surgext_rack::vco::ui;

rack::Model *modelVCOClassic =
    rack::createModel<vcoui::VCOWidget<ot_classic>::M, vcoui::VCOWidget<ot_classic>>(
        "SurgeXTOSCClassic");

rack::Model *modelVCOModern =
    rack::createModel<vcoui::VCOWidget<ot_modern>::M, vcoui::VCOWidget<ot_modern>>(
        "SurgeXTOSCModern");

rack::Model *modelVCOWavetable =
    rack::createModel<vcoui::VCOWidget<ot_wavetable>::M, vcoui::VCOWidget<ot_wavetable>>(
        "SurgeXTOSCWavetable");
rack::Model *modelVCOWindow =
    rack::createModel<vcoui::VCOWidget<ot_window>::M, vcoui::VCOWidget<ot_window>>(
        "SurgeXTOSCWindow");

rack::Model *modelVCOSine =
    rack::createModel<vcoui::VCOWidget<ot_sine>::M, vcoui::VCOWidget<ot_sine>>(
        "SurgeXTOSCSine");
rack::Model *modelVCOFM2 =
    rack::createModel<vcoui::VCOWidget<ot_FM2>::M, vcoui::VCOWidget<ot_FM2>>(
        "SurgeXTOSCFM2");
rack::Model *modelVCOFM3 =
    rack::createModel<vcoui::VCOWidget<ot_FM3>::M, vcoui::VCOWidget<ot_FM3>>(
        "SurgeXTOSCFM3");

rack::Model *modelVCOSHNoise =
    rack::createModel<vcoui::VCOWidget<ot_shnoise>::M, vcoui::VCOWidget<ot_shnoise>>(
        "SurgeXTOSCSHNoise");

rack::Model *modelVCOString =
    rack::createModel<vcoui::VCOWidget<ot_string>::M, vcoui::VCOWidget<ot_string>>(
        "SurgeXTOSCString");
rack::Model *modelVCOAlias =
    rack::createModel<vcoui::VCOWidget<ot_alias>::M, vcoui::VCOWidget<ot_alias>>(
        "SurgeXTOSCAlias");
