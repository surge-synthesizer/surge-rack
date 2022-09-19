#ifndef SURGE_XT_RACK_FXCONFIGHPP
#define SURGE_XT_RACK_FXCONFIGHPP

#include "FX.hpp"
#include "dsp/Effect.h"
#include "XTModuleWidget.hpp"

#include "dsp/effects/FrequencyShifterEffect.h"
#include "dsp/effects/DelayEffect.h"
#include "dsp/effects/Reverb2Effect.h"

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
    const auto bigRow = firstSmallRow - 20;

    return {
        // clang-format off
        {LayoutItem::KNOB12, "LEFT", DelayEffect::dly_time_left, (col[0] + col[1]) * 0.5, bigRow},
        {LayoutItem::KNOB12, "RIGHT", DelayEffect::dly_time_right, (col[2] + col[3]) * 0.5, bigRow},

        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
            col[0], firstSmallRow },
        {LayoutItem::KNOB9, "INPUT", DelayEffect::dly_input_channel, col[1], firstSmallRow},
        {LayoutItem::KNOB9, "RATE", DelayEffect::dly_mod_rate, col[2], firstSmallRow},
        {LayoutItem::KNOB9, "DEPTH", DelayEffect::dly_mod_depth, col[3], firstSmallRow},

        {LayoutItem::KNOB9, "F/B", DelayEffect::dly_feedback, col[0], secondSmallRow},
        {LayoutItem::KNOB9, "XFEED", DelayEffect::dly_crossfeed, col[1], secondSmallRow},
        {LayoutItem::KNOB9, "LOWCUT", DelayEffect::dly_lowcut, col[2], secondSmallRow},
        {LayoutItem::KNOB9, "HICUT", DelayEffect::dly_highcut, col[3], secondSmallRow},

        {LayoutItem::KNOB9, "WIDTH", DelayEffect::dly_width, col[2], thirdSmallRow},
        {LayoutItem::KNOB9, "MIXT", DelayEffect::dly_mix, col[3], thirdSmallRow},
        // clang-format on
    };
}
template <> constexpr int FXConfig<fxt_delay>::usesClock() { return true; }

template <> FXConfig<fxt_freqshift>::layout_t FXConfig<fxt_freqshift>::getLayout()
{
    const auto &col = widgets::StandardWidthWithModulationConstants::columnCenters_MM;
    const auto modRow = widgets::StandardWidthWithModulationConstants::modulationRowCenters_MM[0];

    const auto smallRow = modRow - 16;
    const auto bigRow = smallRow - 22;
    const auto endOfPanel = bigRow - 12;

    return {
        // clang-format off
        {LayoutItem::PORT, "CLOCK", FX<fxt_delay>::INPUT_CLOCK,
            col[0], smallRow },
        {LayoutItem::KNOB9, "DELAY", FrequencyShifterEffect::freq_delay, col[1], smallRow},
        {LayoutItem::KNOB9, "F/B", FrequencyShifterEffect::freq_feedback, col[2], smallRow},
        {LayoutItem::KNOB9, "MIX", FrequencyShifterEffect::freq_mix, col[3], smallRow},
        {LayoutItem::KNOB16, "LEFT", FrequencyShifterEffect::freq_shift, (col[0] + col[1]) * 0.5, bigRow},
        {LayoutItem::KNOB16, "RIGHT", FrequencyShifterEffect::freq_rmult, (col[2] + col[3]) * 0.5, bigRow},
        LayoutItem::createLCDArea(endOfPanel),
        // clang-format on
    };
}
template <> constexpr int FXConfig<fxt_freqshift>::usesClock() { return true; }

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
#if 0
// Top Row is Grouped
group header span 4 "reverb"
{0, "SIZE"},
{1, "DECAY"},
{2, "REFLECT"},
{3, "DAMP"},FX

// next row
group header span 3 "modulation"
{INPUT EXTRA 1, "KNOCK"},
{4, "SPIN"}
{5, "CHAOS"}
{7, "MIX"}
#endif
} // namespace sst::surgext_rack::fx
#endif
