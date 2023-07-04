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

#include "dsp/effects/chowdsp/CHOWEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_CHOW_H
#define SURGE_XT_RACK_SRC_FXCONFIG_CHOW_H

namespace sst::surgext_rack::fx
{

template <> constexpr int FXConfig<fxt_chow>::numParams() { return 4; }
template <> constexpr int FXConfig<fxt_chow>::extraInputs() { return 1; }
template <> constexpr int FXConfig<fxt_chow>::specificParamCount() { return 1; }

template <> FXConfig<fxt_chow>::layout_t FXConfig<fxt_chow>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();
    const auto modRow = layout::LayoutConstants::modulationRowCenters_MM[0];

    const auto bottomRow = FXLayoutHelper::rowStart_MM;
    const auto driveRow = bottomRow - 34;
    const auto colC = (col[1] + col[2]) * 0.5f;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "THRESHOLD", chowdsp::CHOWEffect::chow_thresh, colC, driveRow},

        {LayoutItem::PORT, "TRIG", FX<fxt_chow>::INPUT_SPECIFIC_0, col[0], bottomRow},
        {LayoutItem::TOGGLE_PARAM, "FLIP", FX<fxt_spring_reverb>::FX_SPECIFIC_PARAM_0, col[1], bottomRow},

        LayoutItem::createGrouplabel("FLIP", col[0], bottomRow, 2),

        {LayoutItem::KNOB9, "RATIO", chowdsp::CHOWEffect::chow_ratio, col[2], bottomRow},
        {LayoutItem::KNOB9, "MIX", chowdsp::CHOWEffect::chow_mix, col[3], bottomRow},
        LayoutItem::createPresetLCDArea()
        // clang-format on
    };
}

template <> void FXConfig<fxt_chow>::configSpecificParams(FX<fxt_chow> *m)
{
    typedef FX<fxt_chow> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0, 0, "Flip It Good");
}

template <> void FXConfig<fxt_chow>::configExtraInputs(FX<fxt_chow> *m)
{
    m->configInput(FX<fxt_chow>::INPUT_SPECIFIC_0, "Gate to Flip It!");
}

/*
 * We don't actually process the param separately since the extra inputs check looks at both
 * template <> void FXConfig<fxt_chow>::processSpecificParams(FX<fxt_chow> *m)
 */

template <> void FXConfig<fxt_chow>::processExtraInputs(FX<fxt_chow> *that)
{
    auto t = that->inputs[FX<fxt_chow>::INPUT_SPECIFIC_0].getVoltage() > 3;
    auto d = that->params[FX<fxt_chow>::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    if (t || d)
    {
        that->fxstorage->p[chowdsp::CHOWEffect::chow_flip].set_value_f01(1);
    }
    else
    {
        that->fxstorage->p[chowdsp::CHOWEffect::chow_flip].set_value_f01(0);
    }
}
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_ROTARYSPEAKER_H
