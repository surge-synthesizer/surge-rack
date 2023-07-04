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

#include "dsp/effects/ResonatorEffect.h"

#ifndef SURGE_XT_RACK_SRC_FXCONFIG_RESONATOR_H
#define SURGE_XT_RACK_SRC_FXCONFIG_RESONATOR_H

namespace sst::surgext_rack::fx
{

/*
 * RESONATOR - Each of the bands is extendible
 * - TODO : 2 include extend and activate in preset dirty scan and load
 */

template <> constexpr int FXConfig<fxt_resonator>::specificParamCount() { return 3; }
template <> FXConfig<fxt_resonator>::layout_t FXConfig<fxt_resonator>::getLayout()
{
    const auto col = FXLayoutHelper::standardColumns_MM();

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::labeledGap_MM;

    const auto endOfPanel = row1 - FXLayoutHelper::labeledGap_MM * 0.5f - 2;

    typedef FX<fxt_resonator> fx_t;
    typedef ResonatorEffect sfx_t;

    // fixme use the enums
    // clang-format off
    return {
        {LayoutItem::KNOB9, "FREQ", sfx_t::resonator_freq1, col[0], row1},
        {LayoutItem::EXTEND_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], row1, +1},
        {LayoutItem::KNOB9, "RES", sfx_t::resonator_res1, col[1], row1},
        {LayoutItem::KNOB9, "GAIN", sfx_t::resonator_gain1, col[2], row1},
        LayoutItem::createGrouplabel("BAND 1", col[0], row1, 3),

       {LayoutItem::KNOB9, "FREQ", sfx_t::resonator_freq2, col[0], row2},
       {LayoutItem::EXTEND_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[0], row2, +1},
       {LayoutItem::KNOB9, "RES", sfx_t::resonator_res2, col[1], row2},
       {LayoutItem::KNOB9, "GAIN", sfx_t::resonator_gain2, col[2], row2},
       LayoutItem::createGrouplabel("BAND 2", col[0], row2, 3),

       {LayoutItem::KNOB9, "FREQ", sfx_t::resonator_freq3, col[0], row3},
       {LayoutItem::EXTEND_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+2, col[0], row3, +1},
       {LayoutItem::KNOB9, "RES", sfx_t::resonator_res3, col[1], row3},
       {LayoutItem::KNOB9, "GAIN", sfx_t::resonator_gain3, col[2], row3},
       LayoutItem::createGrouplabel("BAND 3", col[0], row3, 3),

        {LayoutItem::KNOB9, "OUTPUT", sfx_t::resonator_gain, col[3], row2},
        {LayoutItem::KNOB9, "MIX", sfx_t::resonator_mix, col[3], row3},

        LayoutItem::createPresetPlusOneArea(),

        {LayoutItem::LCD_MENU_ITEM_SURGE_PARAM, "MODE", sfx_t::resonator_mode, 0, endOfPanel},
    };

    // clang-format on
}

template <> void FXConfig<fxt_resonator>::configSpecificParams(FX<fxt_resonator> *m)
{
    typedef FX<fxt_resonator> fx_t;
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 0, 0, "Extend Band 1 Frequency");
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 1, 0, "Extend Band 2 Frequency");
    m->configOnOff(fx_t::FX_SPECIFIC_PARAM_0 + 2, 0, "Extend Band 3 Frequency");
}
template <> void FXConfig<fxt_resonator>::processSpecificParams(FX<fxt_resonator> *m)
{
    typedef FX<fxt_resonator> fx_t;
    FXLayoutHelper::processExtend(m, ResonatorEffect::resonator_freq1, fx_t::FX_SPECIFIC_PARAM_0);
    FXLayoutHelper::processExtend(m, ResonatorEffect::resonator_freq2,
                                  fx_t::FX_SPECIFIC_PARAM_0 + 1);
    FXLayoutHelper::processExtend(m, ResonatorEffect::resonator_freq3,
                                  fx_t::FX_SPECIFIC_PARAM_0 + 2);
}

template <>
void FXConfig<fxt_resonator>::loadPresetOntoSpecificParams(
    FX<fxt_resonator> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_resonator> fx_t;
    typedef ResonatorEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.er[sx_t::resonator_freq1]);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.er[sx_t::resonator_freq2]);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 2].setValue(ps.er[sx_t::resonator_freq3]);
}

template <>
bool FXConfig<fxt_resonator>::isDirtyPresetVsSpecificParams(
    FX<fxt_resonator> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_resonator> fx_t;
    typedef ResonatorEffect sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    auto p1 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
    auto p2 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 2].getValue() > 0.5;
    return !(p0 == ps.er[sx_t::resonator_freq1] && p1 == ps.er[sx_t::resonator_freq2] &&
             p2 == ps.er[sx_t::resonator_freq3]);
}

/*
 * END OF RESONATOR
 */
} // namespace sst::surgext_rack::fx
#endif // SURGEXT_RACK_FX_RESONATOR_H
