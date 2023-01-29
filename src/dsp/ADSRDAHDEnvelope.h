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

#ifndef SXT_RACK_ADSDDAHDENVELOPE_H
#define SXT_RACK_ADSDDAHDENVELOPE_H

#include "SurgeStorage.h"

namespace sst::surgext_rack::dsp::envelopes
{
struct ADSRDAHDEnvelope
{
    SurgeStorage *storage;
    ADSRDAHDEnvelope(SurgeStorage *s) : storage(s)
    {
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            outputCache[i] = 0;
            outputCacheCubed[0] = 0;
        }
        onSampleRateChanged();
    }

    enum Mode
    {
        ADSR_MODE,
        DAHD_MODE
    } mode{ADSR_MODE};

    enum Stage
    {
        s_delay, // skipped in ADSR
        s_attack,
        s_decay,   // Skipped in DAHD
        s_sustain, // Ignored in DAHD
        s_release,
        s_analog_residual_release,
        s_eoc,
        s_complete
    } stage{s_complete};

    bool isDigital{true};

    float phase{0}, start{0};

    float output{0}, outputCubed{0}, eoc_output{0};
    float outputCache alignas(16)[BLOCK_SIZE], outBlock0{0.f};
    float outputCacheCubed alignas(16)[BLOCK_SIZE];
    int current{BLOCK_SIZE};
    int eoc_countdown{0};

    // Analog Mode
    float v_c1{0}, v_c1_delayed{0.f};
    bool discharge{false};

    float coeff_offset{0};
    void onSampleRateChanged()
    {
        coeff_offset = 2.f - std::log2(storage->samplerate * BLOCK_SIZE_INV);
    }

    void attackFrom(Mode m, float fv, float attack, int ashp, bool isdig)
    {
        mode = m;
        float f = fv;

        if (isdig)
        {
            switch (ashp)
            {
            case 0:
                // target = sqrt(target);
                f = f * f;
                break;
            case 2:
                // target = target * target * target;
                f = pow(f, 1.0 / 3.0);
                break;
            }
        }
        phase = f;
        if (m == DAHD_MODE)
        {
            if (attack > 0.0001)
            {
                stage = s_delay;
            }
            else
            {
                stage = s_attack;
            }
        }
        else
            stage = s_attack;
        current = BLOCK_SIZE;
        isDigital = isdig;
        eoc_output = 0;
        eoc_countdown = 0;

        v_c1 = f;
        v_c1_delayed = f;
        discharge = false;
    }

    // that's log2(10) there
    static constexpr float etMin{-8}, etMax{3.32192809489}, etScale{etMax - etMin};

    float rFrom{0};
    inline float targetDigitalADSR(const float a, const float d, const float s, const float r,
                                   const int ashape, const int dshape, const int rshape,
                                   const bool gateActive)
    {
        if (!gateActive && stage < s_release)
        {
            rFrom = output;

            switch (rshape)
            {
            case 0:
                rFrom = rFrom * rFrom;
                break;
            case 2:
                rFrom = pow(rFrom, 1.0 / 3.0);
                break;
            }

            stage = s_release;
            phase = 0;
        }
        switch (stage)
        {
        case s_attack:
        {
            phase += storage->envelope_rate_linear_nowrap(a * etScale + etMin);
            if (phase > 1)
            {
                phase = 0;
                stage = s_decay;
                return 1;
            }
            return phase;
        }
        break;

        case s_decay:
        {
            phase += storage->envelope_rate_linear_nowrap(d * etScale + etMin);
            if (phase > 1)
            {
                phase = 0;
                stage = s_sustain;
                return s;
            }
            auto S = s;
            switch (dshape)
            {
            case 0:
                S = S * S;
                break;
            case 2:
                S = pow(S, 1.0 / 3.0);
                break;
            }

            auto dNorm = 1 - phase;
            dNorm = (dNorm) * (1.0 - S) + S;
            // FIXME - deal with shapes
            return dNorm;
        }
        break;

        case s_sustain:
        {
            return s;
        }
        break;
        case s_release:
        {
            phase += storage->envelope_rate_linear_nowrap(r * etScale + etMin);
            if (phase > 1)
            {
                phase = 0;
                stage = s_eoc;
                eoc_countdown = (int)std::round(storage->samplerate * 0.01);
                return 0;
            }
            return rFrom * (1 - phase);
        }
        break;
        default:
            assert(false);
            return 0;
        }
        return 0;
    }

