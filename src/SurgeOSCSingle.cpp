#include "SurgeOSCSingle.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

#include "SurgeOSCSingleConfig.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::vco::ui
{
template <int oscType> struct SurgeOSCSingleWidget : public virtual widgets::SurgeModuleWidgetCommon
{
    typedef SurgeOSCSingle<oscType> M;
    SurgeOSCSingleWidget(M *module);

    float plotH_MM = 36;
    float plotW_MM = 51;
    float plotCX_MM = 30.48;
    float plotCY_MM = 27.35;

    float plotControlsH_MM = 5;

    std::array<float, 4> columnCenters_MM{9.48, 23.48, 37.48, 51.48};
    std::array<float, 5> rowCenters_MM{55,71,85.32, 100.16, 114.5};
    float verticalPortOffset_MM = 0.5;

    float plotStartX = rack::mm2px(plotCX_MM - plotW_MM * 0.5);
    float plotStartY = rack::mm2px(plotCY_MM - plotH_MM * 0.5);
    float plotW = rack::mm2px(plotW_MM);
    float plotH = rack::mm2px(plotH_MM - plotControlsH_MM);

    int numberOfScrews = 12;

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, 8> overlays;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

template <int oscType>
struct OSCPlotWidget : public rack::widget::TransparentWidget, style::StyleListener
{
    OSCPlotWidget() : TransparentWidget() { style::SurgeStyle::addStyleListener(this); }
    ~OSCPlotWidget() { style::SurgeStyle::removeStyleListener(this); }

    typename SurgeOSCSingleWidget<oscType>::M *module{nullptr};
    void setup(typename SurgeOSCSingleWidget<oscType>::M *m)
    {
        module = m;
        if (module)
        {
            storage = module->storage.get();
            oscdata = &(storage->getPatch().scene[0].osc[1]); // this is tne no-mod storage
        }
    }

    void step() override
    {
        if (!module)
            return;

        if (isDirty())
        {
            recalcPath();
        }
        rack::widget::Widget::step();
    }

    void draw(const DrawArgs &args) override { drawPlot(args.vg); }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            drawPlot(args.vg);
        }
    }

    virtual void onStyleChanged() override
    {
        for (auto w : children)
        {
            if (auto fw = dynamic_cast<rack::FramebufferWidget *>(w))
            {
                fw->dirty = true;
            }
        }
    }

    static OSCPlotWidget<oscType> *create(rack::Vec pos, rack::Vec size,
                                          typename SurgeOSCSingleWidget<oscType>::M *module)
    {
        auto *res = rack::createWidget<OSCPlotWidget<oscType>>(pos);

        res->box.pos = pos;
        res->box.size = size;

        res->setup(module);

        return res;
    }

    bool firstDirty{false};
    int dirtyCount{0};
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
            for (int i = 0; i < n_osc_params; i++)
            {
                dval = dval || (tp[oscdata->p[i].param_id_in_scene].i != oscdata->p[i].val.i);
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

        return spawn_osc(oscdata->type.val.i, storage, oscdata, tp, oscbuffer);
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

            float yc = (-osc->output[block_pos] * 0.5 + 0.5) * yp;
            oscPath.emplace_back(i * invups, yc);
            block_pos++;
        }

        osc->~Oscillator();
    }

    void drawPlot(NVGcontext *vg)
    {
        if (!oscPath.empty())
        {
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
            nvgStrokeColor(vg, nvgRGB(255, 0x90, 0));
            nvgStrokeWidth(vg, 1.25);
            nvgStroke(vg);

            nvgStrokeColor(vg, nvgRGBA(255, 0x90, 0, 50));
            nvgStrokeWidth(vg, 3);
            nvgStroke(vg);
        }

        if (!module)
        {
            // Draw the module name here for preview goodness
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style::SurgeStyle::fontIdBold(vg));
            nvgFontSize(vg, 30);
            nvgFillColor(vg, nvgRGB(0xFF, 0x90, 0x00));
            nvgText(vg, box.size.x * 0.5,
                    box.size.y * 0.5,
                    osc_type_names[oscType], nullptr);
        }

