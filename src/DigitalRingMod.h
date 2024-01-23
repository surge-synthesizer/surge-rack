/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
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
#include <sst/filters/HalfRateFilter.h>
#include "DebugHelpers.h"
#include "globals.h"
#include "DSPUtils.h"

#include "sst/basic-blocks/mechanics/block-ops.h"
#include "CXOR.h"
#include "SurgeStorage.h"
#include <sst/rackhelpers/neighbor_connectable.h>

namespace sst::surgext_rack::digitalrm
{
struct DigitalRingMod : modules::XTModule,
                        sst::rackhelpers::module_connector::NeighborConnectable_V1
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

        auto linkS =
            configSwitch(LINK_01, 0, 1, 0, "Link Second A to First Output", {"Don't Link", "Link"});
        linkS->randomizeEnabled = false;
        std::vector<std::string> nm;
        for (int i = 0; i < n_cxm_modes; ++i)
            nm.push_back(combinator_mode_names[i]);
        configSwitch(TYPE_0, 0, n_cxm_modes - 1, 0, "CXOR 1 Algorithm", nm);
        configSwitch(TYPE_1, 0, n_cxm_modes - 1, 0, "CXOR 2 Algorithm", nm);

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

        configBypass(INPUT_0_A_L, OUTPUT_0_L);
        configBypass(INPUT_0_A_R, OUTPUT_0_R);
        configBypass(INPUT_1_A_L, OUTPUT_1_L);
        configBypass(INPUT_1_A_R, OUTPUT_1_R);

