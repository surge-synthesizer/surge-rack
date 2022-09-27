#ifndef SURGE_XT_RACK_FXCONFIGHPP
#define SURGE_XT_RACK_FXCONFIGHPP

#include "FX.hpp"
#include "dsp/Effect.h"
#include "XTModuleWidget.hpp"

#include "dsp/effects/FrequencyShifterEffect.h"
#include "dsp/effects/ChorusEffect.h"
#include "dsp/effects/DelayEffect.h"
#include "dsp/effects/Reverb2Effect.h"
#include "dsp/effects/RotarySpeakerEffect.h"
#include "dsp/effects/chowdsp/ExciterEffect.h"
#include "dsp/effects/ResonatorEffect.h"

namespace sst::surgext_rack::fx
{

struct FXLayoutHelper
{
    static constexpr float rowStart_MM = 71;
    static constexpr float unlabeledGap_MM = 16;
    static constexpr float labeledGap_MM = 20;

    static constexpr float bigCol0 =
        widgets::StandardWidthWithModulationConstants::columnCenters_MM[1] - 7;
    static constexpr float bigCol1 =
        widgets::StandardWidthWithModulationConstants::columnCenters_MM[2] + 7;

    static constexpr float knobGap16_MM = unlabeledGap_MM - 9 + 18;

    template <typename T> inline static void processExtend(T *m, int surgePar, int rackPar)
    {
        auto &p = m->fxstorage->p[surgePar];

        auto er = p.extend_range;
        auto per = m->params[rackPar].getValue() > 0.5;
        if (er != per)
        {
            p.set_extend_range(per);
        }
    }
    template <typename T> inline static void processDeactivate(T *m, int surgePar, int rackPar)
    {
        auto &p = m->fxstorage->p[surgePar];

        auto lce = !p.deactivated;
        auto plce = m->params[rackPar].getValue() > 0.5;
        if (lce != plce)
        {
            p.deactivated = !plce;
        }
    }
};

/*
 * CHORUS:
 * One extra input for a clock
 * Two extra params for power on the lo and hi cut
 */
template <> constexpr bool FXConfig<fxt_chorus4>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_chorus4>::specificParamCount() { return 2; }

template <> FXConfig<fxt_chorus4>::layout_t FXConfig<fxt_chorus4>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;

    const auto outputRow = FXLayoutHelper::rowStart_MM;
    const auto delayRow = outputRow - FXLayoutHelper::labeledGap_MM;
    const auto bigRow = delayRow - FXLayoutHelper::knobGap16_MM;

    const auto endOfPanel = bigRow - 8;

    typedef FX<fxt_chorus4> fx_t;
    typedef ChorusEffect<4> sfx_t;

    // fixme use the enums
    // clang-format off
    return {
        {LayoutItem::KNOB16, "RATE", sfx_t::ch_rate, FXLayoutHelper::bigCol0, bigRow},
        {LayoutItem::KNOB16, "DEPTH", sfx_t::ch_depth, FXLayoutHelper::bigCol1, bigRow},

        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
                 col[0], delayRow },
        {LayoutItem::KNOB9, "TIME", sfx_t::ch_time, col[2], delayRow},
        {LayoutItem::KNOB9, "F/BACK", sfx_t::ch_feedback, col[3], delayRow},
        LayoutItem::createGrouplabel("DELAY", col[2], delayRow, 2),

        {LayoutItem::KNOB9, "LOCUT", sfx_t::ch_lowcut, col[0], outputRow},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[0], outputRow, -1},

        {LayoutItem::KNOB9, "HICUT", sfx_t::ch_highcut, col[1], outputRow},
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0+1, col[1], outputRow, +1},
        LayoutItem::createGrouplabel("EQ", col[0], outputRow, 2).withExtra("SHORTLEFT",1).withExtra("SHORTRIGHT",1),

        {LayoutItem::KNOB9, "WIDTH", sfx_t::ch_width, col[2], outputRow},
        {LayoutItem::KNOB9, "MIX", sfx_t::ch_mix, col[3], outputRow},
        LayoutItem::createGrouplabel("OUTPUT", col[2], outputRow, 2),

        LayoutItem::createPresetLCDArea(),
    };

    // clang-format on
}

