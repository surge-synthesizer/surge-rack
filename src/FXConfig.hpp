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

} // namespace sst::surgext_rack::fx

#include "fxconfig/Chorus.h"
#include "fxconfig/Resonator.h"
#include "fxconfig/Reverb2.h"
#include "fxconfig/RotarySpeaker.h"
#include "fxconfig/Exciter.h"
#include "fxconfig/SpringReverb.h"
#include "fxconfig/Distortion.h"
#include "fxconfig/Chow.h"
#include "fxconfig/Flanger.h"
#include "fxconfig/FrequencyShifter.h"
#include "fxconfig/Ensemble.h"
#include "fxconfig/Neuron.h"
#include "fxconfig/Combulator.h"
#endif
