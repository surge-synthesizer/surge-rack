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

#ifndef SURGEXT_TWIST_CONFIG_H
#define SURGEXT_TWIST_CONFIG_H

#include "dsp/oscillators/TwistOscillator.h"
namespace sst::surgext_rack::vco
{

static std::string twistFirstParam(modules::XTModule *mo)
{
    auto m = static_cast<VCO<ot_twist> *>(mo);
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

static std::string twistSecondParam(modules::XTModule *mo)
{
    auto m = static_cast<VCO<ot_twist> *>(mo);
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

static std::string twistThirdParam(modules::XTModule *mo)
{
    auto m = static_cast<VCO<ot_twist> *>(mo);
    auto model = m->oscstorage_display->p[0].val.i;
    switch (model)
    {
    case 0: // waveforms
        return "SAW";
    case 1: // waveshaper
        return "ASYM";
    case 2: // 2op
        return "FEEDBACK";
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

static std::string twistFourthParam(modules::XTModule *mo)
{
    auto m = static_cast<VCO<ot_twist> *>(mo);
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

template <> VCOConfig<ot_twist>::layout_t VCOConfig<ot_twist>::getLayout()
{
    typedef VCO<ot_twist> M;
    int cp = M::OSC_CTRL_PARAM_0;
    return {
        // clang-format off
        LayoutItem::createVCOKnob(M::PITCH_0, "PITCH", 0, 0),
        LayoutItem::createVCOKnob(cp + 1, twistFirstParam, 0, 2),
        LayoutItem::createVCOKnob(cp + 2, twistSecondParam, 0, 3),

        LayoutItem::createVCOKnob(cp + 5, "RESP", 1, 0),
        LayoutItem::createVCOLight(LayoutItem::POWER_LIGHT, M::ARBITRARY_SWITCH_0, 1, 0),

        LayoutItem::createVCOKnob(cp + 6, "DECAY", 1, 1),
        LayoutItem::createVCOKnob(cp + 3, twistThirdParam, 1, 2),
        LayoutItem::createVCOKnob(cp + 4, twistFourthParam, 1, 3),
        // clang-format on
    };
}

template <> int VCOConfig<ot_twist>::rightMenuParamId() { return 0; }
template <> std::string VCOConfig<ot_twist>::retriggerLabel() { return "TRIG"; }

template <> void VCOConfig<ot_twist>::processVCOSpecificParameters(VCO<ot_twist> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        s->p[TwistOscillator::twist_lpg_response].deactivated = !l0;
    }
}
} // namespace sst::surgext_rack::vco

#endif
