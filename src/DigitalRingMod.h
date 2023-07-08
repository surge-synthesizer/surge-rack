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
#include "SurgeStorage.h"

namespace sst::surgext_rack::digitalrm
{
struct DigitalRingMod : modules::XTModule
{
    enum ParamIds
    {
        TYPE_0,
        TYPE_1,
        LINK_01,
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_0_A_L,
        INPUT_0_A_R,

        INPUT_0_B_L,
        INPUT_0_B_R,

        INPUT_1_A_L,
        INPUT_1_A_R,

        INPUT_1_B_L,
        INPUT_1_B_R,

        NUM_INPUTS

    };
    enum OutputIds
    {
        OUTPUT_0_L,
        OUTPUT_0_R,
        OUTPUT_1_L,
        OUTPUT_1_R,
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

        configSwitch(LINK_01, 0, 1, 0, "Link Second A to First Output", {"Don't Link", "Link"});
        std::vector<std::string> nm;
        for (int i = 0; i < n_cxm_modes; ++i)
            nm.push_back(combinator_mode_names[i]);
        configSwitch(TYPE_0, 0, n_cxm_modes, 0, "CXOR 1 Algorithm", nm);
        configSwitch(TYPE_1, 0, n_cxm_modes, 0, "CXOR 2 Algorithm", nm);

        configInput(INPUT_0_A_L, "CXOR 1 A Left");
        configInput(INPUT_0_A_R, "CXOR 1 A Right");
        configInput(INPUT_1_A_L, "CXOR 2 A Left");
        configInput(INPUT_1_A_R, "CXOR 2 A Right");
        configInput(INPUT_0_B_L, "CXOR 1 B Left");
        configInput(INPUT_0_B_R, "CXOR 1 B Right");
        configInput(INPUT_1_B_L, "CXOR 2 B Left");
        configInput(INPUT_1_B_R, "CXOR 2 B Right");

        configOutput(OUTPUT_0_L, "CXOR 1 Left");
        configOutput(OUTPUT_0_R, "CXOR 1 Right");
        configOutput(OUTPUT_1_L, "CXOR 2 Left");
        configOutput(OUTPUT_1_R, "CXOR 2 Right");
    }

    std::string getName() override { return "Mixer"; }

    void process(const ProcessArgs &args) override {}
};
} // namespace sst::surgext_rack::digitalrm
#endif
