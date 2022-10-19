#include "VCO.h"
#include "VCOConfig.h"
#include "XTWidgets.h"

#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "osdialog.h"

namespace sst::surgext_rack::vco::ui
{
template <int oscType> struct VCOWidget : public widgets::XTModuleWidget
{
    typedef VCO<oscType> M;
    VCOWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, 8> overlays;
    std::array<widgets::ModulatableKnob *, 8> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }

    virtual void characterMenu(rack::Menu *p, M *m)
    {
        if (!m || !m->paramQuantities[M::CHARACTER])
            return;
        auto pq = m->paramQuantities[M::CHARACTER];
        auto pqvi = (int)std::round(pq->getValue());

        for (auto c : {cm_warm, cm_neutral, cm_bright})
        {
            p->addChild(rack::createMenuItem(character_names[c], CHECKMARK(pqvi == c),
                                             [pq, c]() { pq->setValue(c); }));
        }
    }

    virtual void appendModuleSpecificMenu(rack::ui::Menu *menu) override
    {
        if (module)
        {
            auto m = static_cast<M *>(module);
            menu->addChild(new rack::ui::MenuSeparator);

            auto addBoolMenu = [menu, m](auto *l, auto p) {
                if (!m || !m->paramQuantities[p])
                    return;

                auto v = m->paramQuantities[p]->getValue() > 0.5;
                menu->addChild(rack::createMenuItem(
                    l, CHECKMARK(v), [m, v, p]() { m->paramQuantities[p]->setValue(v ? 0 : 1); }));
            };

            addBoolMenu("Retrigger With Phase=0", M::RETRIGGER_STYLE);
            if (VCOConfig<oscType>::supportsUnison())
            {
                addBoolMenu("Extend Unison Detune", M::EXTEND_UNISON);
                addBoolMenu("Absolute Unison Detune", M::ABSOLUTE_UNISON);
            }
            menu->addChild(rack::createSubmenuItem("Character", "",
                                                   [this, m](auto *x) { characterMenu(x, m); }));

            auto driftSlider = new rack::ui::Slider;
            driftSlider->quantity = module->paramQuantities[M::DRIFT];
            driftSlider->box.size.x = 125;
            menu->addChild(driftSlider);

            VCOConfig<oscType>::addMenuItems(m, menu);
        }
    }
};