    inline float targetAnalogADSR(const float a, const float d, const float s, const float r,
                                  const int ashape, const int dshape, const int rshape,
                                  const bool gateActive)
    {
        float coef_A = powf(2.f, std::min(0.f, coeff_offset - (a * etScale + etMin)));
        float coef_D = powf(2.f, std::min(0.f, coeff_offset - (d * etScale + etMin)));
        float coef_R =
            (stage >= s_eoc) ? 6.f : pow(2.f, std::min(0.f, coeff_offset - (r * etScale + etMin)));

        const float v_cc = 1.01f;
        float v_gate = gateActive ? v_cc : 0.f;

        // discharge = _mm_and_ps(_mm_or_ps(_mm_cmpgt_ss(v_c1_delayed, one), discharge),
        // v_gate);
        discharge = ((v_c1_delayed >= 1) || discharge) && gateActive;
        v_c1_delayed = v_c1;

        if (stage == s_attack)
        {
            phase += storage->envelope_rate_linear_nowrap(a * etScale + etMin);
            if (phase > 1)
            {
                stage = s_decay;
                phase = 0;
                discharge = true;
            }
        }

        float sparm = std::clamp(s, 0.f, 1.f);
        float S = sparm; // * sparm;
        switch (dshape)
        {
        case 0:
            S = S * S;
            break;
        case 2:
            S = pow(S, 1.0 / 3.0);
            break;
        }

        float v_attack = discharge ? 0 : v_gate;
        float v_decay = discharge ? S : v_cc;
        float v_release = v_gate;

        float diff_v_a = std::max(0.f, v_attack - v_c1);
        float diff_v_d = (discharge && gateActive) ? v_decay - v_c1 : std::min(0.f, v_decay - v_c1);
        float diff_v_r = std::min(0.f, v_release - v_c1);

        v_c1 = v_c1 + diff_v_a * coef_A;
        v_c1 = v_c1 + diff_v_d * coef_D;
        v_c1 = v_c1 + diff_v_r * coef_R;

        if (stage <= s_decay && !gateActive)
        {
            auto backoutShape = (stage == s_decay ? dshape : ashape);
            // Back out the D scaling
            switch (backoutShape)
            {
            case 0:
                v_c1 = sqrt(v_c1);
                break;
            case 2:
                v_c1 = v_c1 * v_c1 * v_c1;
                break;
            }

            stage = s_release;

            // put in the R scaling
            switch (rshape)
            {
            case 0:
                v_c1 = v_c1 * v_c1;
                break;
            case 2:
                v_c1 = pow(v_c1, 1.0 / 3.0); // fixme faster glitch
                break;
            }
            phase = 0;
        }

        if (stage == s_release)
        {
            phase += storage->envelope_rate_linear_nowrap(r * etScale + etMin);
            if (phase > 1)
            {
                stage = s_analog_residual_release;
                eoc_countdown = (int)std::round(storage->samplerate * 0.01);
            }
        }

        auto res = v_c1;
        if (!gateActive && !discharge && v_c1 < 1e-6)
        {
            if (stage != s_analog_residual_release)
            {
                res = 0;
                eoc_countdown = (int)std::round(storage->samplerate * 0.01);
                stage = s_eoc;
            }
            else
            {
                stage = s_complete;
                eoc_countdown = 0;
            }
        }

        return res;
    }

    inline float targetDigitalDAHD(const float dly, const float a, const float h, const float d,
                                   const int ashape, const int dshape, const int rshape)
    {
        switch (stage)
        {
        case s_delay:
        {
            phase += storage->envelope_rate_linear_nowrap(dly * etScale + etMin);
            if (phase > 1)
            {
                stage = s_attack;
                phase -= 1;
                return phase;
            }
            return 0;
        }
        break;
        case s_attack:
        {
            phase += storage->envelope_rate_linear_nowrap(a * etScale + etMin);
            if (phase > 1)
            {
                phase = 0;
                stage = s_sustain;
                return 1;
            }
            return phase;
        }
        break;
        case s_sustain:
        {
            phase += storage->envelope_rate_linear_nowrap(h * etScale + etMin);
            if (phase > 1)
            {
                phase = 0;
                stage = s_release;
                return 1 - phase;
            }
            return 1;
        }
        break;
        case s_release:
        {
            phase += storage->envelope_rate_linear_nowrap(d * etScale + etMin);
            if (phase > 1)
            {
                phase = 0;
                stage = s_eoc;
                eoc_countdown = (int)std::round(storage->samplerate * 0.01);
                return 0;
            }
            return (1 - phase);
        }
        break;
        default:
            assert(false);
            return 0;
        }
        return 0;
    }

