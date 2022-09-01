#include "SurgeOSCSingle.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"


template <> constexpr bool SingleConfig<ot_modern>::supportsUnison() { return true; }

template <> constexpr bool SingleConfig<ot_classic>::supportsUnison() { return true; }
template<>
SingleConfig<ot_classic>::knobs_t SingleConfig<ot_classic>::getKnobs()
{
    typedef SurgeOSCSingle<ot_classic> M;
    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0, "SHAPE"},
            {M::OSC_CTRL_PARAM_0 + 1, "WIDTH1"},
            {M::OSC_CTRL_PARAM_0 + 2, "WIDTH2"},
            {M::OSC_CTRL_PARAM_0 + 3, "SUBMIX"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <int oscType> struct SurgeOSCSingleWidget : public virtual SurgeModuleWidgetCommon
{
    typedef SurgeOSCSingle<oscType> M;
    SurgeOSCSingleWidget(M *module);

    int plotStart = 18, plotHeight = 100;
    int sideMargin = 5;
    int numberOfScrews = 10;
    int columnWidth = (SCREW_WIDTH * numberOfScrews - 2 * sideMargin)/ 4;
    int labelHeight = 15;

    std::array<std::array<SurgeModulatableRing *, M::n_mod_inputs>, 8> overlays;
    std::array<SurgeUIOnlyToggleButton *, M::n_mod_inputs> toggles;

    void moduleBackground(NVGcontext *vg)
    {
        auto t = plotStart;
        auto h = plotHeight;
        drawTextBGRect(vg, sideMargin, t, box.size.x - 2 * sideMargin, h);

        const auto &knobConfig = SingleConfig<oscType>::getKnobs();
        auto xp = sideMargin, yp = t + h + 2 * sideMargin, idx = 0;
        for (const auto &[p,l] : knobConfig)
        {
            idx++;
            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgText(vg, xp + columnWidth * 0.5, yp + columnWidth,
                    l.c_str(), nullptr);

            if (idx == 4)
            {
                xp = sideMargin; yp += columnWidth + labelHeight;
            }
            else
            {
                xp += columnWidth;
            }
        }

        xp = sideMargin;
        yp += columnWidth + sideMargin + columnWidth;
        for (int i=0; i<M::n_mod_inputs; ++i)
        {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            auto l = std::string( "MOD " ) + std::to_string(i+1);
            nvgText(vg, xp + columnWidth * 0.5, yp,
                    l.c_str(), nullptr);
            xp += columnWidth;
        }
        xp = sideMargin;
        yp += columnWidth + labelHeight + sideMargin;
        std::vector<std::string> lab = {"V/OCT", "TRIG", "L/MON", "R"};
        for (const auto &l : lab)
        {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 11);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgText(vg, xp + columnWidth * 0.5, yp,
                    l.c_str(), nullptr);
            xp += columnWidth;
        }

    }
};

