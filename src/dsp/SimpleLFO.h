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

namespace sst::surgext_rack::dsp::modulators
{
struct SimpleLFO
{
    SurgeStorage *storage;
    SimpleLFO(SurgeStorage *s) : storage(s)
    {
        for (int i = 0; i < BLOCK_SIZE; ++i)
            outputCache[i] = 0;
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

    inline void process(const float r, const float d, const int lshape)
    {

        if (current == BLOCK_SIZE)
        {
            float target{0.f};

            auto frate = storage->envelope_rate_linear_nowrap(-r);
            phase += frate;

            if (phase > 1)
                phase -= 1;

            target = bend1(std::sin(2.0 * M_PI * phase), d);

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