template <int oscType> struct WavetableSelector : widgets::PresetJogSelector
{
    VCO<oscType> *module{nullptr};
    uint32_t wtloadCompare{842932918};

    static WavetableSelector *create(const rack::Vec &pos, const rack::Vec &size,
                                     VCO<oscType> *module)
    {
        auto res = new WavetableSelector();
        res->box.pos = pos;
        res->box.size = size;
        res->module = module;
        res->setup();

        return res;
    }
    int wtNo{100};
    void onPresetJog(int dir) override
    {
        if (!module)
            return;
        // FIX ME - ordering aware jog pls
        bool wantNext = dir > 0;
        auto nt = module->storage->getAdjacentWaveTable(module->wavetableIndex, wantNext);
        sendLoadFor(nt);
    }

    void sendLoadFor(int nt)
    {
        auto msg = typename vco::VCO<oscType>::WavetableMessage();
        msg.index = nt;
        module->wavetableQueue.push(msg);
    }

    void sendLoadForPath(const char *fn)
    {
        auto msg = typename vco::VCO<oscType>::WavetableMessage();
        strncpy(msg.filename, fn, 256);
        msg.filename[255] = 0;
        msg.index = -1;
        module->wavetableQueue.push(msg);
    }

    rack::ui::Menu *menuForCategory(rack::ui::Menu *menu, int categoryId)
    {
        if (!module)
            return nullptr;
        auto storage = module->storage.get();
        auto &cat = storage->wt_category[categoryId];

        for (auto p : storage->wtOrdering)
        {
            if (storage->wt_list[p].category == categoryId)
            {
                menu->addChild(rack::createMenuItem(storage->wt_list[p].name, "",
                                                    [this, p]() { this->sendLoadFor(p); }));
            }
        }
        // menu->addChild(rack::createMenuItem(name));
        for (auto child : cat.children)
        {
            if (child.numberOfPatchesInCategoryAndChildren > 0)
            {
                // this isn't the best approach but it works
                int cidx = 0;

                for (auto &cc : storage->wt_category)
                {
                    if (cc.name == child.name)
                    {
                        break;
                    }

                    cidx++;
                }
                auto &kidcat = storage->wt_category[cidx];

                std::string catName = kidcat.name;
                std::size_t sepPos = catName.find_last_of(PATH_SEPARATOR);
                if (sepPos != std::string::npos)
                {
                    catName = catName.substr(sepPos + 1);
                }

                menu->addChild(rack::createSubmenuItem(
                    catName, "", [cidx, this](auto *x) { this->menuForCategory(x, cidx); }));
            }
        }

        return menu;
    }

    void onShowMenu() override
    {
        if (!module)
            return;
        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel("WaveTables"));
        auto storage = module->storage.get();
        int idx{0};
        bool addSepIfMaking{false};
        for (auto c : storage->wtCategoryOrdering)
        {
            PatchCategory cat = storage->wt_category[c];

            if (idx == storage->firstThirdPartyWTCategory ||
                (idx == storage->firstUserWTCategory &&
                 storage->firstUserWTCategory != storage->wt_category.size()))
            {
                addSepIfMaking = true;
            }

            idx++;

            if (cat.numberOfPatchesInCategoryAndChildren == 0)
            {
                continue;
            }

            if (cat.isRoot)
            {
                if (addSepIfMaking)
                {
                    menu->addChild(new rack::ui::MenuSeparator);
                    addSepIfMaking = false;
                }
                menu->addChild(rack::createSubmenuItem(
                    cat.name, "", [c, this](auto *x) { return menuForCategory(x, c); }));
            }
        }
        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(rack::createMenuItem("Open User Wavetables", "", [this]() {
            rack::system::openDirectory((module->storage->userDataPath / "Wavetables").u8string());
        }));

        menu->addChild(rack::createMenuItem("Rescan Wavetables", "",
                                            [this]() { module->storage->refresh_wtlist(); }));

        menu->addChild(rack::createMenuItem("Load Wavetable File", "", [this]() {
            auto filters = osdialog_filters_parse("Wavetables:wav,.wt");
            DEFER({ osdialog_filters_free(filters); });
            char *openF = osdialog_file(OSDIALOG_OPEN, nullptr, nullptr, filters);
            if (openF)
            {
                DEFER({ std::free(openF); });
                sendLoadForPath(openF);
            }
        }));
    }

    std::string getPresetName() override
    {
        if (module)
            return module->getWavetableName();
        return "WaveTable Name";
    }

    bool isDirty() override
    {
        if (!module)
            return false;

        if (wtloadCompare != module->wavetableLoads)
        {
            wtloadCompare = module->wavetableLoads;
            return true;
        }

        return false; // cache wavetable load from module
    }
};

