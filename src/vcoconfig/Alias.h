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

#ifndef SURGEXT_ALIAS_CONFIG_H
#define SURGEXT_ALIAS_CONFIG_H

#include "dsp/oscillators/AliasOscillator.h"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_alias>::supportsUnison() { return true; }
template <> constexpr int VCOConfig<ot_alias>::additionalVCOParameterCount() { return 16; }
template <> VCOConfig<ot_alias>::layout_t VCOConfig<ot_alias>::getLayout()
{
    typedef VCO<ot_alias> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 1, "WARP", 0, 2),
        LayoutItem::createVCOKnob(cp + 2, "MASK", 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "DETUNE", 1, 0),
        LayoutItem::createVCOPort(M::AUDIO_INPUT, "AUDIO", 1, 1),

        LayoutItem::createVCOKnob(cp + 3, "THRESH", 1, 2),
        LayoutItem::createVCOKnob(cp + 4, "CRUSH", 1, 3)
        // clang-format on
    };
}
template <> int VCOConfig<ot_alias>::rightMenuParamId() { return 0; }
template <> constexpr bool VCOConfig<ot_alias>::supportsAudioIn() { return true; }

template <> void VCOConfig<ot_alias>::configureVCOSpecificParameters(VCO<ot_alias> *m)
{
    for (int i = 0; i < additionalVCOParameterCount(); ++i)
    {
        m->configParam(VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i, -1, 1, 1.0 / (i + 1),
                       std::string("Additive Harmonic ") + std::to_string(i + 1));
    }

    for (int i = 0; i < VCO<ot_alias>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_alias>::ARBITRARY_SWITCH_0 + i, 0, 1, 0,
                       std::string("Unused Param ") + std::to_string(i + 1));
    }
}

template <> void VCOConfig<ot_alias>::processVCOSpecificParameters(VCO<ot_alias> *m)
{
    for (int i = 0; i < 16; ++i)
    {
        auto pv =
            std::clamp(m->params[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i].getValue(), -1.f, 1.f);
        m->oscstorage->extraConfig.data[i] = pv;
        m->oscstorage_display->extraConfig.data[i] = pv;
    }
}

// Harmonic Editor Support

template <> constexpr int VCOConfig<ot_alias>::supportsCustomEditor() { return true; }
template <> bool VCOConfig<ot_alias>::isCustomEditorActivatable(VCO<ot_alias> *m)
{
    auto wv = m->oscstorage_display->p[AliasOscillator::ao_wave].val.i;
    if (wv == AliasOscillator::aow_additive)
        return true;

    return false;
}

namespace alias_ed
{
struct HarmEd : public rack::Widget, style::StyleParticipant
{
    // I'm sure one day the fact that i copied this form the LFO will make me sad
    struct HarmSlider : rack::app::SliderKnob, style::StyleParticipant
    {
        widgets::BufferedDrawFunctionWidget *bdw{nullptr};
        widgets::BufferedDrawFunctionWidget *bdwLight{nullptr};

        static HarmSlider *create(const rack::Vec &pos, const rack::Vec &sz,
                                  modules::XTModule *module, int paramId)
        {
            auto res = new HarmSlider();

            res->box.pos = pos;
            res->box.size = sz;

            res->module = module;
            res->paramId = paramId;
            res->initParamQuantity();

            res->setup();

            return res;
        }

        void setup()
        {
            bdw = new widgets::BufferedDrawFunctionWidget(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawSlider(vg); });
            bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
                rack::Vec(0, 0), box.size, [this](auto *vg) { this->drawLight(vg); });

            addChild(bdw);
            addChild(bdwLight);
        }

