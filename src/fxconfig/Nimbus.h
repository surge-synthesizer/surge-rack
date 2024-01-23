/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
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

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_NIMBUS_H
#define SURGE_XT_RACK_SRC_FXCONFIG_NIMBUS_H

#include "dsp/effects/NimbusEffect.h"

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_nimbus>::numParams() { return 12; }
template <> constexpr int FXConfig<fxt_nimbus>::extraInputs() { return 2; }
template <> constexpr int FXConfig<fxt_nimbus>::extraSchmidtTriggers() { return MAX_POLY; }
template <> constexpr int FXConfig<fxt_nimbus>::specificParamCount() { return 2; }

template <> FXConfig<fxt_nimbus>::layout_t FXConfig<fxt_nimbus>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    // clang-format off
    auto res = FXConfig<fxt_nimbus>::layout_t{

        {LayoutItem::KNOB9, "POSITION", NimbusEffect::nmb_position, col[0], row1},
        {LayoutItem::KNOB9, "SIZE", NimbusEffect::nmb_size, col[1], row1},
        {LayoutItem::KNOB9, "PITCH", NimbusEffect::nmb_pitch, col[2], row1},
        {LayoutItem::KNOB9, "DENSITY", NimbusEffect::nmb_density, col[3], row1},

        {LayoutItem::PORT, "TRIG", FX<fxt_nimbus>::INPUT_SPECIFIC_0 + 1, col[0], row2},

        {LayoutItem::PORT, "", FX<fxt_nimbus>::INPUT_SPECIFIC_0, col[1], row2},
        {LayoutItem::MOMENTARY_PARAM, "", FX<fxt_spring_reverb>::FX_SPECIFIC_PARAM_0, col[2], row2},
        LayoutItem::createKnobSpanLabel("FREEZE", col[1], row2, 2),

        {LayoutItem::KNOB9, "FEEDBACK", NimbusEffect::nmb_feedback, col[3], row2},
        LayoutItem::createGrouplabel("PLAYBACK", col[1], row2, 3),

        {LayoutItem::KNOB9, "TEXTURE", NimbusEffect::nmb_texture, col[0], row3},
        {LayoutItem::KNOB9, "SPREAD", NimbusEffect::nmb_spread, col[1], row3},


        {LayoutItem::KNOB9, "REVERB", NimbusEffect::nmb_reverb, col[2], row3},
        {LayoutItem::KNOB9, "MIX", NimbusEffect::nmb_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),


        LayoutItem::createPresetPlusTwoArea(),
        LayoutItem::createLeftMenuItem("MODE", NimbusEffect::nmb_mode),
        LayoutItem::createRightMenuItem("QUALITY",NimbusEffect::nmb_quality)
    };
    // clang-format on

    auto &sz = res[1];
    auto &den = res[3];
    auto &tex = res[10];

    sz.dynamicLabel = true;
    sz.dynLabelFn = [](auto *m) {
        auto that = dynamic_cast<FX<fxt_nimbus> *>(m);
        if (!that)
            return "ERROR";
        auto md = that->fxstorage->p[NimbusEffect::nmb_mode].val.i;
        if (md == 3)
            return "WARP";
        return "SIZE";
    };

    den.dynamicLabel = true;
    den.dynLabelFn = [](auto *m) {
        auto that = dynamic_cast<FX<fxt_nimbus> *>(m);
        if (!that)
            return "ERROR";
        auto md = that->fxstorage->p[NimbusEffect::nmb_mode].val.i;
        if (md == 0)
            return "DENSITY";
        if (md == 1 || md == 2)
            return "DIFF";
        if (md == 3)
            return "SMEAR";

        return "ERROR";
    };

    tex.dynamicLabel = true;
    tex.dynLabelFn = [](auto *m) {
        auto that = dynamic_cast<FX<fxt_nimbus> *>(m);
        if (!that)
            return "ERROR";
        auto md = that->fxstorage->p[NimbusEffect::nmb_mode].val.i;
        if (md == 0 || md == 3)
            return "TEXTURE";
        if (md == 1 || md == 2)
            return "FILTER";

        return "ERROR";
    };

    return res;
}