template <int oscType>
struct OSCPlotWidget : public rack::widget::TransparentWidget, style::StyleParticipant
{
    typename VCOWidget<oscType>::M *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwPlot{nullptr};
    void setup(typename VCOWidget<oscType>::M *m)
    {
        module = m;
        if (module)
        {
            storage = module->storage.get();
            oscdata = &(storage->getPatch().scene[0].osc[1]); // this is tne no-mod storage
        }
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto *vg) { drawPlotBackground(vg); });
        addChild(bdw);

        bdwPlot = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto *vg) { drawPlot(vg); });
        addChild(bdwPlot);
    }

    void step() override
    {
        if (!module)
            return;

        /*
         * if wavetable changed and draw wavetable bdw->dirty = true
         */

        if (isDirty())
        {
            recalcPath();
            bdwPlot->dirty = true;
        }

        if constexpr (VCOConfig<oscType>::requiresWavetables())
        {
            if (wtloadCompare != module->wavetableLoads)
            {
                wtloadCompare = module->wavetableLoads;
                recalcPath();
                bdw->dirty = true;
                bdwPlot->dirty = true;
            }
        }

        rack::widget::Widget::step();
    }

    virtual void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwPlot->dirty = true;
    }

    static OSCPlotWidget<oscType> *create(rack::Vec pos, rack::Vec size,
                                          typename VCOWidget<oscType>::M *module)
    {
        auto *res = rack::createWidget<OSCPlotWidget<oscType>>(pos);

        res->box.pos = pos;
        res->box.size = size;

        res->setup(module);

        return res;
    }

    bool firstDirty{false};
    int dirtyCount{0};
    int sumDeact{-1};
    int sumAbs{-1};
    int charF{-1};
    uint32_t wtloadCompare{842932918};

    bool isDirty()
    {
        if (!firstDirty)
        {
            firstDirty = true;
            return true;
        }

        bool dval{false};
        if (module)
        {
            auto lSumDeact = 0, lSumAbs = 0;
            for (int i = 0; i < n_osc_params; i++)
            {
                dval = dval || (tp[oscdata->p[i].param_id_in_scene].i != oscdata->p[i].val.i);
                lSumDeact += oscdata->p[i].deactivated * (1 << i);
                lSumAbs += oscdata->p[i].absolute * (1 << i);
            }

            if (lSumDeact != sumDeact || lSumAbs != sumAbs)
            {
                sumDeact = lSumDeact;
                sumAbs = lSumAbs;
                dval = true;
            }

            if (charF != storage->getPatch().character.val.i)
            {
                charF = storage->getPatch().character.val.i;
                dval = true;
            }
        }
        return dval;
    }

    pdata tp[n_scene_params];
    OscillatorStorage *oscdata{nullptr};
    SurgeStorage *storage{nullptr};
    unsigned char oscbuffer alignas(16)[oscillator_buffer_size];

    Oscillator *setupOscillator()
    {
        tp[oscdata->pitch.param_id_in_scene].f = 0;

        for (int i = 0; i < n_osc_params; i++)
        {
            tp[oscdata->p[i].param_id_in_scene].i = oscdata->p[i].val.i;
        }

        auto res = spawn_osc(oscdata->type.val.i, storage, oscdata, tp, oscbuffer);
        res->init_ctrltypes();
        return res;
    }

    std::vector<std::pair<float, float>> oscPath;
    void recalcPath()
    {
        oscPath.clear();

        if (module && module->draw3DWavetable)
        {
            auto wtlockguard = std::lock_guard<std::mutex>(module->storage->waveTableDataMutex);
            auto &wt = oscdata->wt;
            auto pos = -1.f;

            // OK so now go backwards through the tables but also tilt and raise for the 3D effect
            auto smp = wt.size;
            auto smpinv = 1.0 / smp;
            auto w = box.size.x;
            auto h = box.size.y;

            // Now we have a sort of skew back and offset as we go. The skew is sort of a rotation
            // and the depth is sort of how flattened it is. Finally the hCompress augments height.
            auto skewPct = 0.4;
            auto depthPct = 0.6;
            auto hCompress = 0.55;

            bool off = false;
            switch (oscdata->type.val.i)
            {
            case ot_wavetable:
            case ot_window:
                pos = oscdata->p[0].val.f;
                off = oscdata->p[0].extend_range;
                break;
            default:
                pos = 0.f;
                break;
            };

            auto tpos = pos * (wt.n_tables - off);

            auto sel = std::clamp(tpos, 0.f, (wt.n_tables - 1.f));
            auto tb = wt.TableF32WeakPointers[0][(int)std::floor(sel)];
            float tpct = 1.0 * sel / std::max((int)(wt.n_tables - 1), 1);

            if (wt.n_tables == 1)
            {
                tpct = 0.f;
            }

            float x0 = tpct * skewPct * w;
            float y0 = (1.0 - tpct) * depthPct * h;
            auto lw = w * (1.0 - skewPct);
            auto hw = h * depthPct * hCompress;

            auto osc = setupOscillator();

            if (!osc)
            {
                return;
            }

            int totalSamples = box.size.x;
            // We want totalSamples to be one cycle so
            // totalSamples = sampleRate / frequency
            // frequency = sampleRate / totalSamples
            // 440 * 2^((pitch-69)/12) = sampleRate / totalSamples
            // pitch-69 = 12 * log2(sampleRate / totalSamples / 400)
            // but these run oversample so add an octave
            float disp_pitch_rs =
                12.f * std::log2f(storage->samplerate / (totalSamples - 4) / 440.f) + 69.f + 12 +
                0.1;

            osc->init(disp_pitch_rs, true, true);

            int block_pos = BLOCK_SIZE_OS;

            for (int i = 0; i < totalSamples; i++)
            {
                if (block_pos >= BLOCK_SIZE_OS)
                {
                    osc->process_block(disp_pitch_rs);
                    block_pos = 0;
                }

                float val = 0.f;

                val = osc->output[block_pos];
                block_pos++;

                if (i >= 4)
                {

                    float xc = 1.f * (i - 4) / totalSamples;
                    float yc = val;

                    /*
                     * auto tf =
                        juce::AffineTransform().scaled(w * 0.61, h * -0.17).translated(x0, y0 + (0.5
                     * hw));
                     */
                    xc = xc * w * 0.61 + x0;
                    yc = yc * h * (-0.17) + (y0 + 0.5 * hw);

                    oscPath.emplace_back(xc, yc);
                }
            }

            osc->~Oscillator();
        }
        else
        {
            auto wtlockguard = std::lock_guard<std::mutex>(module->storage->waveTableDataMutex);
            auto xp = box.size.x;
            auto yp = box.size.y;

            auto osc = setupOscillator();
            const float ups = 3.0, invups = 1.0 / ups;

            float disp_pitch_rs =
                12.f * std::log2f((700.f * (storage->samplerate / 48000.f)) / 440.f) + 69.f;

            osc->init(disp_pitch_rs, true, true);

            int block_pos{BLOCK_SIZE_OS + 1};
            for (int i = 0; i < xp * ups; ++i)
            {
                if (block_pos >= BLOCK_SIZE_OS)
                {
                    osc->process_block(disp_pitch_rs);
                    block_pos = 0;
                }

                float yc = (-osc->output[block_pos] * 0.47 + 0.5) * yp;
                oscPath.emplace_back(i * invups, yc);
                block_pos++;
            }

            osc->~Oscillator();
        }
    }

    const float xs3d{rack::mm2px(5)};
    const float ys3d{rack::mm2px(3.5)};

    void drawPlotBackground(NVGcontext *vg)
    {
        if (module && VCOConfig<oscType>::requiresWavetables() && module->draw3DWavetable)
        {
            draw3DBackground(vg);
        }
        else
        {
            draw2DBackground(vg);
        }

        if (module && VCOConfig<oscType>::requiresWavetables())
        {
            bool on = module->draw3DWavetable;

            if (on)
            {
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
            }
            else
            {
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            }

            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(vg, xs3d * 0.5, ys3d * 0.5, "3D", nullptr);
        }
    }

    void onButton(const ButtonEvent &e) override
    {
        if (!module)
            return;
        if (e.pos.x < xs3d && e.pos.y < ys3d && e.action == GLFW_RELEASE)
        {
            module->draw3DWavetable = !module->draw3DWavetable;
            bdw->dirty = true;
            bdwPlot->dirty = true;
            recalcPath();
            e.consume(this);
            return;
        }

        TransparentWidget::onButton(e);
    }

    void draw3DBackground(NVGcontext *vg)
    {
        if (!module)
            return;

        auto wtlockguard = std::lock_guard<std::mutex>(module->storage->waveTableDataMutex);
        auto &wt = oscdata->wt;
        auto pos = -1.f;

        // OK so now go backwards through the tables but also tilt and raise for the 3D effect
        auto smp = wt.size;
        auto smpinv = 1.0 / smp;
        auto w = box.size.x;
        auto h = box.size.y;

        // Now we have a sort of skew back and offset as we go. The skew is sort of a rotation
        // and the depth is sort of how flattened it is. Finally the hCompress augments height.
        auto skewPct = 0.4;
        auto depthPct = 0.6;
        auto hCompress = 0.55;

        // calculate thinning factor for frame drawing
        int thintbl = 1;
        int nt = wt.n_tables;

        while (nt > 16)
        {
            thintbl <<= 1;
            nt >>= 1;
        }

        // calculate thinning factor for sample drawing
        int thinsmp = 1;
        int s = smp;

        while (s > 128)
        {
            thinsmp <<= 1;
            s >>= 1;
        }

        static constexpr float backingScale = 2.f;

        auto wxf = w;
        auto hxf = h;

        // draw the wavetable frames
        std::vector<int> ts;

        for (int t = wt.n_tables - 1; t >= 0; t = t - thintbl)
        {
            ts.push_back(t);
        }

        if (ts.back() != 0)
        {
            ts.push_back(0);
        }

        for (auto t : ts)
        {
            auto tb = wt.TableF32WeakPointers[0][t];
            float tpct = 1.0 * t / std::max((int)(wt.n_tables - 1), 1);

            if (wt.n_tables == 1)
            {
                tpct = 0.f;
            }

            float x0 = tpct * skewPct * wxf;
            float y0 = (1.0 - tpct) * depthPct * hxf;
            auto lw = wxf * (1.0 - skewPct);
            auto hw = hxf * depthPct * hCompress;

            std::vector<std::pair<float, float>> p;
            std::vector<std::pair<float, float>> ribbon;

            p.emplace_back(x0, y0 + (-tb[0] + 1) * 0.5 * hw);
            p.emplace_back(x0, y0 + (-tb[0] + 1) * 0.5 * hw);

            for (int s = 1; s < smp; s = s + thinsmp)
            {
                auto x = x0 + s * smpinv * lw;

                p.template emplace_back(x, y0 + (-tb[s] + 1) * 0.5 * hw);
                ribbon.emplace_back(x, y0 + (-tb[s] + 1) * 0.5 * hw);
            }

            if (t > 0)
            {
                nt = std::max(t - thintbl, 0);
                tpct = 1.0 * nt / (wt.n_tables - 1);
                tb = wt.TableF32WeakPointers[0][nt];
                x0 = tpct * skewPct * wxf;
                y0 = (1.0 - tpct) * depthPct * hxf;
                lw = w * (1.0 - skewPct);

                for (int s = smp - 1; s >= 0; s = s - thinsmp)
                {
                    auto x = x0 + s * smpinv * lw;

                    ribbon.template emplace_back(x, y0 + (-tb[s] + 1) * 0.5 * hw);
                }
            }

            if (t > 0)
            {
                nvgBeginPath(vg);
                bool first = true;
                for (const auto &[x, y] : ribbon)
                {
                    if (first)
                        nvgMoveTo(vg, x, y);
                    else
                        nvgLineTo(vg, x, y);
                    first = false;
                }
                auto pc = style()->getColor(style::XTStyle::PLOT_CURVE);
                pc.a = 1.f * 0x20 / 0xFF;
                nvgFillColor(vg, pc);
                nvgFill(vg);
            }

            nvgBeginPath(vg);
            bool first = true;
            for (const auto &[x, y] : p)
            {
                if (first)
                    nvgMoveTo(vg, x, y);
                else
                    nvgLineTo(vg, x, y);
                first = false;
            }
            auto pc = style()->getColor(style::XTStyle::PLOT_CURVE);
            pc.a = 1.f * 0x60 / 0xFF;

            nvgStrokeColor(vg, pc);
            nvgStroke(vg);
        }
    }

    void draw2DBackground(NVGcontext *vg)
    {
        // This will go in layer 0
        int nSteps = 9;
        int mid = (nSteps - 1) / 2;
        float dy = box.size.y * 1.f / (nSteps - 1);

        float nX = std::ceil(box.size.x / dy);
        float dx = box.size.x / nX;

        auto markCol = style()->getColor(style::XTStyle::PLOT_MARKS);
        for (int yd = 0; yd < nSteps; yd++)
        {
            if (yd == mid)
                continue;
            float y = yd * dy;
            float x = 0;

            while (x <= box.size.x)
            {
                nvgBeginPath(vg);
                nvgFillColor(vg, markCol);
                nvgEllipse(vg, x, y, 0.5, 0.5);
                nvgFill(vg);
                x += dx;
            }
        }
        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, box.size.y * 0.5);
        nvgLineTo(vg, box.size.x, box.size.y * 0.5);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, box.size.y);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, markCol);
        nvgMoveTo(vg, 0, 0);
        nvgLineTo(vg, box.size.x, 0);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
    }

    void drawPlot(NVGcontext *vg)
    {
        if (!module)
        {
            // Draw the module name here for preview goodness
            nvgBeginPath(vg);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, 30);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5 + 2, osc_type_names[oscType], nullptr);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5 + 3, "VCO", nullptr);
        }
        else if (!oscPath.empty())
        {
            nvgSave(vg);
            nvgScissor(vg, 0, 0.5, box.size.x, box.size.y - 1);

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            auto gcp = col;
            gcp.a = 0.5;
            auto gcn = col;
            gcn.a = 0.0;

            bool first{true};

            nvgBeginPath(vg);
            for (const auto &[x, y] : oscPath)
            {
                auto uy = std::min(y * 1.0, box.size.y * 0.5);
                if (first)
                {
                    nvgMoveTo(vg, x, uy);
                }
                else
                {
                    nvgLineTo(vg, x, uy);
                }
                first = false;
            }
            nvgLineTo(vg, box.size.x, box.size.y * 0.5);
            nvgLineTo(vg, 0, box.size.y * 0.5);
            nvgFillPaint(vg,
                         nvgLinearGradient(vg, 0, box.size.y * 0.1, 0, box.size.y * 0.5, gcp, gcn));
            nvgFill(vg);

            nvgBeginPath(vg);
            first = true;
            for (const auto &[x, y] : oscPath)
            {
                auto uy = std::max(y * 1.0, box.size.y * 0.5);
                if (first)
                {
                    nvgMoveTo(vg, x, uy);
                }
                else
                {
                    nvgLineTo(vg, x, uy);
                }
                first = false;
            }
            nvgLineTo(vg, box.size.x, box.size.y * 0.5);
            nvgLineTo(vg, 0, box.size.y * 0.5);
            nvgFillPaint(vg,
                         nvgLinearGradient(vg, 0, box.size.y * 0.5, 0, box.size.y * 0.9, gcn, gcp));
            nvgFill(vg);

            nvgBeginPath(vg);
            first = true;
            for (const auto &[x, y] : oscPath)
            {
                if (first)
                {
                    nvgMoveTo(vg, x, y);
                }
                else
                {
                    nvgLineTo(vg, x, y);
                }
                first = false;
            }

            nvgStrokeColor(vg, col);
            nvgStrokeWidth(vg, 1.25);
            nvgStroke(vg);

            col.a = 0.1;
            nvgStrokeColor(vg, col);
            nvgStrokeWidth(vg, 3);
            nvgStroke(vg);
            nvgRestore(vg);
        }
    }
};