template <> void FXConfig<fxt_chorus4>::configSpecificParams(FX<fxt_chorus4> *m)
{
    typedef FX<fxt_chorus4> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 1, "Enable LowCut");
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0 + 1, 0, 1, 1, "Enable HiCut");
}

template <> void FXConfig<fxt_chorus4>::processSpecificParams(FX<fxt_chorus4> *m)
{
    typedef FX<fxt_chorus4> fx_t;
    FXLayoutHelper::processDeactivate(m, ChorusEffect<4>::ch_lowcut, fx_t::FX_SPECIFIC_PARAM_0);
    FXLayoutHelper::processDeactivate(m, ChorusEffect<4>::ch_highcut,
                                      fx_t::FX_SPECIFIC_PARAM_0 + 1);
}

template <>
void FXConfig<fxt_chorus4>::loadPresetOntoSpecificParams(
    FX<fxt_chorus4> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_chorus4> fx_t;
    typedef ChorusEffect<4> sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.da[sx_t::ch_lowcut] ? 0 : 1);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.da[sx_t::ch_highcut] ? 0 : 1);
}

template <>
bool FXConfig<fxt_chorus4>::isDirtyPresetVsSpecificParams(
    FX<fxt_chorus4> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_chorus4> fx_t;
    typedef ChorusEffect<4> sx_t;
    auto p0 = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    auto p1 = m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].getValue() > 0.5;
    return !(p0 == !ps.da[sx_t::ch_lowcut] && p1 == !ps.da[sx_t::ch_highcut]);
}

/*
 * END OF CHORUS
 */

/*
 * RESONATOR - Each of the bands is extendible
 * - TODO : 2 include extend and activate in preset dirty scan and load
 */

template <> constexpr int FXConfig<fxt_resonator>::specificParamCount() { return 3; }
template <> FXConfig<fxt_resonator>::layout_t FXConfig<fxt_resonator>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;

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

        LayoutItem::createLCDArea(endOfPanel),

        {LayoutItem::LCD_MENU_ITEM, "MODE", sfx_t::resonator_mode, 0, endOfPanel},
    };

    // clang-format on
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
    typedef FX<fxt_chorus4> fx_t;
    typedef ResonatorEffect sx_t;
    m->params[fx_t::FX_SPECIFIC_PARAM_0].setValue(ps.er[sx_t::resonator_freq1]);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 1].setValue(ps.er[sx_t::resonator_freq2]);
    m->params[fx_t::FX_SPECIFIC_PARAM_0 + 2].setValue(ps.er[sx_t::resonator_freq3]);
}

template <>
bool FXConfig<fxt_resonator>::isDirtyPresetVsSpecificParams(
    FX<fxt_resonator> *m, const Surge::Storage::FxUserPreset::Preset &ps)
{
    typedef FX<fxt_chorus4> fx_t;
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

/*
 * REVERB2
 */
template <> FXConfig<fxt_reverb2>::layout_t FXConfig<fxt_reverb2>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;

    const auto row3 = FXLayoutHelper::rowStart_MM;
    const auto row2 = row3 - FXLayoutHelper::labeledGap_MM;
    const auto row1 = row2 - FXLayoutHelper::knobGap16_MM;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "SIZE", Reverb2Effect::rev2_room_size, FXLayoutHelper::bigCol0, row1},
        {LayoutItem::KNOB16, "DECAY", Reverb2Effect::rev2_decay_time, FXLayoutHelper::bigCol1, row1},

        {LayoutItem::KNOB9, "PRE-D", Reverb2Effect::rev2_predelay, col[0], row2},
        {LayoutItem::KNOB9, "DIFFUS", Reverb2Effect::rev2_diffusion, col[1], row2},
        {LayoutItem::KNOB9, "MOD", Reverb2Effect::rev2_modulation, col[2], row2},
        {LayoutItem::KNOB9, "BUILDUP", Reverb2Effect::rev2_buildup, col[3], row2},
        LayoutItem::createGrouplabel("REVERB", col[1], row2, 3),

        {LayoutItem::KNOB9, "LO", Reverb2Effect::rev2_lf_damping, col[0], row3},
        {LayoutItem::KNOB9, "HI", Reverb2Effect::rev2_hf_damping, col[1], row3},
        LayoutItem::createGrouplabel("EQ", col[0], row3, 2),
        {LayoutItem::KNOB9, "WIDTH", Reverb2Effect::rev2_width, col[2], row3},
        {LayoutItem::KNOB9, "MIX", Reverb2Effect::rev2_mix, col[3], row3},
        LayoutItem::createGrouplabel("OUTPUT", col[2], row3, 2),


        LayoutItem::createPresetLCDArea(),
        // clang-format on
    };
}
/*
 * END OF REVERB2
 */
