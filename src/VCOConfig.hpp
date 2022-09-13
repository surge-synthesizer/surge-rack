//
// Created by Paul Walker on 9/3/22.
//

#ifndef SURGEXT_RACK_VCOCONFIG_HPP
#define SURGEXT_RACK_VCOCONFIG_HPP

#include "VCO.hpp"

#include "dsp/oscillators/FM2Oscillator.h"
#include "dsp/oscillators/FM3Oscillator.h"
#include "dsp/oscillators/SineOscillator.h"
#include "dsp/oscillators/StringOscillator.h"
#include "dsp/oscillators/SampleAndHoldOscillator.h"
#include "dsp/oscillators/WindowOscillator.h"

namespace sst::surgext_rack::vco
{

template <> constexpr bool VCOConfig<ot_classic>::supportsUnison() { return true; }
template <> VCOConfig<ot_classic>::knobs_t VCOConfig<ot_classic>::getKnobs()
{
    typedef VCO<ot_classic> M;
    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0, "SHAPE"},
            {M::OSC_CTRL_PARAM_0 + 1, "WIDTH1"},
            {M::OSC_CTRL_PARAM_0 + 2, "WIDTH2"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "SUBMIX"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> VCOConfig<ot_string>::knobs_t VCOConfig<ot_string>::getKnobs()
{
    typedef VCO<ot_string> M;

    return {
        {M::PITCH_0, "PITCH"},
        {M::OSC_CTRL_PARAM_0 + 1, "EXCITER"},
        {KnobDef::PARAM, M::OSC_CTRL_PARAM_0 + 2, "S1 - DECAY - S2", 2},
        {M::OSC_CTRL_PARAM_0 + 3, ""},
        {KnobDef::INPUT, M::AUDIO_INPUT, "AUDIO"},
        {M::OSC_CTRL_PARAM_0 + 6, "STIFF"},
        {M::OSC_CTRL_PARAM_0 + 5, "BALANCE"},
        {M::OSC_CTRL_PARAM_0 + 4, "DETUNE"},
    };
}
template <> int VCOConfig<ot_string>::rightMenuParamId() { return 0; }

template <> void VCOConfig<ot_string>::oscillatorSpecificSetup(VCO<ot_string> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[StringOscillator::str_stiffness].deform_type = StringOscillator::filter_compensate;
    }
}

template <> constexpr bool VCOConfig<ot_modern>::supportsUnison() { return true; }
template <> VCOConfig<ot_modern>::knobs_t VCOConfig<ot_modern>::getKnobs()
{
    typedef VCO<ot_modern> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 0, "SAW"},
            {M::OSC_CTRL_PARAM_0 + 1, "PULSE"},
            {M::OSC_CTRL_PARAM_0 + 2, "MULTI"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "WIDTH"},
            {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> constexpr bool VCOConfig<ot_wavetable>::supportsUnison() { return true; }
template <> constexpr bool VCOConfig<ot_wavetable>::requiresWavetables() { return true; }
template <> VCOConfig<ot_wavetable>::knobs_t VCOConfig<ot_wavetable>::getKnobs()
{
    typedef VCO<ot_wavetable> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 0, "MORPH"},
            {M::OSC_CTRL_PARAM_0 + 2, "SATUR"},
            {M::OSC_CTRL_PARAM_0 + 3, "FORMANT"},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 1, "V SKEW"},
            {M::OSC_CTRL_PARAM_0 + 4, "H SKEW"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> constexpr bool VCOConfig<ot_window>::supportsUnison() { return true; }
template <> constexpr bool VCOConfig<ot_window>::requiresWavetables() { return true; }
template <> VCOConfig<ot_window>::knobs_t VCOConfig<ot_window>::getKnobs()
{
    typedef VCO<ot_window> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 0, "MORPH"},
            {M::OSC_CTRL_PARAM_0 + 1, "FORMANT"},
            {KnobDef::BLANK},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "LOCUT"},
            {M::OSC_CTRL_PARAM_0 + 4, "HICUT"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> VCOConfig<ot_window>::lightOnTo_t VCOConfig<ot_window>::getLightsOnKnobsTo()
{
    return {{5, 0}, {6, 1}};
}

template <> int VCOConfig<ot_window>::rightMenuParamId() { return 2; }

template <> void VCOConfig<ot_window>::oscillatorSpecificSetup(VCO<ot_window> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[WindowOscillator::win_morph].set_extend_range(true);
    }
}

template <> void VCOConfig<ot_window>::processLightParameters(VCO<ot_window> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_window>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_window>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != !s->p[WindowOscillator::win_lowcut].deactivated)
            s->p[WindowOscillator::win_lowcut].deactivated = !l0;
        if (l1 != !s->p[WindowOscillator::win_highcut].deactivated)
            s->p[WindowOscillator::win_highcut].deactivated = !l1;
    }
}

