#include "SurgeOSCSingle.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

template <int oscType> struct SurgeOSCSingleWidget : public virtual SurgeModuleWidgetCommon
{
    typedef SurgeOSCSingle<oscType> M;
    SurgeOSCSingleWidget(M *module);

    int ioRegionWidth = 105;

    float waveHeight = 100;
    float controlsY = SCREW_WIDTH + padFromEdge + 3;
    float controlsHeight = orangeLine - controlsY - padMargin - waveHeight;
    float controlHeightPer = controlsHeight / (n_osc_params + 1) * 2;

    void moduleBackground(NVGcontext *vg)
    {
        // The input triggers and output
        nvgBeginPath(vg);

        // Draw the output blue box
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;
        drawBlueIORect(vg, x0 + ioMargin, orangeLine + ioMargin, ioRegionWidth,
                       box.size.y - orangeLine - 2 * ioMargin);

        nvgFillColor(vg, ioRegionText());
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgSave(vg);
        nvgTranslate(vg, x0 + ioMargin + ioRegionWidth - 2, orangeLine + ioMargin * 1.5);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgRotate(vg, M_PI / 2);
        nvgText(vg, 0, 0, "Output", NULL);
        nvgRestore(vg);

        rack::Vec ll;
        ll = ioPortLocation(0);
        ll.y = box.size.y - ioMargin - 1.5;
        ll.x += 24.6721 / 2;
        nvgFontSize(vg, 11);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "L/Mon", NULL);

        ll = ioPortLocation(1);
        ll.y = box.size.y - ioMargin - 1.5;
        ll.x += 24.6721 / 2;
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "R", NULL);

        ll = ioPortLocation(2);
        ll.y = box.size.y - ioMargin - 1.5;
        ll.x += 24.6721 / 2;
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "Gain", NULL);

        for (int i = 0; i < n_osc_params + 1; ++i)
        {
            int param = M::PITCH_0;
            int cvid = M::PITCH_CV;
            if (i != 0)
            {
                param = M::OSC_CTRL_PARAM_0 + i - 1;
                cvid = M::OSC_CTRL_CV_0 + i - 1;
            }

            std::string label = "param " + std::to_string(i - 1);
            if (i == 0)
                label = "Pitch";
            else if (module)
            {
                auto rm = dynamic_cast<M *>(module);
                if (rm)
                    label = rm->paramNames[i - 1];
            }

            float yp = (i % 4) * controlHeightPer + controlsY;
            float yctrl = yp + controlHeightPer / 2 - portY / 2 - padMargin / 2 - 3;
            int xOff = (i >= 4 ? box.size.x / 2 : 0) + 5;

            nvgSave(vg);
            nvgFillColor(vg, panelLabel());
            nvgTranslate(vg, xOff, yp);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgText(vg, 0, 0, label.c_str(), NULL);
            nvgRestore(vg);
        }

        auto t = orangeLine - 104;
        auto h = 94;
        drawTextBGRect(vg, 10, t, box.size.x - 20, h);
    }

    rack::Vec ioPortLocation(int ctrl)
    { // 0 is L; 1 is R; 2 is gain
        float x0 = box.size.x - ioRegionWidth - 2 * ioMargin;

        int xRes = x0 + ioMargin + padFromEdge + (ctrl * (portX + 4));
        int yRes = orangeLine + 1.5 * ioMargin;

        return rack::Vec(xRes, yRes);
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
            oscdata = &(storage->getPatch().scene[0].osc[0]);
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

        if (!module)
        {
            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, yp / 2);
            for (int i = 1; i < xp; ++i)
            {
                auto nx = i * 1.f / xp;
                auto sx = nx * 4 * 3.14159;
                auto sy = std::sin(sx);
                auto py = (-sy * 0.5 + 0.5) * yp;
                nvgLineTo(vg, i, py);
            }
            nvgStrokeColor(vg, nvgRGB(255, 0, 0));
            nvgStrokeWidth(vg, 1);
            nvgStroke(vg);
        }
        else
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

    box.size = rack::Vec(SCREW_WIDTH * 14, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, std::string(M::name) + " VCO");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) { this->moduleBackground(vg); };
    addChild(bg);

    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(0), module, M::OUTPUT_L));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(1), module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(2), module, M::OUTPUT_GAIN));

    float x0 = 2 * ioMargin;
    int yRes = orangeLine + 1.5 * ioMargin;

    auto retrigPos = rack::Vec(x0, yRes);
    addInput(rack::createInput<rack::PJ301MPort>(retrigPos, module, M::RETRIGGER));

    auto t = orangeLine - 104;
    auto h = 94;
    addChild(
        OSCPlotWidget<oscType>::create(rack::Vec(10, t), rack::Vec(box.size.x - 20, h), module));

    for (int i = 0; i < n_osc_params + 1; ++i)
    {
        int param = M::PITCH_0;
        int cvid = M::PITCH_CV;
        if (i != 0)
        {
            param = M::OSC_CTRL_PARAM_0 + i - 1;
            cvid = M::OSC_CTRL_CV_0 + i - 1;
        }

        float yp = (i % 4) * controlHeightPer + controlsY;
        float yctrl = yp + controlHeightPer / 2 - portY / 2 - padMargin / 2;
        int xOff = (i >= 4 ? box.size.x / 2 : 0);

        addParam(
            rack::createParam<SurgeSmallKnob>(rack::Vec(xOff + padFromEdge, yctrl), module, param));
        addInput(rack::createInput<rack::PJ301MPort>(
            rack::Vec(xOff + padFromEdge + padMargin + portX, yctrl), module, cvid));

        if (i != 0)
        {
            addParam(rack::createParam<SurgeDisableStateSwitch>(
                rack::Vec(xOff + padFromEdge + 2 * padMargin + 2 * portX, yctrl), module,
                M::OSC_EXTEND_PARAM_0 + i - i));
            addParam(rack::createParam<SurgeDisableStateSwitch>(
                rack::Vec(xOff + padFromEdge + 2 * padMargin + 2 * portX + 12, yctrl), module,
                M::OSC_DEACTIVATE_INVERSE_PARAM_0 + i - i));
        }
        float xt = padFromEdge + 2 * padMargin + 2 * portX + 12;
    }
}

template <> constexpr bool SingleConfig<ot_modern>::supportsUnison() { return true; }

rack::Model *modelSurgeOSCModern =
    rack::createModel<SurgeOSCSingleWidget<ot_modern>::M, SurgeOSCSingleWidget<ot_modern>>(
        "SurgeOSCModern");

rack::Model *modelSurgeOSCString =
    rack::createModel<SurgeOSCSingleWidget<ot_string>::M, SurgeOSCSingleWidget<ot_string>>(
        "SurgeOSCString");

rack::Model *modelSurgeOSCAlias =
    rack::createModel<SurgeOSCSingleWidget<ot_alias>::M, SurgeOSCSingleWidget<ot_alias>>(
        "SurgeOSCAlias");