        static constexpr float cramY = 0.98, padY = (1.f - cramY) * 0.5f;
        void drawSlider(NVGcontext *vg)
        {
            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, box.size.x, box.size.y);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);

            auto pq = getParamQuantity();
            if (!pq)
                return;
            auto v = (-pq->getValue()) * 0.5 + 0.5;

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            auto gcp = col;
            gcp.a = 0.0;
            auto gcn = col;
            gcn.a = 0.9;

            auto s = box.size.y * 0.5;
            auto e = cramY * box.size.y * v + padY;
            if (s > e)
            {
                std::swap(gcp, gcn);
                std::swap(s, e);
            }
            nvgBeginPath(vg);
            nvgRect(vg, 1, s, box.size.x - 2, e - s);
            nvgFillPaint(vg, nvgLinearGradient(vg, 0, s, 0, e, gcp, gcn));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, box.size.y * 0.5);
            nvgLineTo(vg, box.size.x, box.size.y * 0.5);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }

        void drawLight(NVGcontext *vg)
        {
            auto pq = getParamQuantity();
            if (!pq)
                return;
            auto v = (-pq->getValue()) * 0.5 + 0.5;

            auto e = cramY * box.size.y * v + padY;

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);

            nvgBeginPath(vg);
            nvgMoveTo(vg, 1, e);
            nvgLineTo(vg, box.size.x - 1, e);
            nvgStrokeWidth(vg, 1.5);
            nvgStrokeColor(vg, col);
            nvgStroke(vg);
        }

        int sp{-1}, ep{-1};
        bool uni{false};

        inline void onChange(const rack::widget::Widget::ChangeEvent &e) override
        {
            bdw->dirty = true;
            bdwLight->dirty = true;
        }

        void onStyleChanged() override
        {
            bdw->dirty = true;
            bdwLight->dirty = true;
        }

        rack::Vec buttonPos;
        void onButton(const ButtonEvent &e) override
        {
            if (e.action == GLFW_PRESS)
                buttonPos = e.pos;

            if (e.action == GLFW_RELEASE)
            {
                auto diff = buttonPos - e.pos;
                if (diff.norm() < 0.05)
                {
                    // auto e = cramY * box.size.y * v + padY;
                    // e - padY = cY * bsy * v
                    // v = ( e- padY)/(cY * bsy);
                    auto val = (buttonPos.y - padY) / (cramY * box.size.y);
                    val = (1 - val) * 2 - 1;
                    if (getParamQuantity())
                        getParamQuantity()->setValue(val);
                }
                buttonPos = rack::Vec(-1, -1);
                bdw->dirty = true;
                bdwLight->dirty = true;
            }

            SliderKnob::onButton(e);
        }
    };

    VCO<ot_alias> *module{nullptr};
    std::function<void(rack::Widget *)> onClose{nullptr};
    bool closePressed{false};
    void onButton(const ButtonEvent &e) override
    {
        if (e.pos.x > box.size.x - xsedit && e.pos.y < ysedit)
        {
            if (e.action == GLFW_PRESS)
            {
                closePressed = true;
            }
            else if (e.action == GLFW_RELEASE && closePressed)
            {
                closePressed = false;
                e.consume(this);
                if (onClose)
                    onClose(this);
            }
        }
        rack::Widget::onButton(e);
    }

    const float xsedit{rack::mm2px(12)};
    const float ysedit{rack::mm2px(3.5)};
    widgets::BufferedDrawFunctionWidget *bdwClose{nullptr};

    void setup()
    {
        if (module == nullptr)
            return;

        bdwClose = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                           [this](auto *v) { drawClose(v); });
        addChild(bdwClose);

        for (int i = 0; i < 16; ++i)
        {
            int par = VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i;
            float y0 = rack::mm2px(6);
            float xpad = rack::mm2px(2);
            float h = box.size.y - rack::mm2px(1) - y0;
            float w = (box.size.x - 2 * xpad) / 16;
            float x0 = xpad + w * i;

            auto s = HarmSlider::create(rack::Vec(x0, y0), rack::Vec(w, h), module, par);
            addChild(s);
        }
    }

    void drawClose(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRect(vg, box.size.x - xsedit, 0, xsedit, ysedit);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_FG));
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, box.size.x - xsedit * 0.5, ysedit * 0.5, "CLOSE", nullptr);
    }

    void onStyleChanged() override { bdwClose->dirty = true; }
};
} // namespace alias_ed

template <>
rack::Widget *VCOConfig<ot_alias>::createCustomEditorAt(const rack::Vec &pos, const rack::Vec &size,
                                                        VCO<ot_alias> *m,
                                                        std::function<void(rack::Widget *)> onClose)
{
    auto res = new alias_ed::HarmEd();
    res->box.pos = pos;
    res->box.size = size;
    res->onClose = onClose;
    res->module = m;

    res->setup();
    return res;
}

} // namespace sst::surgext_rack::vco

#endif
