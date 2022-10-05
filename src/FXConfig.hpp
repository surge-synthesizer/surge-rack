#ifndef SURGE_XT_RACK_FXCONFIGHPP
#define SURGE_XT_RACK_FXCONFIGHPP

#include "FX.hpp"
#include "dsp/Effect.h"
#include "XTModuleWidget.hpp"

#include "dsp/effects/FrequencyShifterEffect.h"
#include "dsp/effects/DelayEffect.h"

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

//// NEED RE_DOING NOW WE HAVE PROPER SIZES
// SPRING REVERB
// An extra input for the 'knock' parameter which we send up with a
// schmidt trigger

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

} // namespace sst::surgext_rack::fx

#include "fxconfig/Chorus.h"
#include "fxconfig/Resonator.h"
#include "fxconfig/Reverb2.h"
#include "fxconfig/RotarySpeaker.h"
#include "fxconfig/Exciter.h"
#include "fxconfig/SpringReverb.h"
#include "fxconfig/Distortion.h"
#include "fxconfig/Chow.h"
#endif
