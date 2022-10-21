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

#include "LFO.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "LayoutEngine.h"

namespace sst::surgext_rack::lfo::ui
{
struct LFOWidget : widgets::XTModuleWidget
{
    typedef lfo::LFO M;
    LFOWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, M::n_lfo_params> overlays;
    std::array<widgets::ModulatableKnob *, M::n_lfo_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
};

struct LFOTypeWidget : rack::app::ParamWidget, style::StyleParticipant
{
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};

    static constexpr int nTypes{8};
    std::array<std::shared_ptr<rack::Svg>, nTypes> svgs;

    static LFOTypeWidget *create(float widthInScrews, LFO *module, int param)
    {
        auto res = new LFOTypeWidget();
        res->box.pos.x = rack::mm2px(widgets::LCDBackground::contentPosX_MM);
        res->box.pos.y = widgets::LCDBackground::posy + rack::mm2px(1);
        res->box.size.x = rack::app::RACK_GRID_WIDTH * widthInScrews -
                          rack::mm2px(2 * widgets::LCDBackground::contentPosX_MM);
        res->box.size.y = rack::mm2px(6);
        res->module = module;
        res->paramId = param;
        res->initParamQuantity();

        res->setup();

        return res;
    }

    void setup()
    {
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { this->drawWidget(vg); });
        addChild(bdw);
        reloadGlyphs();
    }

    int sel{0};
    void step() override
    {
        int nsel = sel;
        if (module)
        {
            auto lf = static_cast<LFO *>(module);
            nsel = lf->lfostorage->shape.val.i;
        }
        if (nsel != sel)
            bdw->dirty = true;
        sel = nsel;
    }

    void drawWidget(NVGcontext *vg)
    {
        auto dSpan = box.size.x / nTypes;
        auto col = style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT);
        for (int i = 0; i < nTypes; ++i)
        {
            auto x = dSpan * i;
            auto y = 0;
            auto w = dSpan;
            auto h = box.size.y;

            auto pt = col;
            if (i == sel)
            {
                pt = style()->getColor(style::XTStyle::LED_PANEL);
                nvgBeginPath(vg);
                nvgRect(vg, x, y, w, h);
                nvgFillColor(vg, col);
                nvgFill(vg);
            }

            if (i != 0)
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, x, 0);
                nvgLineTo(vg, x, box.size.x);
                nvgStrokeWidth(vg, 0.5);
                nvgStrokeColor(vg, col);
                nvgStroke(vg);
            }

            auto hn = svgs[i]->handle;
            for (auto s = hn->shapes; s; s = s->next)
            {
                s->fill.color = (255 << 24) + (((int)(pt.b * 255)) << 16) +
                                (((int)(pt.g * 255)) << 8) + (int)(pt.r * 255);
            }

            float centerFudge = 1;
            nvgSave(vg);
            nvgTranslate(vg, x + centerFudge, y);
            rack::svgDraw(vg, svgs[i]->handle);
            nvgRestore(vg);
        }
        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, box.size.y);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgStrokeWidth(vg, 1.0);
        nvgStrokeColor(vg, col);
        nvgStroke(vg);
    }

    void onButton(const ButtonEvent &e) override
    {
        if (!module || !module->paramQuantities[LFO::SHAPE])
            return;

        if (e.action == GLFW_PRESS)
        {
            auto dSpan = box.size.x / (nTypes);
            float which = std::floor(e.pos.x / dSpan);
            module->paramQuantities[LFO::SHAPE]->setValue(which / (n_lfo_types - 1));
            bdw->dirty = true;
            e.consume(this);
        }
    }

    void reloadGlyphs()
    {
        for (int i = 0; i < nTypes; ++i)
        {
            auto svg = rack::Svg::load(rack::asset::plugin(
                pluginInstance, std::string("res/xt/glyphs/lt_") + std::to_string(i) + ".svg"));
            svgs[i] = svg;
        }
    }
    void onStyleChanged() override
    {
        bdw->dirty = true;
        reloadGlyphs();
    }
    void onChange(const ChangeEvent &e) override { bdw->dirty = true; }
};