template <int oscType> struct OSCPlotWidget : public rack::widget::Widget, SurgeStyle::StyleListener
{
    OSCPlotWidget() : Widget() { SurgeStyle::addStyleListener(this); }
    ~OSCPlotWidget() { SurgeStyle::removeStyleListener(this); }

    typename SurgeOSCSingleWidget<oscType>::M *module{nullptr};
    void setup(typename SurgeOSCSingleWidget<oscType>::M *m)
    {
        module = m;
        addChild(new BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                [this](NVGcontext *vg) { this->drawPlot(vg); }));
        if (module)
        {
            storage = module->storage.get();
            oscdata = &(storage->getPatch().scene[0].osc[1]); // this is tne no-mod storage
        }
    }

    void step() override
    {
        if (isDirty())
        {
            for (auto w : children)
            {
                if (auto fw = dynamic_cast<rack::FramebufferWidget *>(w))
                {
                    fw->dirty = true;
                }
            }
        }
        rack::widget::Widget::step();
    }

    virtual void styleHasChanged() override
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

    void drawPlot(NVGcontext *vg)
    {
        auto xp = box.size.x;
        auto yp = box.size.y;

        if (module)
        {
            auto osc = setupOscillator();
            const float ups = 3.0, invups = 1.0/ups;

            float disp_pitch_rs =
                12.f * std::log2f((700.f * (storage->samplerate / 48000.f)) / 440.f) + 69.f;

            osc->init(disp_pitch_rs, true, true);

            int block_pos{BLOCK_SIZE_OS + 1};
            nvgBeginPath(vg);
            for (int i=0; i<xp * ups; ++i)
            {
                if (block_pos >= BLOCK_SIZE_OS)
                {
                    osc->process_block(disp_pitch_rs);
                    block_pos = 0;
                }

                float yc = (-osc->output[block_pos] * 0.5 + 0.5) * yp;
                if (i == 0)
                {
                    nvgMoveTo(vg, i * invups, yc);
                }
                else
                {
                    nvgLineTo(vg, i * invups, yc);
                }
                block_pos ++;
            }

            osc->~Oscillator();
            nvgStrokeColor(vg, nvgRGB(255, 0x90, 0));
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);
        }
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

    box.size = rack::Vec(SCREW_WIDTH * numberOfScrews, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, std::string(M::name) + " VCO");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) { this->moduleBackground(vg); };
    addChild(bg);

    // addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(0), module, M::OUTPUT_L));
    // addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(1), module, M::OUTPUT_R));


    // auto retrigPos = rack::Vec(x0, yRes);
    // addInput(rack::createInput<rack::PJ301MPort>(retrigPos, module, M::RETRIGGER));

    auto t = plotStart;
    auto h = plotHeight;
    addChild(
        OSCPlotWidget<oscType>::create(rack::Vec(sideMargin, t), rack::Vec(box.size.x - 2 * sideMargin, h), module));

    const auto &knobConfig = SingleConfig<oscType>::getKnobs();

    auto xp = sideMargin, yp = t + h + 2 * sideMargin, idx = 0;

    for (const auto &[pid, label] : knobConfig)
    {
        auto uxp = xp + (columnWidth - 28) * 0.5;
        auto uyp = yp + (columnWidth - 28) * 0.5;
        auto baseKnob =
            rack::createParam<rack::RoundBlackKnob>(rack::Vec(uxp, uyp), module, pid);
        addParam(baseKnob);
        for (int m=0; m<M::n_mod_inputs; ++m)
        {
            int id = M::modulatorIndexFor(pid, m);
            auto *k = SurgeModulatableRing::create(rack::Vec(uxp, uyp), 28, module, id);
            overlays[idx][m] = k;
            k->setVisible(false);
            k->underlyerParamWidget = baseKnob;
            addChild(k);
        }

        idx++;
        if (idx == 4)
        {
            xp = sideMargin;
            yp += columnWidth + labelHeight;
        }
        else
        {
            xp += columnWidth;
        }
    }

    xp = sideMargin;
    yp += columnWidth + labelHeight + sideMargin;
    for (int i=0; i<M::n_mod_inputs; ++i)
    {
        auto *k = rack::createWidget<SurgeUIOnlyToggleButton>(rack::Vec(xp + (columnWidth-20)/2, yp));
        toggles[i] = k;
        k->onToggle = [this, toggleIdx = i](bool isOn)
        {
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
                for (const auto &ob: overlays)
                    if (ob[toggleIdx])
                    {
                        ob[toggleIdx]->setVisible(true);
                        ob[toggleIdx]->bdw->dirty = true;
                    }
            }
        };

        addChild(k);

        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xp + (columnWidth-24)/2, yp + columnWidth), module,
                                                     M::OSC_MOD_INPUT + i));

        xp += columnWidth;
    }

    xp = sideMargin;
    yp += 2 * columnWidth + labelHeight + sideMargin;

    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xp + (columnWidth-24)/2, yp), module, M::PITCH_CV));
    xp += columnWidth;
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xp + (columnWidth-24)/2, yp), module, M::RETRIGGER));
    xp += columnWidth;


    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(xp + (columnWidth-24)/2, yp), module, M::OUTPUT_L));
    xp += columnWidth;
    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(xp + (columnWidth-24)/2, yp), module, M::OUTPUT_R));
    xp += columnWidth;

}


rack::Model *modelSurgeOSCClassic =
    rack::createModel<SurgeOSCSingleWidget<ot_classic>::M, SurgeOSCSingleWidget<ot_classic>>(
        "SurgeXTOSCClassic");

rack::Model *modelSurgeOSCModern =
    rack::createModel<SurgeOSCSingleWidget<ot_modern>::M, SurgeOSCSingleWidget<ot_modern>>(
        "SurgeXTOSCModern");

rack::Model *modelSurgeOSCString =
    rack::createModel<SurgeOSCSingleWidget<ot_string>::M, SurgeOSCSingleWidget<ot_string>>(
        "SurgeXTOSCString");

rack::Model *modelSurgeOSCAlias =
    rack::createModel<SurgeOSCSingleWidget<ot_alias>::M, SurgeOSCSingleWidget<ot_alias>>(
        "SurgeXTOSCAlias");