    inline float targetAnalogDAHD(const float dly, const float a, const float h, const float d,
                                  const int ashape, const int dshape, const int rshape)
    {
        if (stage == s_delay)
        {
            phase += storage->envelope_rate_linear_nowrap(dly * etScale + etMin);
            if (phase > 1)
            {
                stage = s_attack;
                phase -= 1;
            }
            return 0;
        }

        if (stage == s_sustain)
        {
            phase += storage->envelope_rate_linear_nowrap(h * etScale + etMin);
            if (phase > 1)
            {
                stage = s_release;
                phase = 1;
            }
            return 1;
        }

        auto ndc = (v_c1_delayed >= 0.99999f);
        if (ndc && !discharge)
        {
            phase = 0;
            stage = s_sustain;
        }

        discharge = ndc || discharge;
        v_c1_delayed = v_c1;

        static constexpr float v_gate = 1.02f;
        auto v_attack = (!discharge) * v_gate;
        auto v_decay = (!discharge) * v_gate;

        // In this case we only need the coefs in their stage
        float coef_A =
            !discharge ? powf(2.f, std::min(0.f, coeff_offset - (a * etScale + etMin))) : 0;
        float coef_D =
            discharge ? powf(2.f, std::min(0.f, coeff_offset - (d * etScale + etMin))) : 0;

        auto diff_v_a = std::max(0.f, v_attack - v_c1);
        auto diff_v_d = std::min(0.f, v_decay - v_c1);

        v_c1 = v_c1 + diff_v_a * coef_A + diff_v_d * coef_D;
        auto res = v_c1;

        if (stage == s_release)
        {
            phase -= storage->envelope_rate_linear(d * etScale + etMin);
            if (phase <= 0)
            {
                eoc_countdown = (int)std::round(storage->samplerate * 0.01);
                stage = s_analog_residual_release;
            }
        }
        if (v_c1 < 1e-6 && discharge)
        {
            v_c1 = 0;
            v_c1_delayed = 0;
            discharge = false;
            res = 0;
            if (stage != s_analog_residual_release)
            {
                eoc_countdown = (int)std::round(storage->samplerate * 0.01);
                stage = s_eoc;
            }
            else
            {
                stage = s_complete;
                eoc_countdown = 0;
            }
        }
        return (stage == s_sustain ? 1 : res);
    }

    inline void process(const float a, const float d, const float s, const float r,
                        const int ashape, const int dshape, const int rshape, const bool gateActive)
    {
        if (stage == s_complete)
        {
            output = 0;
            return;
        }

        if (stage == s_eoc)
        {
            output = 0;
            eoc_output = 1;

            eoc_countdown--;
            if (eoc_countdown == 0)
            {
                eoc_output = 0;
                stage = s_complete;
            }
            return;
        }
        eoc_output = 0;

        if (stage == s_analog_residual_release && eoc_countdown)
        {
            eoc_output = 1;
            eoc_countdown--;
        }
        if (current == BLOCK_SIZE)
        {
            float target = 0;
            switch (mode)
            {
            case ADSR_MODE:
                if (isDigital)
                    target = targetDigitalADSR(a, d, s, r, ashape, dshape, rshape, gateActive);
                else
                    target = targetAnalogADSR(a, d, s, r, 1, 1, 1, gateActive);
                break;
            case DAHD_MODE:
                if (isDigital)
                    target = targetDigitalDAHD(a, d, s, r, ashape, dshape, rshape);
                else
                    target = targetAnalogDAHD(a, d, s, r, 1, 1, 1);
                break;
            }

            if (isDigital)
            {
                if (stage == s_attack)
                {
                    switch (ashape)
                    {
                    case 0:
                        target = sqrt(target);
                        break;
                    case 2:
                        target = target * target * target;
                        break;
                    }
                }
                else if (stage == s_decay)
                {
                    switch (dshape)
                    {
                    case 0:
                        target = sqrt(target);
                        break;
                    case 2:
                        target = target * target * target;
                        break;
                    }
                }
                else if (stage == s_release || stage == s_analog_residual_release)
                {
                    switch (rshape)
                    {
                    case 0:
                        target = sqrt(target);
                        break;
                    case 2:
                        target = target * target * target;
                        break;
                    }
                }
            }

            float dO = (target - outBlock0) * BLOCK_SIZE_INV;
            for (int i = 0; i < BLOCK_SIZE; ++i)
            {
                outputCache[i] = outBlock0 + dO * i;
                outputCacheCubed[i] = outputCache[i] * outputCache[i] * outputCache[i];
            }
            outBlock0 = target;
            current = 0;
        }

        output = outputCache[current];
        outputCubed = outputCacheCubed[current];
        current++;
    }
};
} // namespace sst::surgext_rack::dsp::envelopes
#endif // RACK_HACK_ADARENVELOPE_H
