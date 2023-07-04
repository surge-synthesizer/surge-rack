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

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_NIMBUS_H
#define SURGE_XT_RACK_SRC_FXCONFIG_NIMBUS_H

#include "dsp/effects/NimbusEffect.h"

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_nimbus>::numParams() { return 12; }
template <> constexpr int FXConfig<fxt_nimbus>::extraInputs() { return 1; }
template <> constexpr int FXConfig<fxt_nimbus>::extraSchmidtTriggers() { return 1; }

template <> FXConfig<fxt_nimbus>::layout_t FXConfig<fxt_nimbus>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM - (14 - 9) * 0.5f;

    const auto col15 = (col[0] + col[1]) * 0.5f;
    const auto col25 = (col[2] + col[3]) * 0.5f;

    // clang-format off
    auto res = FXConfig<fxt_nimbus>::layout_t{

        {LayoutItem::KNOB12, "POSITION", NimbusEffect::nmb_position, col15, row1},
        {LayoutItem::KNOB12, "", NimbusEffect::nmb_size, col25, row1},

        {LayoutItem::KNOB9, "PITCH", NimbusEffect::nmb_pitch, col[0], row2},
        {LayoutItem::KNOB9, "", NimbusEffect::nmb_density, col[1], row2},
        {LayoutItem::KNOB9, "", NimbusEffect::nmb_texture, col[2], row2},
        {LayoutItem::KNOB9, "SPREAD", NimbusEffect::nmb_spread, col[3], row2},

        {LayoutItem::PORT, "FREEZE", FX<fxt_nimbus>::INPUT_SPECIFIC_0, col[0], row3},
        {LayoutItem::KNOB9, "FEEDBACK", NimbusEffect::nmb_feedback, col[1], row3},
        LayoutItem::createGrouplabel("PLAYBACK", col[0], row3, 2),

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
    auto &tex = res[4];

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
            return "DIFF"
                   "";
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

template <> void FXConfig<fxt_nimbus>::configExtraInputs(FX<fxt_nimbus> *m)
{
    m->configInput(FX<fxt_nimbus>::INPUT_SPECIFIC_0, "Gate to Freeze");
}

template <> void FXConfig<fxt_nimbus>::processExtraInputs(FX<fxt_nimbus> *that)
{
    auto frozen = that->inputs[FX<fxt_nimbus>::INPUT_SPECIFIC_0].getVoltage() > 3;

    if (frozen)
    {
        that->fxstorage->p[NimbusEffect::nmb_freeze].set_value_f01(1);
    }
    else
    {
        that->fxstorage->p[NimbusEffect::nmb_freeze].set_value_f01(0);
    }
}
} // namespace sst::surgext_rack::fx

#endif // SURGEXTRACK_NIMBUS_H