struct LFOWaveform : rack::Widget, style::StyleParticipant
{
    LFO *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwBG{nullptr}, *bdwLight{nullptr};
    void setup()
    {
        bdwBG = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                        [this](auto vg) { this->drawBG(vg); });
        addChild(bdwBG);
        bdwLight = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto vg) { this->drawLight(vg); });
        addChild(bdwLight);
        memset(tp, 0, n_scene_params * sizeof(pdata));
    }

    void onStyleChanged() override
    {
        bdwLight->dirty = true;
        bdwBG->dirty = true;
    }

    void drawBG(NVGcontext *vg) {}

    void drawLight(NVGcontext *vg)
    {
        if (!module)
            return;
        drawWaveform(vg);
    }

    pdata tp[n_scene_params];
    void step() override
    {
        if (!module)
            return;

        auto lfodata = module->lfostorageDisplay;
        auto storage = module->storage.get();

        bool changed = false;
        changed = changed || (tp[lfodata->delay.param_id_in_scene].i != lfodata->delay.val.i);
        changed = changed || (tp[lfodata->attack.param_id_in_scene].i != lfodata->attack.val.i);
        changed = changed || (tp[lfodata->hold.param_id_in_scene].i != lfodata->hold.val.i);
        changed = changed || (tp[lfodata->decay.param_id_in_scene].i != lfodata->decay.val.i);
        changed = changed || (tp[lfodata->sustain.param_id_in_scene].i != lfodata->sustain.val.i);
        changed = changed || (tp[lfodata->release.param_id_in_scene].i != lfodata->release.val.i);

        changed =
            changed || (tp[lfodata->magnitude.param_id_in_scene].i != lfodata->magnitude.val.i);
        changed = changed || (tp[lfodata->rate.param_id_in_scene].i != lfodata->rate.val.i);
        changed = changed || (tp[lfodata->shape.param_id_in_scene].i != lfodata->shape.val.i);
        changed =
            changed || (tp[lfodata->start_phase.param_id_in_scene].i != lfodata->start_phase.val.i);
        changed = changed || (tp[lfodata->deform.param_id_in_scene].i != lfodata->deform.val.i);

        if (changed)
        {
            bdwLight->dirty = true;
            bdwBG->dirty = true;
        }
    }
    void drawWaveform(NVGcontext *vg)
    {
        auto lfodata = module->lfostorageDisplay;
        auto storage = module->storage.get();

        tp[lfodata->delay.param_id_in_scene].i = lfodata->delay.val.i;
        tp[lfodata->attack.param_id_in_scene].i = lfodata->attack.val.i;
        tp[lfodata->hold.param_id_in_scene].i = lfodata->hold.val.i;
        tp[lfodata->decay.param_id_in_scene].i = lfodata->decay.val.i;
        tp[lfodata->sustain.param_id_in_scene].i = lfodata->sustain.val.i;
        tp[lfodata->release.param_id_in_scene].i = lfodata->release.val.i;

        tp[lfodata->magnitude.param_id_in_scene].i = lfodata->magnitude.val.i;
        tp[lfodata->rate.param_id_in_scene].i = lfodata->rate.val.i;
        tp[lfodata->shape.param_id_in_scene].i = lfodata->shape.val.i;
        tp[lfodata->start_phase.param_id_in_scene].i = lfodata->start_phase.val.i;
        tp[lfodata->deform.param_id_in_scene].i = lfodata->deform.val.i;
        tp[lfodata->trigmode.param_id_in_scene].i = lm_keytrigger;

        float susTime = 0.5;
        bool msegRelease = false;
        float msegReleaseAt = 0;
        float lfoEnvelopeDAHDTime =
            pow(2.0f, lfodata->delay.val.f) + pow(2.0f, lfodata->attack.val.f) +
            pow(2.0f, lfodata->hold.val.f) + pow(2.0f, lfodata->decay.val.f);

        float totalEnvTime =
            lfoEnvelopeDAHDTime + std::min(pow(2.0f, lfodata->release.val.f), 4.f) +
            0.5; // susTime; this is now 0.5 to keep the envelope fixed in gate mode

        float rateInHz = pow(2.0, (double)lfodata->rate.val.f);
        if (lfodata->rate.temposync)
            rateInHz *= storage->temposyncratio;

        /*
         * What we want is no more than 50 wavelengths. So
         *
         * totalEnvTime * rateInHz < 50
         *
         * totalEnvTime < 50 / rateInHz
         *
         * so
         */
        totalEnvTime = std::min(totalEnvTime, 50.f / rateInHz);

        std::unique_ptr<LFOModulationSource> tlfo = std::make_unique<LFOModulationSource>();
        tlfo->assign(storage, lfodata, tp, nullptr, module->surge_ss.get(), module->surge_ms.get(),
                     module->surge_fs.get(), true);
        tlfo->attack();

        bool hasFullWave = false, waveIsAmpWave = false;

        bool drawEnvelope = !lfodata->delay.deactivated;

        int minSamples = (1 << 0) * (int)(box.size.x);
        int totalSamples =
            std::max((int)minSamples, (int)(totalEnvTime * storage->samplerate / BLOCK_SIZE));
        float drawnTime = totalSamples * storage->samplerate_inv * BLOCK_SIZE;

        // OK so let's assume we want about 1000 pixels worth tops in
        int averagingWindow = (int)(totalSamples / 1000.0) + 1;

        float valScale = 100.0;
        int susCountdown = -1;

        float priorval = 0.f, priorwval = 0.f;

        std::string invalidMessage;

        std::vector<std::pair<float, float>> pathV, eupathV, edpathV;

        for (int i = 0; i < totalSamples; i += averagingWindow)
        {
            float val = 0;
            float wval = 0;
            float eval = 0;
            float minval = 1000000, minwval = 1000000;
            float maxval = -1000000, maxwval = -1000000;
            float firstval;
            float lastval;

            for (int s = 0; s < averagingWindow; s++)
            {
                tlfo->process_block();

                if (susCountdown < 0 && tlfo->env_state == lfoeg_stuck)
                {
                    susCountdown = susTime * storage->samplerate / BLOCK_SIZE;
                }
                else if (susCountdown == 0 && tlfo->env_state == lfoeg_stuck)
                {
                    tlfo->release();
                }
                else if (susCountdown > 0)
                {
                    susCountdown--;
                }

                val += tlfo->get_output(0);

                if (s == 0)
                {
                    firstval = tlfo->get_output(0);
                }

                if (s == averagingWindow - 1)
                {
                    lastval = tlfo->get_output(0);
                }

                minval = std::min(tlfo->get_output(0), minval);
                maxval = std::max(tlfo->get_output(0), maxval);
                eval += tlfo->env_val * lfodata->magnitude.get_extended(lfodata->magnitude.val.f);
            }

            val = val / averagingWindow;
            wval = wval / averagingWindow;
            eval = eval / averagingWindow;
            val = ((-val + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
            wval = ((-wval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
            minwval = ((-minwval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
            maxwval = ((-maxwval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;

            float euval = ((-eval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
            float edval = ((eval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
            float xc = valScale * i / totalSamples;

            if (i == 0)
            {
                pathV.emplace_back(xc, val);
                eupathV.emplace_back(xc, euval);
                edpathV.emplace_back(xc, edval);

                priorval = val;
                priorwval = wval;
            }
            else
            {
                minval = ((-minval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
                maxval = ((-maxval + 1.0f) * 0.5 * 0.8 + 0.1) * valScale;
                // Windows is sensitive to out-of-order line draws in a way which causes spikes.
                // Make sure we draw one closest to prior first. See #1438
                float firstval = minval;
                float secondval = maxval;

                if (priorval - minval < maxval - priorval)
                {
                    firstval = maxval;
                    secondval = minval;
                }

                pathV.emplace_back(xc - 0.1 * valScale / totalSamples, firstval);
                pathV.emplace_back(xc + 0.1 * valScale / totalSamples, secondval);

                priorval = val;
                eupathV.emplace_back(xc, euval);
                edpathV.emplace_back(xc, edval);
            }
        }

        tlfo->completedModulation();
        {
            bool first{true};
            nvgBeginPath(vg);
            auto sx = box.size.x / valScale, sy = box.size.y / valScale;
            for (const auto &[x, y] : eupathV)
            {
                if (first)
                {
                    nvgMoveTo(vg, sx * x, sy * y);
                }
                else
                {
                    nvgLineTo(vg, sx * x, sy * y);
                }
                first = false;
            }
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
        }
        {
            bool first{true};
            nvgBeginPath(vg);
            auto sx = box.size.x / valScale, sy = box.size.y / valScale;
            for (const auto &[x, y] : edpathV)
            {
                if (first)
                {
                    nvgMoveTo(vg, sx * x, sy * y);
                }
                else
                {
                    nvgLineTo(vg, sx * x, sy * y);
                }
                first = false;
            }
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
        }
        {
            bool first{true};
            nvgBeginPath(vg);
            auto sx = box.size.x / valScale, sy = box.size.y / valScale;
            for (const auto &[x, y] : pathV)
            {
                if (first)
                {
                    nvgMoveTo(vg, sx * x, sy * y);
                }
                else
                {
                    nvgLineTo(vg, sx * x, sy * y);
                }
                first = false;
            }
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgStrokeWidth(vg, 1.25);
            nvgStroke(vg);
        }

#if 0
        if (tFullWave)
        {
            tFullWave->completedModulation();
            delete tFullWave;
        }

        auto at = juce::AffineTransform()
                      .scale(waveform_display.getWidth() / valScale,
                             waveform_display.getHeight() / valScale)
                      .translated(waveform_display.getTopLeft());

        for (int i = -1; i < 2; ++i)
        {
            float off = i * 0.4 + 0.5; // so that/s 0.1, 0.5, 0.9
            float x0 = 0, y0 = valScale * off, x1 = valScale, y1 = valScale * off;

            at.transformPoint(x0, y0);
            at.transformPoint(x1, y1);

            if (i == 0)
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::Center));
            }
            else
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::Bounds));
            }

            g.drawLine(x0, y0, x1, y1, 1);
        }

        auto pointColor = skin->getColor(Colors::LFO::Waveform::Dots);
        int nxd = 61, nyd = 9;

        for (int xd = 0; xd < nxd; xd++)
        {
            float normx = 1.f * xd / (nxd - 1) * 0.99;
            for (int yd = 1; yd < nyd - 1; yd++)
            {
                if (yd == (nyd - 1) / 2)
                    continue;

                float normy = 1.f * yd / (nyd - 1);
                float dotPointX = normx * valScale, dotPointY = (0.8 * normy + 0.1) * valScale;
                at.transformPoint(dotPointX, dotPointY);
                float esize = 1.f;
                float xoff = (xd == 0 ? esize : 0);
                g.setColour(pointColor);
                g.fillEllipse(dotPointX - esize, dotPointY - esize, esize, esize);
            }
        }

        if (drawEnvelope)
        {
            g.setColour(skin->getColor(Colors::LFO::Waveform::Envelope));
            g.strokePath(eupath, juce::PathStrokeType(1.f), at);

            if (!isUnipolar())
            {
                g.strokePath(edpath, juce::PathStrokeType(1.f), at);
            }
        }

        if (drawBeats)
        {
            // calculate beat grid related data
            auto bpm = storage->temposyncratio * 120;

            auto denFactor = 4.0 / tsDen;
            auto beatsPerSecond = bpm / 60.0;
            auto secondsPerBeat = 1 / beatsPerSecond;
            auto deltaBeat = secondsPerBeat / drawnTime * valScale * denFactor;

            int nBeats = std::max(ceil(drawnTime * beatsPerSecond / denFactor), 1.0);

            auto measureLen = deltaBeat * tsNum;
            int everyMeasure = 1;

            while (measureLen < 4) // a hand selected parameter by playing around with it, tbh
            {
                measureLen *= 2;
                everyMeasure *= 2;
            }

            for (auto l = 0; l <= nBeats; ++l)
            {
                auto xp = deltaBeat * l;

                if (l % (tsNum * everyMeasure) == 0 || nBeats <= tsDen)
                {
                    auto soff = 0.0;
                    if (l > 10)
                        soff = 0.0;
                    float vruleSX = xp, vruleSY = valScale * .15, vruleEX = xp,
                          vruleEY = valScale * .85;
                    at.transformPoint(vruleSX, vruleSY);
                    at.transformPoint(vruleEX, vruleEY);
                    // major beat divisions on the LFO waveform bg
                    g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::ExtendedTicks));
                    g.drawLine(vruleSX, vruleSY, vruleEX, vruleEY, 1);

                    juce::Point<float> sp(xp, valScale * (.01)), ep(xp, valScale * (.1));
                    sp = sp.transformedBy(at);
                    ep = ep.transformedBy(at);
                    // ticks for major beats
                    if (l % tsNum == 0)
                        g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Ticks));
                    else
                        // small ticks for the ruler for nbeats case
                        g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::SmallTicks));

                    g.drawLine(sp.getX(), sp.getY(), ep.getX(), ep.getY(), 1.0);

                    char s[TXT_SIZE];
                    snprintf(s, TXT_SIZE, "%d", l + 1);

                    juce::Point<int> tp(xp + 1, valScale * 0.0);
                    tp = tp.transformedBy(at);
                    g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Text));
                    g.setFont(skin->fontManager->lfoTypeFont);
                    g.drawText(s, tp.x, tp.y, 20, 10, juce::Justification::bottomLeft);
                }
                else if (everyMeasure == 1)
                {
                    auto sp = juce::Point<float>(xp, valScale * 0.06).transformedBy(at);
                    auto ep = juce::Point<float>(xp, valScale * 0.1).transformedBy(at);

                    if (l % tsNum == 0)
                        g.setColour(juce::Colours::black);
                    else
                        // small ticks for the ruler
                        g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::SmallTicks));
                    g.drawLine(sp.x, sp.y, ep.x, ep.y, 0.5);
                }
            }
        }

        if (hasFullWave)
        {
            if (waveIsAmpWave)
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::GhostedWave));
                auto dotted = juce::Path();
                auto st = juce::PathStrokeType(0.3, juce::PathStrokeType::beveled,
                                               juce::PathStrokeType::butt);
                float dashLength[2] = {4.f, 2.f};
                st.createDashedStroke(dotted, deactPath, dashLength, 2, at);
                g.strokePath(dotted, st);
            }
            else
            {
                g.setColour(skin->getColor(Colors::LFO::Waveform::DeactivatedWave));
                g.strokePath(deactPath,
                             juce::PathStrokeType(0.5f, juce::PathStrokeType::beveled,
                                                  juce::PathStrokeType::butt),
                             at);
            }
        }

        g.setColour(skin->getColor(Colors::LFO::Waveform::Wave));
        g.strokePath(
            path,
            juce::PathStrokeType(1.f, juce::PathStrokeType::beveled, juce::PathStrokeType::butt),
            at);

        // lower ruler calculation
        // find time delta
        int maxNumLabels = 5;
        std::vector<float> timeDeltas = {0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0};
        auto currDelta = timeDeltas.begin();
        while (currDelta != timeDeltas.end() && (drawnTime / *currDelta) > maxNumLabels)
        {
            currDelta++;
        }
        float delta = timeDeltas.back();
        if (currDelta != timeDeltas.end())
            delta = *currDelta;

        int nLabels = (int)(drawnTime / delta) + 1;
        float dx = delta / drawnTime * valScale;

        for (int l = 0; l < nLabels; ++l)
        {
            float xp = dx * l;
            float yp = valScale * 0.9;
            float typ = yp;
            auto tp = juce::Point<float>(xp + 0.5, typ + 0.5).transformedBy(at);
            g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Text));
            g.setFont(skin->fontManager->lfoTypeFont);
            char txt[TXT_SIZE];
            float tv = delta * l;
            if (fabs(roundf(tv) - tv) < 0.05)
                snprintf(txt, TXT_SIZE, "%d s", (int)round(delta * l));
            else if (delta < 0.1)
                snprintf(txt, TXT_SIZE, "%.2f s", delta * l);
            else
                snprintf(txt, TXT_SIZE, "%.1f s", delta * l);
            g.drawText(txt, tp.x, tp.y, 30, 10, juce::Justification::topLeft);

            auto sp = juce::Point<float>(xp, valScale * 0.95).transformedBy(at);
            auto ep = juce::Point<float>(xp, valScale * 0.9).transformedBy(at);

            g.setColour(skin->getColor(Colors::LFO::Waveform::Ruler::Ticks));
            g.drawLine(sp.x, sp.y, ep.x, ep.y, 1.0);
        }

        /*
         * In 1.8 I think we don't want to show the key release point in the simulated wave
         * with the MSEG but I wrote it to debug and we may change our mind so keeping this code
         * here
         */
        if (msegRelease && false)
        {
#if SHOW_RELEASE_TIMES
            float xp = msegReleaseAt / drawnTime * valScale;
            was a vstgui Point sp(xp, valScale * 0.9), ep(xp, valScale * 0.1);
            tf.transform(sp);
            tf.transform(ep);
            dc->setFrameColor(juce::Colours::red);
            dc->drawLine(sp, ep);
#endif
        }

        if (warnForInvalid)
        {
            g.setColour(skin->getColor(Colors::LFO::Waveform::Wave));
            g.setFont(skin->fontManager->getLatoAtSize(14, juce::Font::bold));
            g.drawText(invalidMessage, waveform_display.withTrimmedBottom(30),
                       juce::Justification::centred);
        }
