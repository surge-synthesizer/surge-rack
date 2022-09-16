#ifndef SURGE_XT_RACK_FXCONFIGHPP
#define SURGE_XT_RACK_FXCONFIGHPP

#include "FX.hpp"
#include "dsp/Effect.h"

namespace sst::surgext_rack::fx
{
// SPRING REVERB
// An extra input for the 'knock' parameter which we send up with a
// schmidt trigger
template <> constexpr int FXConfig<fxt_spring_reverb>::extraInputs() { return 1; }
template <> FXConfig<fxt_spring_reverb>::layout_t FXConfig<fxt_spring_reverb>::getLayout()
{
    // clang-format off
    return {
        {LayoutItem::KNOB16, "SIZE", 0, 12, 28},
        {LayoutItem::KNOB16, "DECAY", 1, 32, 28},
        {LayoutItem::KNOB9, "REFL", 2, 51.48, 20},
        {LayoutItem::KNOB9, "DAMP", 3, 51.48, 36},

        {LayoutItem::PORT, "KNOCK", FX<fxt_spring_reverb>::INPUT_SPECIFIC_0, 9.48, 65},
        {LayoutItem::KNOB9, "SPIN", 4, 23.48, 65},
        {LayoutItem::KNOB9, "CHAOS", 5, 37.48, 65},
        {LayoutItem::KNOB9, "MIX", 7, 51.48, 65},
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

// Gross hack so I can test
template <> FXConfig<fxt_delay>::layout_t FXConfig<fxt_delay>::getLayout()
{
    // clang-format off
    return {
        {LayoutItem::KNOB16, "LEFT", 0, 12, 28},
        {LayoutItem::KNOB16, "RIGHT", 1, 32, 28},
        {LayoutItem::KNOB9, "FBACK", 2, 51.48, 20},
        {LayoutItem::KNOB9, "XFEED", 3, 51.48, 36},

        {LayoutItem::KNOB9, "WIDTH", 11, 37.48, 65},
        {LayoutItem::KNOB9, "MIX", 10, 51.48, 65},
    };

    // clang-format on
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