// next up:
// do: reverb 1
// spring reverb (started below)
// ensemble (like resonator)

//// NEED RE_DOING NOW WE HAVE PROPER SIZES
// SPRING REVERB
// An extra input for the 'knock' parameter which we send up with a
// schmidt trigger
template <> constexpr int FXConfig<fxt_spring_reverb>::extraInputs() { return 1; }
template <> FXConfig<fxt_spring_reverb>::layout_t FXConfig<fxt_spring_reverb>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto thirdRow = 71;
    const auto secondRow = thirdRow - FXLayoutHelper::labeledGap_MM;
    const auto firstRow = secondRow - FXLayoutHelper::unlabeledGap_MM;
    const auto bigRow = (secondRow + firstRow) * 0.5f;

    const auto endOfPanel = firstRow - 7;

    // fixme use the enums
    // clang-format off
    return {
        {LayoutItem::KNOB16, "SIZE", 0, col[1] - 10, bigRow},
        {LayoutItem::KNOB16, "DECAY", 1, col[1] + 10, bigRow},
        {LayoutItem::KNOB9, "REFL", 2, col[3], firstRow},
        {LayoutItem::KNOB9, "DAMP", 3, col[3], secondRow},

        {LayoutItem::PORT, "KNOCK", FX<fxt_spring_reverb>::INPUT_SPECIFIC_0, col[0], thirdRow},
        {LayoutItem::KNOB9, "SPIN", 4, col[1], thirdRow},
        {LayoutItem::KNOB9, "CHAOS", 5, col[2], thirdRow},
        LayoutItem::createGrouplabel("MODULATION", col[0], thirdRow, 3),
        {LayoutItem::KNOB9, "MIX", 7, col[3], thirdRow},
        LayoutItem::createLCDArea(endOfPanel),
    };

    // clang-format on
}

template <> void FXConfig<fxt_spring_reverb>::processExtraInputs(FX<fxt_spring_reverb> *that)
{
    if (that->inputs[FX<fxt_spring_reverb>::INPUT_SPECIFIC_0].isConnected())
    {
        if (that->inputs[FX<fxt_spring_reverb>::INPUT_SPECIFIC_0].getVoltage() > 3)
        {
            that->fxstorage->p[6].set_value_f01(1);
        }
        else
        {
            that->fxstorage->p[6].set_value_f01(0);
        }
    }
}