#endif
    }
};

LFOWidget::LFOWidget(LFOWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    int screwWidth = 20;
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * screwWidth, rack::app::RACK_GRID_HEIGHT);

    typedef layout::LayoutEngine<LFOWidget, M::RATE> engine_t;
    engine_t::initializeModulationToBlank(this);

    auto bg = new widgets::Background(box.size, "LFO x EG", "other", "EGLFO");
    addChild(bg);

    typedef layout::LayoutItem li_t;

    {
        const auto col = layout::LayoutConstants::firstColumnCenter_MM;
        const auto cw = layout::LayoutConstants::lfoColumnWidth_MM;
        const auto row1 = layout::LayoutConstants::vcoRowCenters_MM[1];
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::KNOB9, "RATE", M::RATE, col, row1},
                {li_t::KNOB9, "PHASE", M::PHASE, col + cw, row1},
                {li_t::KNOB9, "DEFORM", M::DEFORM, col + 2 * cw, row1},
                {li_t::KNOB9, "AMP", M::AMPLITUDE, col + 3 * cw, row1},
                li_t::createGrouplabel("WAVEFORM", col, row1, 4, cw)
         };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }
    }

    {
        const auto knobCenter = layout::LayoutConstants::vcoRowCenters_MM[1];
        // We want the top of the sliders and top of the knobs to align
        const auto row1 = knobCenter + 4.75f;

        const auto middleOutputCenter = layout::LayoutConstants::firstColumnCenter_MM +
                                        layout::LayoutConstants::lfoColumnWidth_MM * 5;

        float cw = layout::LayoutConstants::vSliderSpacing_MM;
        float c0 = middleOutputCenter - 2.5 * cw;
        // fixme use the enums
        // clang-format off
         std::vector<li_t> layout = {
                {li_t::VSLIDER, "D", M::E_DELAY, c0, row1},
                {li_t::VSLIDER, "A", M::E_ATTACK, c0 + cw, row1},
                {li_t::VSLIDER, "H", M::E_HOLD, c0 + 2 * cw, row1},
                {li_t::VSLIDER, "D", M::E_DECAY, c0 + 3 * cw, row1},
                {li_t::VSLIDER, "S", M::E_SUSTAIN, c0 + 4 * cw, row1},
                {li_t::VSLIDER, "R", M::E_RELEASE, c0 + 5 *cw, row1},
        };
        // clang-format on

        for (const auto &lay : layout)
        {
            engine_t::layoutItem(this, lay, "LFO");
        }

        // OK this is a bit hacky. We know we want 40mm centered over the middle
        auto gl = new widgets::GroupLabel();
        auto ht = rack::mm2px(4.5);
        auto yup = rack::mm2px(1.75);
        gl->label = "ENVELOPE";
        gl->box.pos.x = rack::mm2px(middleOutputCenter - 20);
        gl->box.size.x = rack::mm2px(40);
        gl->box.pos.y = rack::mm2px(knobCenter - 8) - yup;
        gl->box.size.y = ht;
        gl->setup();
        addChild(gl);
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIG", "CLK RATE", "CLK ENV", "PHASE"};
        for (auto p :
             {M::INPUT_TRIGGER, M::INPUT_CLOCK_RATE, M::INPUT_CLOCK_ENV, M::INPUT_PHASE_DIRECT})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::lfoColumnWidth_MM * col;
            addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                              module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto lab = engine_t::makeLabelAt(bl, col, labv[col], style::XTStyle::TEXT_LABEL,
                                             layout::LayoutConstants::lfoColumnWidth_MM);
            addChild(lab);

            col++;
        }
    }

    auto ht = layout::LayoutConstants::rowStart_MM - 11;
    auto lcd = widgets::LCDBackground::createWithHeight(ht, screwWidth);
    addChild(lcd);

    auto tw = LFOTypeWidget::create(screwWidth, module, M::SHAPE);
    addChild(tw);

    auto ww = new LFOWaveform();
    ww->module = module;
    ww->box.pos.x = tw->box.pos.x;
    ww->box.pos.y = tw->box.pos.y + tw->box.size.y;
    ww->box.size.x = tw->box.size.x;
    ww->box.size.y = rack::mm2px(ht) - ww->box.pos.y - rack::mm2px(6);
    ww->setup();
    addChild(ww);

    {
        int col = 0;
        std::vector<std::string> labv{"LFO", "EG", "LFOxEG"};
        for (auto p : {M::OUTPUT_WAVE, M::OUTPUT_ENV, M::OUTPUT_MIX})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::lfoColumnWidth_MM * (3 + col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto cx = xp;

            auto boxx0 = cx - layout::LayoutConstants::lfoColumnWidth_MM * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::lfoColumnWidth_MM, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, labv[col]);
            addChild(lab);

            col++;
        }
    }

    {
        int col = 0;
        std::vector<std::string> labv{"TRIGA", "TRIGB", "EOC"};
        for (auto p : {M::OUTPUT_TRIGF, M::OUTPUT_TRIGA, M::OUTPUT_TRIGPHASE})
        {
            auto yp = layout::LayoutConstants::modulationRowCenters_MM[1];
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::lfoColumnWidth_MM * (3 + col + 1);
            addOutput(rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                module, p));

            auto bl = yp - 5;
            auto cx = xp;

            auto boxx0 = cx - layout::LayoutConstants::lfoColumnWidth_MM * 0.5;
            auto boxy0 = bl - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(layout::LayoutConstants::lfoColumnWidth_MM, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, labv[col]);
            addChild(lab);

            col++;
        }
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::LFO_MOD_INPUT, 0,
                                   layout::LayoutConstants::lfoColumnWidth_MM);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::lfo::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeLFO = rack::createModel<sst::surgext_rack::lfo::ui::LFOWidget::M,
                                               sst::surgext_rack::lfo::ui::LFOWidget>("SurgeXTLFO");
