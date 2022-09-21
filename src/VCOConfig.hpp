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
#include "dsp/oscillators/TwistOscillator.h"

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
template <>
std::function<std::string(const std::string &)> VCOConfig<ot_string>::rightMenuTransformFunction()
{
    auto res = [](const std::string &s) {
        auto rv = s;
        auto pn = rv.find("PINK NOISE");
        if (pn != std::string::npos)
        {
            rv = rv.substr(0, pn) + "PINK NS";
        }
        pn = rv.find("CONSTANT ");
        if (pn != std::string::npos)
        {
            rv = std::string("CNS ") + rv.substr(pn + strlen("CONSTANT "));
        }
        return rv;
    };
    return res;
}
template <> constexpr bool VCOConfig<ot_string>::supportsAudioIn() { return true; }
template <> std::string VCOConfig<ot_string>::retriggerLabel() { return "TRIG"; }
template <> int VCOConfig<ot_string>::getMenuLightID() { return 0; }
template <> std::string VCOConfig<ot_string>::getMenuLightString() { return "2X"; }
template <> void VCOConfig<ot_string>::processLightParameters(VCO<ot_string> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_string>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        auto p = &(s->p[StringOscillator::str_exciter_level]);
        auto dt = p->deform_type & StringOscillator::os_all;
        if (l0 && dt != StringOscillator::os_twox)
        {
            p->deform_type =
                (p->deform_type & ~StringOscillator::os_all) | StringOscillator::os_twox;
            m->forceRespawnDueToExternality = true;
        }
        else if (!l0 && dt != StringOscillator::os_onex)
        {
            p->deform_type =
                (p->deform_type & ~StringOscillator::os_all) | StringOscillator::os_onex;
            m->forceRespawnDueToExternality = true;
        }
    }
}

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
template <> VCOConfig<ot_FM3>::lightOnTo_t VCOConfig<ot_FM3>::getLightsOnKnobsTo()
{
    return {{3, 0}, {5, 1}};
}

template <> VCOConfig<ot_FM3>::LightType VCOConfig<ot_FM3>::getLightTypeFor(int idx)
{
    return ABSOLUTE;
}

template <> void VCOConfig<ot_FM3>::processLightParameters(VCO<ot_FM3> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);
    auto l1 = (bool)(m->params[VCO<ot_FM3>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        if (l0 != s->p[FM3Oscillator::fm3_m1ratio].absolute)
            s->p[FM3Oscillator::fm3_m1ratio].absolute = l0;
        if (l1 != s->p[FM3Oscillator::fm3_m2ratio].absolute)
            s->p[FM3Oscillator::fm3_m2ratio].absolute = l1;
    }
}

template <> constexpr bool VCOConfig<ot_alias>::supportsUnison() { return true; }
template <> VCOConfig<ot_alias>::knobs_t VCOConfig<ot_alias>::getKnobs()
{
    typedef VCO<ot_alias> M;

    return {{M::PITCH_0, "PITCH"},
            {M::OSC_CTRL_PARAM_0 + 1, "WARP"},
            {M::OSC_CTRL_PARAM_0 + 2, "MASK"},
            {KnobDef::BLANK},
            {KnobDef::INPUT, M::AUDIO_INPUT, "AUDIO"},
            {M::OSC_CTRL_PARAM_0 + 3, "THRESH"},
            {M::OSC_CTRL_PARAM_0 + 4, "BITCRSH"},
            {M::OSC_CTRL_PARAM_0 + 5, "DETUNE"}};
}
template <> int VCOConfig<ot_alias>::rightMenuParamId() { return 0; }
// template <> constexpr bool VCOConfig<ot_alias>::supportsAudioIn() { return true; }

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

static std::string twistFirstParam(VCO<ot_twist> *m)
{
    auto model = m->oscstorage_display->p[0].val.i;
    switch (model)
    {
    case 0: // waveforms
        return "DETUNE";
    case 1: // waveshaper
        return "SHAPER";
    case 2: // 2op
    case 3: // formant
        return "RATIO";
    case 4: // harmonic
        return "BUMP";
    case 5: // wt
        return "BANK";
    case 6: // chords
        return "TYPE";
    case 7: // vowel
        return "SPEAK";
    case 8: // gran cloud
        return "RANDOM";
    case 9: // filtered noise
        return "TYPE";
    case 10: // part noise
        return "FREQ";
    case 11: // inharm string
        return "INHARM";
    case 12: // modal resonator
        return "MATERIAL";
    case 13: // analog kick
        return "SHARP";
    case 14: // analog snare
    case 15: // analog hat
        return "TONE/NS";
    }
    return std::to_string(model);
}

