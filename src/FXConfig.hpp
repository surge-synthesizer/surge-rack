#ifndef SURGE_XT_RACK_FXCONFIGHPP
#define SURGE_XT_RACK_FXCONFIGHPP

#include "FX.hpp"
#include "dsp/Effect.h"
#include "XTModuleWidget.hpp"

#include "dsp/effects/FrequencyShifterEffect.h"
#include "dsp/effects/DelayEffect.h"
#include "dsp/effects/Reverb2Effect.h"
#include "dsp/effects/RotarySpeakerEffect.h"
#include "dsp/effects/chowdsp/ExciterEffect.h"

namespace sst::surgext_rack::fx
{
// SPRING REVERB
// An extra input for the 'knock' parameter which we send up with a
// schmidt trigger
template <> constexpr int FXConfig<fxt_spring_reverb>::extraInputs() { return 1; }
template <> FXConfig<fxt_spring_reverb>::layout_t FXConfig<fxt_spring_reverb>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto thirdRow = modRow - 16;
    const auto secondRow = thirdRow - 16;
    const auto firstRow = secondRow - 16;
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
        LayoutItem::createGrouplabel("DELAY", col[1], smallRow - 9, widgets::StandardWidthWithModulationConstants::columnWidth_MM * 1.5),
        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
            col[0], smallRow },
        {LayoutItem::KNOB9, "DELAY", FrequencyShifterEffect::freq_delay, col[1], smallRow},
        {LayoutItem::KNOB9, "F/B", FrequencyShifterEffect::freq_feedback, col[2], smallRow},
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

template <> FXConfig<fxt_reverb2>::layout_t FXConfig<fxt_reverb2>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto thirdSmallRow = modRow - 16;
    const auto secondSmallRow = thirdSmallRow - 16;
    const auto bigRow = secondSmallRow - 20;
    const auto endOfPanel = bigRow - 12;

    return {
        // clang-format off
        {LayoutItem::KNOB16, "SIZE", Reverb2Effect::rev2_room_size, (col[0] + col[1]) * 0.5, bigRow},
        {LayoutItem::KNOB16, "DECAY", Reverb2Effect::rev2_decay_time, (col[2] + col[3]) * 0.5, bigRow},

        {LayoutItem::KNOB9, "PRE-D", Reverb2Effect::rev2_predelay, col[0], secondSmallRow},
        {LayoutItem::KNOB9, "DIFFUS", Reverb2Effect::rev2_diffusion, col[1], secondSmallRow},
        {LayoutItem::KNOB9, "MOD", Reverb2Effect::rev2_modulation, col[2], secondSmallRow},
        {LayoutItem::KNOB9, "BUILDUP", Reverb2Effect::rev2_buildup, col[3], secondSmallRow},

        {LayoutItem::KNOB9, "LO", Reverb2Effect::rev2_lf_damping, col[0], thirdSmallRow},
        {LayoutItem::KNOB9, "HI", Reverb2Effect::rev2_hf_damping, col[1], thirdSmallRow},
        {LayoutItem::KNOB9, "WIDTH", Reverb2Effect::rev2_width, col[2], thirdSmallRow},
        {LayoutItem::KNOB9, "MIX", Reverb2Effect::rev2_mix, col[3], thirdSmallRow},

        LayoutItem::createLCDArea(endOfPanel)
        // clang-format on
    };
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
        {LayoutItem::POWER_LIGHT, "", fx_t::FX_SPECIFIC_PARAM_0, col[3], firstRow, 9},
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