        memset(inputA, 0, sizeof(inputA));
        memset(inputB, 0, sizeof(inputB));
        memset(output, 0, sizeof(output));
        blockPos = 0;
        for (int inst = 0; inst < 2; ++inst)
        {
            for (int i = 0; i < MAX_POLY; ++i)
            {
                halfbandOut[inst][i] =
                    std::make_unique<sst::filters::HalfRate::HalfRateFilter>(6, true);
                halfbandOut[inst][i]->reset();

                halfbandInA[inst][i] =
                    std::make_unique<sst::filters::HalfRate::HalfRateFilter>(6, true);
                halfbandInA[inst][i]->reset();

                halfbandInB[inst][i] =
                    std::make_unique<sst::filters::HalfRate::HalfRateFilter>(6, true);
                halfbandInB[inst][i]->reset();
            }
        }
    }

    std::string getName() override { return "Mixer"; }

    void process(const ProcessArgs &args) override
    {
        namespace mech = sst::basic_blocks::mechanics;

        if (blockPos == blockSize)
        {
            /* Figure out polyphony */
            aMono[0] = !inputs[INPUT_0_A_R].isConnected();
            aMono[1] = !inputs[INPUT_1_A_R].isConnected();
            bMono[0] = !inputs[INPUT_0_B_R].isConnected();
            bMono[1] = !inputs[INPUT_1_B_R].isConnected();
            auto tpoly0 =
                std::max({inputs[INPUT_0_A_L].getChannels(), inputs[INPUT_0_A_R].getChannels(),
                          inputs[INPUT_0_B_L].getChannels(), inputs[INPUT_0_B_R].getChannels()});
            auto tpoly1 =
                std::max({inputs[INPUT_1_A_L].getChannels(), inputs[INPUT_1_A_R].getChannels(),
                          inputs[INPUT_1_B_L].getChannels(), inputs[INPUT_1_B_R].getChannels()});

            if (poly[0] != tpoly0)
            {
                auto inst = 0;
                for (int i = poly[0]; i < tpoly0; ++i)
                {
                    halfbandOut[inst][i]->reset();
                    halfbandInA[inst][i]->reset();
                    halfbandInB[inst][i]->reset();
                }
            }

            bool isLink = params[LINK_01].getValue() > 0.5;
            if (isLink)
            {
                tpoly1 = std::max(tpoly0, tpoly1);
                memcpy(&inputA[1], &inputA[0], sizeof(inputA[0]));
            }

            if (poly[1] != tpoly1)
            {
                auto inst = 1;
                for (int i = poly[1]; i < tpoly0; ++i)
                {
                    halfbandOut[inst][i]->reset();
                    halfbandInA[inst][i]->reset();
                    halfbandInB[inst][i]->reset();
                }
            }

            poly[0] = tpoly0;
            poly[1] = tpoly1;

            outputs[OUTPUT_0_L].setChannels(poly[0]);
            outputs[OUTPUT_0_R].setChannels(poly[0]);

            outputs[OUTPUT_1_L].setChannels(poly[1]);
            outputs[OUTPUT_1_R].setChannels(poly[1]);

            for (int inst = 0; inst < 2; ++inst)
            {
                auto mode = (CombinatorMode)std::round(params[TYPE_0 + inst].getValue());
                for (int p = 0; p < poly[inst]; ++p)
                {
                    float inAOS alignas(16)[2][blockSize << 1];
                    float inBOS alignas(16)[2][blockSize << 1];
                    float outOS alignas(16)[2][blockSize << 1];

                    // Halfband up A and B
                    halfbandInA[inst][p]->process_block_U2(inputA[inst][p][0], inputA[inst][p][1],
                                                           inAOS[0], inAOS[1], blockSizeOS);
                    halfbandInB[inst][p]->process_block_U2(inputB[inst][p][0], inputB[inst][p][1],
                                                           inBOS[0], inBOS[1], blockSizeOS);

                    float *src1_l = &inAOS[0][0];
                    float *src1_r = &inAOS[1][0];
                    float *src2_l = &inBOS[0][0];
                    float *src2_r = &inBOS[1][0];
                    auto nquads = blockSizeOS >> 2;
                    float *dst_l = &outOS[0][0];
                    float *dst_r = &outOS[1][0];

                    switch (mode)
                    {
                    case CombinatorMode::cxm_ring:
                        mech::mul_block<BLOCK_SIZE_OS>(src1_l, src2_l, dst_l);
                        mech::mul_block<BLOCK_SIZE_OS>(src1_r, src2_r, dst_r);
                        break;
                    case CombinatorMode::cxm_cxor43_0:
                        cxor43_0_block(src1_l, src2_l, dst_l, nquads);
                        cxor43_0_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor43_1:
                        cxor43_1_block(src1_l, src2_l, dst_l, nquads);
                        cxor43_1_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor43_2:
                        cxor43_2_block(src1_l, src2_l, dst_l, nquads);
                        cxor43_2_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor43_3:
                        cxor43_3_block(src1_l, src2_l, dst_l, nquads);
                        cxor43_3_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor43_4:
                        cxor43_4_block(src1_l, src2_l, dst_l, nquads);
                        cxor43_4_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor93_0:
                        cxor93_0_block(src1_l, src2_l, dst_l, nquads);
                        cxor93_0_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor93_1:
                        cxor93_1_block(src1_l, src2_l, dst_l, nquads);
                        cxor93_1_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor93_2:
                        cxor93_2_block(src1_l, src2_l, dst_l, nquads);
                        cxor93_2_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor93_3:
                        cxor93_3_block(src1_l, src2_l, dst_l, nquads);
                        cxor93_3_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    case CombinatorMode::cxm_cxor93_4:
                        cxor93_4_block(src1_l, src2_l, dst_l, nquads);
                        cxor93_4_block(src1_r, src2_r, dst_r, nquads);
                        break;
                    default:
                        mech::mul_block<BLOCK_SIZE_OS>(src1_l, src2_l, dst_l);
                        mech::mul_block<BLOCK_SIZE_OS>(src1_r, src2_r, dst_r);
                        break;
                    }

                    halfbandOut[inst][p]->process_block_D2(outOS[0], outOS[1], blockSizeOS);
                    mech::copy_from_to<blockSize>(outOS[0], output[inst][p][0]);
                    mech::copy_from_to<blockSize>(outOS[1], output[inst][p][1]);
                }
            }

            blockPos = 0;
            memset(inputA, 0, sizeof(inputA));
            memset(inputB, 0, sizeof(inputB));
        }
        for (int inst = 0; inst < 2; ++inst)
        {
            for (int p = 0; p < poly[inst]; ++p)
            {
                inputA[inst][p][0][blockPos] = inputs[INPUT_0_A_L + inst * 4].getVoltage(p);
                inputA[inst][p][1][blockPos] = aMono[inst]
                                                   ? inputs[INPUT_0_A_L + inst * 4].getVoltage(p)
                                                   : inputs[INPUT_0_A_R + inst * 4].getVoltage(p);
                inputB[inst][p][0][blockPos] = inputs[INPUT_0_B_L + inst * 4].getVoltage(p);
                inputB[inst][p][1][blockPos] = bMono[inst]
                                                   ? inputs[INPUT_0_B_L + inst * 4].getVoltage(p)
                                                   : inputs[INPUT_0_B_R + inst * 4].getVoltage(p);

                outputs[OUTPUT_0_L + inst * 2].setVoltage(output[inst][p][0][blockPos], p);
                outputs[OUTPUT_0_R + inst * 2].setVoltage(output[inst][p][1][blockPos], p);
            }
        }
        blockPos++;
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        return {{std::make_pair("CXOR 1 Port A", std::make_pair(INPUT_0_A_L, INPUT_0_A_R)),
                 std::make_pair("CXOR 1 Port B", std::make_pair(INPUT_0_B_L, INPUT_0_B_R)),
                 std::make_pair("CXOR 2 Port A", std::make_pair(INPUT_1_B_L, INPUT_1_B_R)),
                 std::make_pair("CXOR 2 Port A", std::make_pair(INPUT_1_B_L, INPUT_1_B_R))}};
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{
            std::make_pair("CXOR 1", std::make_pair(OUTPUT_0_L, OUTPUT_0_R)),
            std::make_pair("CXOR 2", std::make_pair(OUTPUT_1_L, OUTPUT_1_R)),
        }};
    }

    std::array<std::array<std::unique_ptr<sst::filters::HalfRate::HalfRateFilter>, MAX_POLY>, 2>
        halfbandInA, halfbandInB, halfbandOut;
    static constexpr int blockSize{8}, blockSizeOS{blockSize << 1};
    float inputA alignas(16)[2][MAX_POLY][2][blockSize]; // 0,1; poly; L/R
    bool aMono[2]{false, false};
    float inputB alignas(16)[2][MAX_POLY][2][blockSize]; // 0,1; poly; L/R
    bool bMono[2]{false, false};
    float output alignas(16)[2][MAX_POLY][2][blockSize]; // 0,1; poly; L/R
    int blockPos{0};
    int poly[2]{1, 1};
};
} // namespace sst::surgext_rack::digitalrm
#endif