static std::string twistSecondParam(VCO<ot_twist> *m)
{
    auto model = m->oscstorage_display->p[0].val.i;
    switch (model)
    {
    case 0: // waveforms
        return "SQUARE";
    case 1: // waveshaper
        return "FOLD";
    case 2: // 2op
        return "AMOUNT";
    case 3: // formant
        return "FORMANT";
    case 4: // harmonic
        return "PEAK";
    case 5: // wt
        return "X MORPH";
    case 6: // chords
        return "INVER";
    case 7: // vowel
        return "SPECIES";
    case 8: // gran cloud
        return "DENISTY";
    case 9: // filtered noise
        return "FREQ";
    case 10: // part noise
        return "DENSITY";
    case 11: // inharm string
        return "BRIGHT";
    case 12: // modal resonator
        return "BRIGHT";
    case 13: // analog kick
        return "BRIGHT";
    case 14: // analog snare
        return "MODEL";
    case 15: // analog hat
        return "LOCUT";
    }
    return std::to_string(model);
}

static std::string twistThirdParam(VCO<ot_twist> *m)
{
    auto model = m->oscstorage_display->p[0].val.i;
    switch (model)
    {
    case 0: // waveforms
        return "SAW";
    case 1: // waveshaper
        return "ASYM";
    case 2: // 2op
        return "F/BACK";
    case 3: // formant
        return "SHAPE";
    case 4: // harmonic
        return "SHAPE";
    case 5: // wt
        return "Y MORPH";
    case 6: // chords
        return "SHAPE";
    case 7: // vowel
        return "SEGMENT";
    case 8: // gran cloud
        return "DURATION";
    case 9: // filtered noise
        return "RES";
    case 10: // part noise
        return "TYPE";
    case 11: // inharm string
        return "DECAY";
    case 12: // modal resonator
        return "DECAY";
    case 13: // analog kick
        return "DECAY";
    case 14: // analog snare
        return "DECAY";
    case 15: // analog hat
        return "DECAY";
    }
    return std::to_string(model);
}

static std::string twistFourthParam(VCO<ot_twist> *m)
{
    auto model = m->oscstorage_display->p[0].val.i;
    switch (model)
    {
    case 0: // waveforms
        return "SYNC";
    case 1: // waveshaper
        return "VAR";
    case 2: // 2op
        return "SUB";
    case 3: // formant
        return "PD";
    case 4: // harmonic
        return "ORGAN";
    case 5: // wt
        return "LOFI";
    case 6: // chords
        return "ROOT";
    case 7: // vowel
        return "RAW";
    case 8: // gran cloud
        return "SINE";
    case 9: // filtered noise
        return "DUAL";
    case 10: // part noise
        return "RAW";
    case 11: // inharm string
        return "EXCITER";
    case 12: // modal resonator
        return "EXCITER";
    case 13: // analog kick
        return "VAR";
    case 14: // analog snare
        return "VAR";
    case 15: // analog hat
        return "VAR";
    }
    return std::to_string(model);
}

template <> VCOConfig<ot_twist>::knobs_t VCOConfig<ot_twist>::getKnobs()
{
    typedef VCO<ot_twist> M;

    return {
        {M::PITCH_0, "PITCH"},
        {KnobDef::BLANK},
        KnobDef::withDynamicLabel(M::OSC_CTRL_PARAM_0 + 1,
                                  [](VCO<ot_twist> *m) { return twistFirstParam(m); }),
        KnobDef::withDynamicLabel(M::OSC_CTRL_PARAM_0 + 2,
                                  [](VCO<ot_twist> *m) { return twistSecondParam(m); }),
        {M::OSC_CTRL_PARAM_0 + 5, "RESP"},
        {M::OSC_CTRL_PARAM_0 + 6, "DECAY"},
        KnobDef::withDynamicLabel(M::OSC_CTRL_PARAM_0 + 3,
                                  [](VCO<ot_twist> *m) { return twistThirdParam(m); }),
        KnobDef::withDynamicLabel(M::OSC_CTRL_PARAM_0 + 4,
                                  [](VCO<ot_twist> *m) { return twistFourthParam(m); }),
    };
}
template <> int VCOConfig<ot_twist>::rightMenuParamId() { return 0; }
template <> std::string VCOConfig<ot_twist>::retriggerLabel() { return "TRIG"; }

template <> VCOConfig<ot_twist>::lightOnTo_t VCOConfig<ot_twist>::getLightsOnKnobsTo()
{
    return {{4, 0}};
}

template <> void VCOConfig<ot_twist>::processLightParameters(VCO<ot_twist> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[TwistOscillator::twist_lpg_response].deactivated = !l0;
    }
}

} // namespace sst::surgext_rack::vco

#endif // SURGEXT_RACK_VCOCONFIG_HPP
