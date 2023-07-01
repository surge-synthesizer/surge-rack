/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Copyright 2019 - 2022, Various authors, as described in the github
 * transaction log.
 *
 * SurgeXT for VCV Rack is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef XTRACK_digitalrm_HPP
#define XTRACK_digitalrm_HPP

#include "SurgeXT.h"
#include "XTModule.h"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"
#include "DSPUtils.h"

#include "sst/basic-blocks/dsp/CorrelatedNoise.h"
#include "CXOR.h"

namespace sst::surgext_rack::digitalrm
{
struct DigitalRingMod : modules::XTModule
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_A_L,
        INPUT_A_R,

        INPUT_B_L,
        INPUT_B_R,

        NUM_INPUTS

    };
    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    DigitalRingMod() : XTModule()
    {
        std::lock_guard<std::mutex> ltg(xtSurgeCreateMutex);

        setupSurgeCommon(NUM_PARAMS, false, false);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    std::string getName() override { return "Mixer"; }

    void process(const ProcessArgs &args) override {}
};
} // namespace sst::surgext_rack::digitalrm
#endif