template <> FXConfig<fxt_delay>::layout_t FXConfig<fxt_delay>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto thirdSmallRow = modRow - 16;
    const auto secondSmallRow = thirdSmallRow - 16;
    const auto firstSmallRow = secondSmallRow - 16;
    const auto zeroSmallRow = firstSmallRow - 16;
    const auto bigRow = (firstSmallRow + zeroSmallRow) * 0.5f;

    const auto endOfPanel = zeroSmallRow - 8;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "LEFT", DelayEffect::dly_time_left, col[1] - 10, bigRow},
        {LayoutItem::KNOB16, "RIGHT", DelayEffect::dly_time_right, col[1] + 10, bigRow},

        {LayoutItem::KNOB9, "LOWCUT", DelayEffect::dly_lowcut, col[3], zeroSmallRow},
        {LayoutItem::KNOB9, "HICUT", DelayEffect::dly_highcut, col[3], firstSmallRow},

        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
            col[0], secondSmallRow },
        {LayoutItem::KNOB9, "INPUT", DelayEffect::dly_input_channel, col[1], secondSmallRow},
        {LayoutItem::KNOB9, "RATE", DelayEffect::dly_mod_rate, col[2], secondSmallRow},
        {LayoutItem::KNOB9, "DEPTH", DelayEffect::dly_mod_depth, col[3], secondSmallRow},

        {LayoutItem::KNOB9, "F/B", DelayEffect::dly_feedback, col[0], thirdSmallRow},
        {LayoutItem::KNOB9, "XFEED", DelayEffect::dly_crossfeed, col[1], thirdSmallRow},

        {LayoutItem::KNOB9, "WIDTH", DelayEffect::dly_width, col[2], thirdSmallRow},
        {LayoutItem::KNOB9, "MIXT", DelayEffect::dly_mix, col[3], thirdSmallRow},
        // clang-format on
    };
}
template <> constexpr bool FXConfig<fxt_delay>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_freqshift>::specificParamCount() { return 1; }
template <> FXConfig<fxt_freqshift>::layout_t FXConfig<fxt_freqshift>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto smallRow = modRow - 16;
    const auto bigRow = smallRow - 26;
    const auto endOfPanel = bigRow - 12;

    typedef FX<fxt_freqshift> fx_t;

    return {
        // clang-format off
        //LayoutItem::createGrouplabel("DELAY", col[1], smallRow - 9, widgets::StandardWidthWithModulationConstants::columnWidth_MM * 1.5),
        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
            col[0], smallRow },
        {LayoutItem::KNOB9, "TIME", FrequencyShifterEffect::freq_delay, col[1], smallRow},
        {LayoutItem::KNOB9, "F/B", FrequencyShifterEffect::freq_feedback, col[2], smallRow},
        //LayoutItem::createGrouplabel("DELAY", col[0], smallRow, 3),
        {LayoutItem::KNOB9, "MIX", FrequencyShifterEffect::freq_mix, col[3], smallRow},
        {LayoutItem::KNOB16, "LEFT", FrequencyShifterEffect::freq_shift, (col[0] + col[1]) * 0.5, bigRow},
        {LayoutItem::EXTEND_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, (col[0] + col[1]) * 0.5, bigRow, 16},
        {LayoutItem::KNOB16, "RIGHT", FrequencyShifterEffect::freq_rmult, (col[2] + col[3]) * 0.5, bigRow},
        LayoutItem::createLCDArea(endOfPanel),
        // clang-format on
    };
}
template <> constexpr bool FXConfig<fxt_freqshift>::usesClock() { return true; }
template <> void FXConfig<fxt_freqshift>::configSpecificParams(FX<fxt_freqshift> *m)
{
    typedef FX<fxt_freqshift> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 0, "Extend Frequency");

    // Limit max delay to 1second by setting the upper bound to '0' in 2^x land
    auto &p = m->fxstorage->p[FrequencyShifterEffect::freq_delay];
    m->paramQuantities[fx_t::FX_PARAM_0 + FrequencyShifterEffect::freq_delay]->maxValue =
        -p.val_min.f / (p.val_max.f - p.val_min.f);
}

template <> void FXConfig<fxt_freqshift>::processSpecificParams(FX<fxt_freqshift> *m)
{
    typedef FX<fxt_freqshift> fx_t;
    auto ext = m->fxstorage->p[FrequencyShifterEffect::freq_shift].extend_range;
    auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;

    if (parVa != ext)
    {
        m->fxstorage->p[FrequencyShifterEffect::freq_shift].set_extend_range(parVa);
    }
}