template <int oscType>
VCOWidget<oscType>::VCOWidget(VCOWidget<oscType>::M *module) : XTModuleWidget()
{
    setModule(module);
    typedef layout::LayoutEngine<VCOWidget<oscType>, M::PITCH_0> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);

    std::string panelLabel = std::string(M::name) + " VCO";
    for (auto &q : panelLabel)
        q = std::toupper(q);

    auto bg = new widgets::Background(box.size, panelLabel, "vco", "BlankVCO");
    addChild(bg);

    float plotStartX = rack::mm2px(layout::LayoutConstants::VCOplotCX_MM -
                                   layout::LayoutConstants::VCOplotW_MM * 0.5);
    float plotStartY = rack::mm2px(layout::LayoutConstants::VCOplotCY_MM -
                                   layout::LayoutConstants::VCOplotH_MM * 0.5);
    float plotW = rack::mm2px(layout::LayoutConstants::VCOplotW_MM);
    float plotH = rack::mm2px(layout::LayoutConstants::VCOplotH_MM -
                              layout::LayoutConstants::VCOplotControlsH_MM);
    float underPlotStartY = plotStartY + plotH;
    float underPlotH = rack::mm2px(layout::LayoutConstants::VCOplotControlsH_MM);

    auto t = plotStartY;
    auto h = plotH;

    if (VCOConfig<oscType>::requiresWavetables())
    {
        auto fivemm = rack::mm2px(5);
        auto halfmm = rack::mm2px(0.5);
        auto wts = WavetableSelector<oscType>::create(rack::Vec(plotStartX, plotStartY),
                                                      rack::Vec(plotW, fivemm - halfmm), module);
        addChild(wts);
        plotStartY += fivemm;
        plotH -= fivemm;
    }
    addChild(OSCPlotWidget<oscType>::create(rack::Vec(plotStartX, plotStartY),
                                            rack::Vec(plotW, plotH), module));

    float underX = plotStartX + 2;
    auto oct = widgets::LabeledPlotAreaControl::create(rack::Vec(underX, underPlotStartY),
                                                       rack::Vec(34, underPlotH), "OCT", module,
                                                       M::OCTAVE_SHIFT);
    oct->formatLabel = [](float f, const std::string &s) {
        std::string r = s;
        if (f > 0)
            r = "+" + r;
        return r;
    };
    addChild(oct);
    underX += 34 + 5;

    if constexpr (VCOConfig<oscType>::supportsUnison())
    {
        auto oct = widgets::LabeledPlotAreaControl::create(rack::Vec(underX, underPlotStartY),
                                                           rack::Vec(31, underPlotH), "UNI", module,
                                                           M::OSC_CTRL_PARAM_0 + 6);
        addChild(oct);
        underX += 32 + 2;
    }

    if (VCOConfig<oscType>::getMenuLightID() >= 0)
    {
        auto oct = widgets::PlotAreaSwitch::create(
            rack::Vec(underX, underPlotStartY), rack::Vec(18, underPlotH),
            VCOConfig<oscType>::getMenuLightString(), module,
            M::ARBITRARY_SWITCH_0 + VCOConfig<oscType>::getMenuLightID());
        addChild(oct);
        underX += 18 + 2;
    }

    if (VCOConfig<oscType>::rightMenuParamId() >= 0)
    {
        auto restSz = plotW - underX + plotStartX;
        auto plt = widgets::PlotAreaMenuItem::create(
            rack::Vec(underX, underPlotStartY), rack::Vec(restSz, underPlotH), module,
            M::OSC_CTRL_PARAM_0 + VCOConfig<oscType>::rightMenuParamId());
        plt->onShowMenu = [this, plt]() {
            if (!this->module)
                return;

            auto *vcm = static_cast<VCO<oscType> *>(this->module);

            auto pq = plt->getParamQuantity();
            if (!pq)
                return;

            auto &surgePar = vcm->oscstorage->p[VCOConfig<oscType>::rightMenuParamId()];
            if (!(surgePar.valtype == vt_int))
                return;

            auto men = rack::createMenu();
            men->addChild(rack::createMenuLabel(pq->getLabel()));

            int step{1};
            for (int i = surgePar.val_min.i; i <= surgePar.val_max.i; i += step)
            {
                char txt[256];
                auto fv = Parameter::intScaledToFloat(i, surgePar.val_max.i, surgePar.val_min.i);
                surgePar.get_display(txt, true, fv);
                men->addChild(rack::createMenuItem(txt, "", [pq, fv]() { pq->setValue(fv); }));
            }
        };
        plt->transformLabel = VCOConfig<oscType>::rightMenuTransformFunction();
        addChild(plt);
    }

    const auto &layout = VCOConfig<oscType>::getLayout();
    for (const auto &lay : layout)
    {
        engine_t::layoutItem(this, lay, panelLabel);
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::OSC_MOD_INPUT);

    engine_t::createInputOutputPorts(this, M::PITCH_CV, M::RETRIGGER, M::OUTPUT_L, M::OUTPUT_R);

    engine_t ::createLeftRightInputLabels(this, "V/OCT", VCOConfig<oscType>::retriggerLabel());

    resetStyleCouplingToModule();
}

} // namespace sst::surgext_rack::vco::ui

