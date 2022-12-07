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

#ifndef SURGERACK_SIMPLELFO_H
#define SURGERACK_SIMPLELFO_H

#include "SurgeStorage.h"
#include "DSPUtils.h"

namespace sst::surgext_rack::dsp::modulators
{
struct SimpleLFO
{
    SurgeStorage *storage;
    std::default_random_engine gen;
    std::uniform_real_distribution<float> distro;
    std::function<float()> urng;

    float rngState[2]{0, 0};
    float rngHistory[4]{0, 0, 0, 0};

    float rngCurrent{0};

    SimpleLFO(SurgeStorage *s) : storage(s)
    {
        gen = std::default_random_engine();
        gen.seed(storage->rand_u32());
        distro = std::uniform_real_distribution<float>(-1.f, 1.f);
        urng = [this]() -> float { return distro(gen); };

        for (int i = 0; i < BLOCK_SIZE; ++i)
            outputCache[i] = 0;

        rngState[0] = urng();
        rngState[1] = urng();
        for (int i = 0; i < 4; ++i)
        {
            rngCurrent = correlated_noise_o2mk2_suppliedrng(rngState[0], rngState[1], 0, urng);
            rngHistory[3 - i] = rngCurrent;
        }
    }

    enum Shape
    {
        SINE,
        RAMP,
        TRI,
        PULSE,
        SMOOTH_NOISE,
        SH_NOISE
    };

    float outputCache[BLOCK_SIZE], outBlock0{0.f};
    float output;
    int current{BLOCK_SIZE};
    float phase;

    inline float bend1(float x, float d)
    {
        auto a = 0.5 * std::clamp(d, -3.f, 3.f);
        x = x - a * x * x + a;
        x = x - a * x * x + a;
        return x;
    }

    inline void attackForDisplay(const int lshape)
    {
        attack(lshape);

        gen = std::default_random_engine();
        gen.seed(525600 + 8675309);
        distro = std::uniform_real_distribution<float>(-1.f, 1.f);
        urng = [this]() -> float { return distro(gen); };

        for (int i = 0; i < BLOCK_SIZE; ++i)
            outputCache[i] = 0;

        rngState[0] = urng();
        rngState[1] = urng();
        for (int i = 0; i < 4; ++i)
        {
            rngCurrent = correlated_noise_o2mk2_suppliedrng(rngState[0], rngState[1], 0, urng);
            rngHistory[3 - i] = rngCurrent;
        }
    }
    // FIXME - make this work for proper attacks
    inline void attack(const int lshape)
    {
        phase = 0;
        current = BLOCK_SIZE;
        for (int i = 0; i < BLOCK_SIZE; ++i)
            outputCache[i] = 0;
        output = 0;
        outBlock0 = 0;
    }

    // Really just used from the UI thread
    inline void processResettingBlock(const float f, const float d, const int lshape)
    {
        current = BLOCK_SIZE;
        process(f, d, lshape);
    }

    float lastDPhase{0};
    inline void applyPhaseOffset(float dPhase)
    {
        if (dPhase != lastDPhase)
        {
            phase += dPhase - lastDPhase;
            if (phase > 1)
                phase -= 1;
        }
        lastDPhase = dPhase;
    }

    inline void process(const float r, const float d, const int lshape)
    {
        if (current == BLOCK_SIZE)
        {
            float target{0.f};

            auto frate = storage->envelope_rate_linear_nowrap(-r);
            phase += frate;

            if (phase > 1)
            {
                if (lshape == SH_NOISE || lshape == SMOOTH_NOISE)
                {
                    // The deform can push correlated noise out of bounds
                    auto ud = d * 0.8;
                    rngCurrent =
                        correlated_noise_o2mk2_suppliedrng(rngState[0], rngState[1], ud, urng);

                    rngHistory[3] = rngHistory[2];
                    rngHistory[2] = rngHistory[1];
                    rngHistory[1] = rngHistory[0];

                    rngHistory[0] = rngCurrent;
                }
                phase -= 1;
            }
            auto shp = (Shape)(lshape);
            switch (shp)
            {
            case SINE:
                target = bend1(std::sin(2.0 * M_PI * phase), d);
                break;
            case RAMP:
                target = bend1(2 * phase - 1, d);
                break;
            case TRI:
            {
                auto tphase = (phase + 0.25);
                if (tphase > 1)
                    tphase -= 1;
                target = bend1(-1.f + 4.f * ((tphase > 0.5) ? (1 - tphase) : tphase), d);
                break;
            }
            case PULSE:
                target = (phase < (d + 1) * 0.5) ? 1 : -1;
                break;
            case SMOOTH_NOISE:
                target =
                    cubic_ipol(rngHistory[3], rngHistory[2], rngHistory[1], rngHistory[0], phase);
                break;
            case SH_NOISE:
                target = rngCurrent;
                break;
            default:
                target = 0.1;
                break;
            }
            float dO = (target - outBlock0) * BLOCK_SIZE_INV;
            for (int i = 0; i < BLOCK_SIZE; ++i)
            {
                outputCache[i] = outBlock0 + dO * i;
            }
            outBlock0 = target;
            current = 0;
        }
        output = outputCache[current];
        current++;
    }
};
} // namespace sst::surgext_rack::dsp::modulators
#endif // RACK_HACK_SIMPLELFO_H