#define DEBUG_BOUNDS 0
#if DEBUG_BOUNDS
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg,nvgRGBA(100,100,255, 120));
        nvgFill(vg);
#endif
    }
};

template <int oscType>
SurgeOSCSingleWidget<oscType>::SurgeOSCSingleWidget(SurgeOSCSingleWidget<oscType>::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    for (auto &ob : overlays)
        for (auto &o : ob)
            o = nullptr;

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, std::string(M::name));
    addChild(bg);

    auto t = plotStartY;
    auto h = plotH;
    addChild(OSCPlotWidget<oscType>::create(rack::Vec(plotStartX, plotStartY),
                                            rack::Vec(plotW, plotH), module));

    const auto &knobConfig = SingleConfig<oscType>::getKnobs();

    auto idx = 0;
    int row = 0, col  = 0;
    for (const auto k : knobConfig)
    {
        auto pid = k.id;
        auto label = k.name;

        if (k.type == SingleConfig<oscType>::KnobDef::Type::PARAM)
        {
            auto uxp = columnCenters_MM[col];
            auto uyp = rowCenters_MM[row];
            auto baseKnob =
                rack::createParamCentered<widgets::Knob9>(rack::mm2px(rack::Vec(uxp, uyp)), module, pid);
            addParam(baseKnob);
            for (int m = 0; m < M::n_mod_inputs; ++m)
            {
                auto radius = rack::mm2px(widgets::Knob9::knobSize_MM + 2 * widgets::Knob9::ringWidth_MM);
                int id = M::modulatorIndexFor(pid, m);
                auto *k = widgets::ModRingKnob::createCentered(rack::mm2px(rack::Vec(uxp, uyp)),
                                                       radius, module, id);
                overlays[idx][m] = k;
                k->setVisible(false);
                k->underlyerParamWidget = baseKnob;
                addChild(k);
            }
        }
        else
        {
            auto uxp = columnCenters_MM[col];
            auto uyp = rowCenters_MM[row];
            addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module, k.id));
        }
        idx++;
        col++;
        if (idx == 4)
        {
           col = 0;
           row ++;
        }
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
}

} // namespace sst::surgext_rack::vco::ui

namespace vcoui = sst::surgext_rack::vco::ui;

rack::Model *modelSurgeOSCClassic =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_classic>::M, vcoui::SurgeOSCSingleWidget<ot_classic>>(
        "SurgeXTOSCClassic");

rack::Model *modelSurgeOSCModern =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_modern>::M, vcoui::SurgeOSCSingleWidget<ot_modern>>(
        "SurgeXTOSCModern");

rack::Model *modelSurgeOSCWavetable =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_wavetable>::M, vcoui::SurgeOSCSingleWidget<ot_wavetable>>(
        "SurgeXTOSCWavetable");
rack::Model *modelSurgeOSCWindow =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_window>::M, vcoui::SurgeOSCSingleWidget<ot_window>>(
        "SurgeXTOSCWindow");

rack::Model *modelSurgeOSCSine =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_sine>::M, vcoui::SurgeOSCSingleWidget<ot_sine>>(
        "SurgeXTOSCSine");
rack::Model *modelSurgeOSCFM2 =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_FM2>::M, vcoui::SurgeOSCSingleWidget<ot_FM2>>(
        "SurgeXTOSCFM2");
rack::Model *modelSurgeOSCFM3 =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_FM3>::M, vcoui::SurgeOSCSingleWidget<ot_FM3>>(
        "SurgeXTOSCFM3");

rack::Model *modelSurgeOSCSHNoise =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_shnoise>::M, vcoui::SurgeOSCSingleWidget<ot_shnoise>>(
        "SurgeXTOSCSHNoise");

rack::Model *modelSurgeOSCString =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_string>::M, vcoui::SurgeOSCSingleWidget<ot_string>>(
        "SurgeXTOSCString");
rack::Model *modelSurgeOSCAlias =
    rack::createModel<vcoui::SurgeOSCSingleWidget<ot_alias>::M, vcoui::SurgeOSCSingleWidget<ot_alias>>(
        "SurgeXTOSCAlias");