template <> constexpr bool VCOConfig<ot_sine>::supportsUnison() { return true; }
template <> VCOConfig<ot_sine>::knobs_t VCOConfig<ot_sine>::getKnobs()
{
    typedef VCO<ot_sine> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 1, "FEEDBACK"},
            {KnobDef::BLANK},
            {KnobDef::BLANK},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "LOCUT"},
            {M::OSC_CTRL_PARAM_0 + 4, "HICUT"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}

template <> VCOConfig<ot_sine>::lightOnTo_t VCOConfig<ot_sine>::getLightsOnKnobsTo()
{
    return {{5, 0}, {6, 1}};
}

template <> int VCOConfig<ot_sine>::rightMenuParamId() { return 0; }

template <> void VCOConfig<ot_sine>::oscillatorSpecificSetup(VCO<ot_sine> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[SineOscillator::sine_feedback].set_extend_range(true);
    }
}

template <> void VCOConfig<ot_sine>::processLightParameters(VCO<ot_sine> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_sine>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_sine>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != !s->p[SineOscillator::sine_lowcut].deactivated)
            s->p[SineOscillator::sine_lowcut].deactivated = !l0;
        if (l1 != !s->p[SineOscillator::sine_highcut].deactivated)
            s->p[SineOscillator::sine_highcut].deactivated = !l1;
    }
}

template <> VCOConfig<ot_FM2>::knobs_t VCOConfig<ot_FM2>::getKnobs()
{
    typedef VCO<ot_FM2> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 6, "FBACK"},
            {M::OSC_CTRL_PARAM_0 + 0, "M1 AMT"},
            {M::OSC_CTRL_PARAM_0 + 1, "M1 RATIO"},
            {M::OSC_CTRL_PARAM_0 + 4, "OFFSET"},
            {M::OSC_CTRL_PARAM_0 + 5, "PHASE"},
            {M::OSC_CTRL_PARAM_0 + 2, "M2 AMT"},
            {M::OSC_CTRL_PARAM_0 + 3, "M2 RATIO"}};
}

template <> VCOConfig<ot_FM3>::knobs_t VCOConfig<ot_FM3>::getKnobs()
{
    typedef VCO<ot_FM3> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 6, "FBACK"},
            {M::OSC_CTRL_PARAM_0 + 0, "M1 AMT"},
            {M::OSC_CTRL_PARAM_0 + 1, "M1 RATIO"},
            {M::OSC_CTRL_PARAM_0 + 2, "M2 AMT"},
            {M::OSC_CTRL_PARAM_0 + 3, "M2 RATIO"},
            {M::OSC_CTRL_PARAM_0 + 4, "M3 AMT"},
            {M::OSC_CTRL_PARAM_0 + 5, "M3 FREQ"}};
}

template <> constexpr bool VCOConfig<ot_alias>::supportsUnison() { return true; }
template <> VCOConfig<ot_alias>::knobs_t VCOConfig<ot_alias>::getKnobs()
{
    typedef VCO<ot_alias> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 1, "WARP"},
            {M::OSC_CTRL_PARAM_0 + 2, "MASK"},
            {KnobDef::BLANK},
            {KnobDef::BLANK},
            {M::OSC_CTRL_PARAM_0 + 3, "THRESH"},
            {M::OSC_CTRL_PARAM_0 + 4, "BITCRSH"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}
template <> int VCOConfig<ot_alias>::rightMenuParamId() { return 0; }

template <> constexpr bool VCOConfig<ot_shnoise>::supportsUnison() { return true; }
template <> VCOConfig<ot_shnoise>::knobs_t VCOConfig<ot_shnoise>::getKnobs()
{
    typedef VCO<ot_shnoise> M;

    return {
        {M::PITCH_0, "PITCH"},
        {M::OSC_CTRL_PARAM_0 + 0, "CORR"},
        {M::OSC_CTRL_PARAM_0 + 1, "WIDTH"},
        {M::OSC_CTRL_PARAM_0 + 4, "SYNC"},
        {KnobDef::BLANK},
        {M::OSC_CTRL_PARAM_0 + 2, "LOCUT"},
        {M::OSC_CTRL_PARAM_0 + 3, "HICUT"},
        {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"},
    };
}

template <> VCOConfig<ot_shnoise>::lightOnTo_t VCOConfig<ot_shnoise>::getLightsOnKnobsTo()
{
    return {{5, 0}, {6, 1}};
}

template <> void VCOConfig<ot_shnoise>::processLightParameters(VCO<ot_shnoise> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_shnoise>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != !s->p[SampleAndHoldOscillator::shn_lowcut].deactivated)
            s->p[SampleAndHoldOscillator::shn_lowcut].deactivated = !l0;
        if (l1 != !s->p[SampleAndHoldOscillator::shn_highcut].deactivated)
            s->p[SampleAndHoldOscillator::shn_highcut].deactivated = !l1;
    }
}

} // namespace sst::surgext_rack::vco

#endif // SURGEXT_RACK_VCOCONFIG_HPP