template <> FXConfig<fxt_exciter>::layout_t FXConfig<fxt_exciter>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto bottomRow = modRow - 16;
    const auto topRow = bottomRow - 16;
    const auto bigRow = (bottomRow + topRow) * 0.5f;

    const auto endOfPanel = topRow - 8;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "DRIVE", chowdsp::ExciterEffect::exciter_drive, (col[1]+col[0]) * 0.5, bigRow},

        {LayoutItem::KNOB9, "ATTACK", chowdsp::ExciterEffect::exciter_att, col[2], topRow},
        {LayoutItem::KNOB9, "RELEASE", chowdsp::ExciterEffect::exciter_rel, col[3], topRow},

        {LayoutItem::KNOB9, "TONE", chowdsp::ExciterEffect::exciter_tone, col[2], bottomRow},
        {LayoutItem::KNOB9, "MIX", chowdsp::ExciterEffect::exciter_mix, col[3], bottomRow},
        LayoutItem::createLCDArea(endOfPanel)
        // clang-format on
    };
}

/*
 * ROTARY SPEAKER
 * - uses clock
 * - one specific param which is 'enables drive'
 */
template <> constexpr bool FXConfig<fxt_rotaryspeaker>::usesClock() { return true; }
template <> constexpr int FXConfig<fxt_rotaryspeaker>::specificParamCount() { return 1; }
template <> FXConfig<fxt_rotaryspeaker>::layout_t FXConfig<fxt_rotaryspeaker>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto thirdRow = modRow - 16;
    const auto secondRow = thirdRow - 16;
    const auto firstRow = secondRow - 16;
    const auto bigRow = (secondRow + firstRow) * 0.5f;

    const auto endOfPanel = firstRow - 7;

    // ToDo: On Off for drive and Rrive as a selected type
    typedef FX<fxt_rotaryspeaker> fx_t;

    // clang-format off
    return {
        {LayoutItem::KNOB16, "HORN", RotarySpeakerEffect::rot_horn_rate, col[1] - 10, bigRow},
        {LayoutItem::KNOB16, "ROTOR", RotarySpeakerEffect::rot_rotor_rate, col[1] + 10, bigRow},
        {LayoutItem::KNOB9, "DRIVE", RotarySpeakerEffect::rot_drive, col[3], firstRow},
        //{LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[3], firstRow, 9},
        {LayoutItem::PORT, "CLOCK", fx_t ::INPUT_CLOCK, col[3], secondRow},

        {LayoutItem::KNOB9, "DOPPLER", RotarySpeakerEffect::rot_doppler, col[0], thirdRow},
        {LayoutItem::KNOB9, "TREMOLO", RotarySpeakerEffect::rot_tremolo, col[1], thirdRow},
        {LayoutItem::KNOB9, "WIDTH", RotarySpeakerEffect::rot_width, col[2], thirdRow},
        {LayoutItem::KNOB9, "MIX", RotarySpeakerEffect::rot_mix, col[3], thirdRow},

        LayoutItem::createLCDArea(endOfPanel),
        {LayoutItem::LCD_MENU_ITEM, "MODEL", RotarySpeakerEffect::rot_waveshape, 0, endOfPanel}
    };

    // clang-format on
}

template <> void FXConfig<fxt_rotaryspeaker>::configSpecificParams(FX<fxt_rotaryspeaker> *m)
{
    typedef FX<fxt_rotaryspeaker> fx_t;
    m->configParam(fx_t::FX_SPECIFIC_PARAM_0, 0, 1, 1, "Enable Drive");
}

template <> void FXConfig<fxt_rotaryspeaker>::processSpecificParams(FX<fxt_rotaryspeaker> *m)
{
    typedef FX<fxt_rotaryspeaker> fx_t;
    auto drOff = m->fxstorage->p[RotarySpeakerEffect::rot_drive].deactivated;
    auto parVa = m->params[fx_t::FX_SPECIFIC_PARAM_0].getValue() > 0.5;
    if (parVa != (!drOff))
    {
        m->fxstorage->p[RotarySpeakerEffect::rot_drive].deactivated = !parVa;
    }
}
} // namespace sst::surgext_rack::fx
#endif
