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

#ifndef SURGE_XT_RACK_SRC_DIGITALRINGMOD_H
#define SURGE_XT_RACK_SRC_DIGITALRINGMOD_H

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