namespace vcoui = sst::surgext_rack::vco::ui;

rack::Model *modelVCOClassic =
    rack::createModel<vcoui::VCOWidget<ot_classic>::M, vcoui::VCOWidget<ot_classic>>(
        "SurgeXTOSCClassic");

rack::Model *modelVCOModern =
    rack::createModel<vcoui::VCOWidget<ot_modern>::M, vcoui::VCOWidget<ot_modern>>(
        "SurgeXTOSCModern");

rack::Model *modelVCOWavetable =
    rack::createModel<vcoui::VCOWidget<ot_wavetable>::M, vcoui::VCOWidget<ot_wavetable>>(
        "SurgeXTOSCWavetable");
rack::Model *modelVCOWindow =
    rack::createModel<vcoui::VCOWidget<ot_window>::M, vcoui::VCOWidget<ot_window>>(
        "SurgeXTOSCWindow");

rack::Model *modelVCOSine =
    rack::createModel<vcoui::VCOWidget<ot_sine>::M, vcoui::VCOWidget<ot_sine>>("SurgeXTOSCSine");
rack::Model *modelVCOFM2 =
    rack::createModel<vcoui::VCOWidget<ot_FM2>::M, vcoui::VCOWidget<ot_FM2>>("SurgeXTOSCFM2");
rack::Model *modelVCOFM3 =
    rack::createModel<vcoui::VCOWidget<ot_FM3>::M, vcoui::VCOWidget<ot_FM3>>("SurgeXTOSCFM3");

rack::Model *modelVCOSHNoise =
    rack::createModel<vcoui::VCOWidget<ot_shnoise>::M, vcoui::VCOWidget<ot_shnoise>>(
        "SurgeXTOSCSHNoise");

rack::Model *modelVCOString =
    rack::createModel<vcoui::VCOWidget<ot_string>::M, vcoui::VCOWidget<ot_string>>(
        "SurgeXTOSCString");
rack::Model *modelVCOAlias =
    rack::createModel<vcoui::VCOWidget<ot_alias>::M, vcoui::VCOWidget<ot_alias>>("SurgeXTOSCAlias");
rack::Model *modelVCOTwist =
    rack::createModel<vcoui::VCOWidget<ot_twist>::M, vcoui::VCOWidget<ot_twist>>("SurgeXTOSCTwist");
