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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_ALIAS_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_ALIAS_H

#include "dsp/oscillators/AliasOscillator.h"
#include "../NBarEditorWidget.h"

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
template <> bool VCOConfig<ot_alias>::showRightMenuChoice(int choiceIndex)
{
    if (choiceIndex >= AliasOscillator::aow_mem_alias &&
        choiceIndex <= AliasOscillator::aow_mem_stepseqdata)
        return false;

    return true;
}

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
    // Push the extra config data on for the additives and stuff
    for (int i = 0; i < 16; ++i)
    {
        auto pv =
            std::clamp(m->params[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i].getValue(), -1.f, 1.f);
        m->oscstorage->extraConfig.data[i] = pv;
        m->oscstorage_display->extraConfig.data[i] = pv;
    }

    // Block the modes we forbid here also
    auto sp = &(m->oscstorage_display->p[AliasOscillator::ao_wave]);
    auto mx = sp->val_max.i;
    auto mn = sp->val_min.i;
    auto rp = m->paramQuantities[VCO<ot_alias>::OSC_CTRL_PARAM_0 + AliasOscillator::ao_wave];

    auto vl = Parameter::intUnscaledFromFloat(rp->getValue(), mx, mn);

    if (vl == AliasOscillator::aow_mem_stepseqdata || vl == AliasOscillator::aow_mem_dawextra)
    {
        // We probably hit this with a drag from above so
        rp->setValue(Parameter::intScaledToFloat(AliasOscillator::aow_noise, mx, mn));
    }
    else if (vl >= AliasOscillator::aow_mem_alias && vl <= AliasOscillator::aow_mem_stepseqdata)
    {
        // We probably hit this with a drag from below so
        rp->setValue(Parameter::intScaledToFloat(AliasOscillator::aow_audiobuffer, mx, mn));
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

        int par = VCO<ot_alias>::ADDITIONAL_VCO_PARAMS;
        float y0 = rack::mm2px(6);
        float xpad = rack::mm2px(2);
        float h = box.size.y - rack::mm2px(1) - y0;
        float w = (box.size.x - 2 * xpad);
        auto bed =
            widgets::NBarWidget<16>::create(rack::Vec(xpad, y0), rack::Vec(w, h), module, par);
        bed->barLabel = "Harmonic";
        bed->makeAdditionalMenu = [this](auto w, auto m) { makeAdditionalAdditivePresetMenu(m); };

        addChild(bed);
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

    void makeAdditionalAdditivePresetMenu(rack::Menu *m)
    {
        if (!module)
            return;
        m->addChild(new rack::MenuSeparator);
        m->addChild(rack::createMenuLabel("Presets"));
        m->addChild(rack::createMenuItem("Sine", "", [this]() {
            for (int i = 0; i < AliasOscillator::n_additive_partials; ++i)
            {
                auto v = (i == 0 ? 1.f : 0.f);
                module->paramQuantities[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i]->setValue(v);
            }
        }));

        m->addChild(rack::createMenuItem("Triangle", "", [this]() {
            for (int i = 0; i < AliasOscillator::n_additive_partials; ++i)
            {
                auto qq = i;
                auto v = (qq % 2 == 0) * 1.f / ((qq + 1) * (qq + 1));
                if (qq % 4 == 2)
                    v = -v;

                module->paramQuantities[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i]->setValue(v);
            }
        }));

        m->addChild(rack::createMenuItem("Sawtooth", "", [this]() {
            for (int i = 0; i < AliasOscillator::n_additive_partials; ++i)
            {
                auto v = 1.0 / (i + 1);
                module->paramQuantities[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i]->setValue(v);
            }
        }));

        m->addChild(rack::createMenuItem("Square", "", [this]() {
            for (int i = 0; i < AliasOscillator::n_additive_partials; ++i)
            {
                auto v = (i % 2 == 0) * 1.f / (i + 1);
                module->paramQuantities[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i]->setValue(v);
            }
        }));

        m->addChild(rack::createMenuItem("Random", "", [this]() {
            for (int i = 0; i < AliasOscillator::n_additive_partials; ++i)
            {
                auto v = 1.f * rand() / (float)RAND_MAX;
                v = v * 2 - 1;
                module->paramQuantities[VCO<ot_alias>::ADDITIONAL_VCO_PARAMS + i]->setValue(v);
            }
        }));
    }
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