template <> void FXConfig<fxt_nimbus>::configSpecificParams(FX<fxt_nimbus> *m)
{
    typedef FX<fxt_nimbus> fx_t;
    m->configOnOffNoRand(fx_t::FX_SPECIFIC_PARAM_0, 0, "Manual Freeze");
    m->configOnOffNoRand(fx_t::FX_SPECIFIC_PARAM_0 + 1, 0, "Randomize Engine");

    m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_mode]->randomizeEnabled =
        false;
    m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_quality]->randomizeEnabled =
        false;
}

template <> void FXConfig<fxt_nimbus>::configExtraInputs(FX<fxt_nimbus> *m)
{
    m->configInput(FX<fxt_nimbus>::INPUT_SPECIFIC_0, "Gate to Freeze");
    m->configInput(FX<fxt_nimbus>::INPUT_SPECIFIC_0 + 1, "Trigger");

    m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_freeze]->randomizeEnabled =
        false;
}

template <> void FXConfig<fxt_nimbus>::processSpecificParams(FX<fxt_nimbus> *m)
{
    auto md = m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_mode]->getValue();

    auto lv = m->paramQuantities[FX<fxt_nimbus>::FX_SPECIFIC_PARAM_0 + 1]->getValue() > 0.5;
    m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_mode]->randomizeEnabled = lv;
    m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_quality]->randomizeEnabled =
        lv;
}

template <> void FXConfig<fxt_nimbus>::processExtraInputs(FX<fxt_nimbus> *that, int channel)
{
    auto uc = channel * (that->inputs[FX<fxt_nimbus>::INPUT_SPECIFIC_0].getChannels() > 1);
    auto tc = channel * (that->inputs[FX<fxt_nimbus>::INPUT_SPECIFIC_0 + 1].getChannels() > 1);

    auto frozen = that->inputs[FX<fxt_nimbus>::INPUT_SPECIFIC_0].getVoltage(uc) > 3;
    frozen = frozen || (that->params[FX<fxt_nimbus>::FX_SPECIFIC_PARAM_0].getValue() > 0.5);
    auto triggered = that->extraInputTriggers[channel].process(
        that->inputs[FX<fxt_nimbus>::INPUT_SPECIFIC_0 + 1].getVoltage(tc));

    if (frozen)
    {
        that->fxstorage->p[NimbusEffect::nmb_freeze].set_value_f01(1);
    }
    else
    {
        that->fxstorage->p[NimbusEffect::nmb_freeze].set_value_f01(0);
    }

    if (that->polyphonicMode)
    {
        auto nb = static_cast<NimbusEffect *>(that->surge_effect_poly[channel].get());
        nb->setNimbusTrigger(triggered);
    }
    else
    {
        auto nb = static_cast<NimbusEffect *>(that->surge_effect.get());
        nb->setNimbusTrigger(triggered);
    }
}

template <>
void FXConfig<fxt_nimbus>::addFXSpecificMenuItems(FX<fxt_nimbus> *m, rack::ui::Menu *toThis)
{
    auto l1 = (int)std::round(m->params[FX<fxt_nimbus>::FX_SPECIFIC_PARAM_0 + 1].getValue());

    toThis->addChild(new rack::ui::MenuSeparator());
    toThis->addChild(
        rack::createMenuItem("Randomize Nimbus Mode/Quality", CHECKMARK(l1), [m, l1]() {
            m->params[FX<fxt_nimbus>::FX_SPECIFIC_PARAM_0 + 1].setValue(l1 ? 0 : 1);
            m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_mode]
                ->randomizeEnabled = !l1;
            m->paramQuantities[FX<fxt_nimbus>::FX_PARAM_0 + NimbusEffect::nmb_quality]
                ->randomizeEnabled = !l1;
        }));
}
} // namespace sst::surgext_rack::fx

#endif // SURGEXTRACK_NIMBUS_H
