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

#ifndef SURGE_XT_RACK_SRC_VCOCONFIG_TWIST_H
#define SURGE_XT_RACK_SRC_VCOCONFIG_TWIST_H

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
        return "DENSITY";
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

template <> constexpr bool VCOConfig<ot_twist>::recreateOnSampleRateChange() { return true; }
template <> int VCOConfig<ot_twist>::rightMenuParamId() { return 0; }
template <> std::string VCOConfig<ot_twist>::retriggerLabel() { return "TRIG"; }

template <> void VCOConfig<ot_twist>::oscillatorSpecificSetup(VCO<ot_twist> *m)
{
    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        // LPG is off but set up
        s->p[TwistOscillator::twist_lpg_response].set_value_f01(0.5);
        s->p[TwistOscillator::twist_lpg_decay].set_value_f01(0.5);
    }
}
template <> inline void VCOConfig<ot_twist>::configureVCOSpecificParameters(VCO<ot_twist> *m)
{
    m->configOnOff(VCO<ot_twist>::ARBITRARY_SWITCH_0 + 0, 0, "Enable LPG on Trigger");
    m->configOnOffNoRand(VCO<ot_twist>::ARBITRARY_SWITCH_0 + 1, 0, "Randomize Includes Engine");

    m->intStateForConfig[0] = m->inputs[VCO<ot_twist>::RETRIGGER].isConnected();
    m->intStateForConfig[1] = m->oscstorage->p[TwistOscillator::twist_lpg_response].deactivated;
    for (int i = 2; i < VCO<ot_twist>::n_arbitrary_switches; ++i)
    {
        m->configParam(VCO<ot_twist>::ARBITRARY_SWITCH_0 + i, 0, 1, 0, "Unused");
    }
}
template <> void VCOConfig<ot_twist>::processVCOSpecificParameters(VCO<ot_twist> *m)
{
    auto l0 = (bool)(m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 0].getValue() > 0.5);

    bool wasConnected = m->intStateForConfig[0];
    auto isCon = m->inputs[VCO<ot_twist>::RETRIGGER].isConnected();

    if (!l0 && !wasConnected && isCon)
    {
        l0 = true;
        m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 0].setValue(1);
    }
    else if (l0 && wasConnected && !isCon)
    {
        l0 = false;
        m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 0].setValue(0);
    }

    m->intStateForConfig[0] = isCon;

    for (auto s : {m->oscstorage, m->oscstorage_display})
    {
        auto deact = !l0;
        /*
         * Never paint the LPG on the display
         */
        if (s == m->oscstorage_display)
            deact = true;
        s->p[TwistOscillator::twist_lpg_response].deactivated = deact;
    }

    auto l1 = (bool)(m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 1].getValue() > 0.5);
    auto epq = m->paramQuantities[VCO<ot_twist>::OSC_CTRL_PARAM_0 + TwistOscillator::twist_engine];
    if (epq->randomizeEnabled != l1)
    {
        epq->randomizeEnabled = l1;
    }
}

template <> bool VCOConfig<ot_twist>::getVCOSpecificReInit(VCO<ot_twist> *m)
{
    auto deact = m->oscstorage->p[TwistOscillator::twist_lpg_response].deactivated;
    auto res = (deact != m->intStateForConfig[1]);
    m->intStateForConfig[1] = deact;
    return res;
}

template <> void VCOConfig<ot_twist>::postSpawnOscillatorChange(Oscillator *o)
{
    auto to = dynamic_cast<TwistOscillator *>(o);
    if (to)
    {
        to->useCorrectLPGBlockSize = true;
    }
}

template <>
void VCOConfig<ot_twist>::oscillatorReInit(VCO<ot_twist> *m, Oscillator *o, float pitch0)
{
    // Twist is special in that it doesn't phase lock and has a warmup
    // for voice on which for surge single voice means it makes sense
    // but here the re-init when triggered (which is the LPG gesture)
    // doesn't make sense. So just do nothing here if LPG is on and
    // trigger is connected
    auto deact = m->oscstorage->p[TwistOscillator::twist_lpg_response].deactivated;
    if (deact || !m->inputs[VCO<ot_twist>::RETRIGGER].isConnected())
    {
        o->init(pitch0);
    }
}

template <> void VCOConfig<ot_twist>::addMenuItems(VCO<ot_twist> *m, rack::ui::Menu *toThis)
{
    auto l1 = (int)std::round(m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 1].getValue());

    toThis->addChild(rack::createMenuItem("Randomize Twist Engine", CHECKMARK(l1), [m, l1]() {
        m->params[VCO<ot_twist>::ARBITRARY_SWITCH_0 + 1].setValue(l1 ? 0 : 1);
        m->paramQuantities[VCO<ot_twist>::OSC_CTRL_PARAM_0 + TwistOscillator::twist_engine]
            ->randomizeEnabled = !l1;
    }));
}

} // namespace sst::surgext_rack::vco

#endif
