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

#include "VCO.h"
#include "VCOConfig.h"
#include "XTWidgets.h"

#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "osdialog.h"

namespace sst::surgext_rack::vco::ui
{

static std::atomic<bool> downloadingContent{false};
static float contentProgress{0};

template <int oscType> struct WavetableMenuBuilder
{
    static void sendLoadFor(VCO<oscType> *module, int nt)
    {
        auto msg = typename vco::VCO<oscType>::WavetableMessage();
        msg.index = nt;
        module->wavetableQueue.push(msg);
    }

    static void sendLoadForPath(VCO<oscType> *module, const char *fn, int sz = -1)
    {
        auto msg = typename vco::VCO<oscType>::WavetableMessage();
        strncpy(msg.filename, fn, 256);
        msg.filename[255] = 0;
        msg.index = -1;
        msg.defaultSize = sz;
        module->wavetableQueue.push(msg);
    }

    static rack::ui::Menu *menuForCategory(rack::ui::Menu *menu, VCO<oscType> *module,
                                           int categoryId)
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
                                                    [module, p]() { sendLoadFor(module, p); }));
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
                    catName, "", [cidx, module](auto *x) { menuForCategory(x, module, cidx); }));
            }
        }

        return menu;
    }

    static void downloadExtraContent(VCO<oscType> *module)
    {
        if (downloadingContent)
            return;

        auto t = std::thread([module]() {
            downloadingContent = true;
            std::string archivePath = rack::asset::user("SurgeXTRack/SurgeXT_ExtraContent.tar.zst");
            std::string url = "https://github.com/surge-synthesizer/surge-rack/releases/download/"
                              "Content/SurgeXTRack_ExtraContent.tar.zst";
            std::string dirPath = rack::asset::user("SurgeXTRack/");
            rack::network::requestDownload(url, archivePath, &contentProgress);
            rack::system::unarchiveToDirectory(archivePath, dirPath);
            rack::system::remove(archivePath);

            downloadingContent = false;
            module->forceRefreshWT = true;
        });
        t.detach();
    }

    static void buildMenuOnto(rack::ui::Menu *menu, VCO<oscType> *module)
    {
        if (!module)
            return;
        menu->addChild(rack::createMenuLabel("WaveTables"));
        auto storage = module->storage.get();
        int idx{0};
        bool addSepIfMaking{false};
        for (auto c : storage->wtCategoryOrdering)
        {
            PatchCategory cat = storage->wt_category[c];

            if (idx == storage->firstThirdPartyWTCategory ||
                (idx == storage->firstUserWTCategory &&
                 storage->firstUserWTCategory != (int)storage->wt_category.size()))
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
                    cat.name, "", [c, module](auto *x) { return menuForCategory(x, module, c); }));
            }
        }
        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(rack::createMenuItem("Load Wavetable File", "", [module]() {
#ifdef USING_CARDINAL_NOT_RACK
            async_dialog_filebrowser(false, "wavetable.wav", nullptr, "Load Wavetable File",
                                     [=](char *openF) {
                                         if (openF)
                                         {
                                             DEFER({ std::free(openF); });
                                             sendLoadForPath(module, openF);
                                         }
                                     });
#else
#if MAC
            osdialog_filters *filters{nullptr};
#else
            auto filters = osdialog_filters_parse("Wavetables:wav,.WAV,.Wav,.wt,.WT,.Wt");
            DEFER({ osdialog_filters_free(filters); });
#endif
            char *openF = osdialog_file(OSDIALOG_OPEN, nullptr, nullptr, filters);
            if (openF)
            {
                DEFER({ std::free(openF); });
                sendLoadForPath(module, openF);
            }
#endif
        }));
        menu->addChild(rack::createMenuItem("Load WaveEdit Wavetable", "", [module]() {
#ifdef USING_CARDINAL_NOT_RACK
            async_dialog_filebrowser(false, "wavetable.wav", nullptr, "Load WaveEdit Wavetable",
                                     [=](char *openF) {
                                         if (openF)
                                         {
                                             DEFER({ std::free(openF); });
                                             sendLoadForPath(module, openF, 256);
                                         }
                                     });
#else
            auto filters = osdialog_filters_parse("Wavetables:wav,.WAV,.Wav");
            DEFER({ osdialog_filters_free(filters); });
            char *openF = osdialog_file(OSDIALOG_OPEN, nullptr, nullptr, filters);
            if (openF)
            {
                DEFER({ std::free(openF); });
                sendLoadForPath(module, openF, 256);
            }
#endif
        }));
        menu->addChild(rack::createSubmenuItem("Load Untagged Wav as", "", [module](auto *pm) {
            for (int i = 6; i < 13; ++i)
            {
                auto label = std::to_string(1 << i) + " Sample Frame WaveTable";
                pm->addChild(rack::createMenuItem(label, "", [module, i]() {
#ifdef USING_CARDINAL_NOT_RACK
                    async_dialog_filebrowser(false, "wavetable.wav", nullptr, "Load Untagged Wav",
                                             [=](char *openF) {
                                                 if (openF)
                                                 {
                                                     DEFER({ std::free(openF); });
                                                     sendLoadForPath(module, openF, 1 << i);
                                                 }
                                             });
#else
                    auto filters = osdialog_filters_parse("Wavetables:wav,.WAV,.Wav");
                    DEFER({ osdialog_filters_free(filters); });
                    char *openF = osdialog_file(OSDIALOG_OPEN, nullptr, nullptr, filters);
                    if (openF)
                    {
                        DEFER({ std::free(openF); });
                        sendLoadForPath(module, openF, 1 << i);
                    }
#endif
                }));
            }
        }));
        menu->addChild(new rack::MenuSeparator);
        menu->addChild(rack::createMenuItem(
            "Download Extra Wavetable Content", "", [module]() { downloadExtraContent(module); },
            downloadingContent));

        menu->addChild(rack::createMenuItem("Reveal User Wavetables Directory", "", [module]() {
            module->storage->createUserDirectory(); // fine if it exists
            rack::system::openDirectory((module->storage->userDataPath / "Wavetables").u8string());
        }));
        menu->addChild(rack::createMenuItem("Rescan Wavetables", "",
                                            [module]() { module->forceRefreshWT = true; }));
    }
};
template <int oscType> struct VCOWidget : public widgets::XTModuleWidget
{
    typedef VCO<oscType> M;
    VCOWidget(M *module);

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, 8> overlays;
    std::array<widgets::ModulatableKnob *, 8> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void childrenChanged() { resetStyleCouplingToModule(); }
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

    virtual void downsampleMenu(rack::Menu *p, M *m)
    {
        if (!m)
            return;

        auto hbM = m->halfbandM;
        auto hbS = m->halfbandSteep;

        for (auto steep : {true, false})
        {
            for (auto M : {6, 5, 4, 3, 2, 1})
            {
                auto name =
                    std::string("M = ") + std::to_string(M) + ", " + (steep ? "steep" : "shallow");
                p->addChild(
                    rack::createMenuItem(name, CHECKMARK(hbM == M && hbS == steep), [m, M, steep] {
                        m->setHalfbandCharacteristics(M, steep);
                    }));
            }
            if (steep)
                p->addChild(new rack::MenuSeparator);
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

            if (VCOConfig<oscType>::requiresWavetables())
            {
                menu->addChild(new rack::MenuSeparator);
                menu->addChild(rack::createSubmenuItem("Wavetables", "", [m](auto *x) {
                    WavetableMenuBuilder<oscType>::buildMenuOnto(x, m);
                }));
            }
            menu->addChild(new rack::MenuSeparator);
            menu->addChild(rack::createSubmenuItem("Character", "",
                                                   [this, m](auto *x) { characterMenu(x, m); }));

            auto driftSlider = new rack::ui::Slider;
            driftSlider->quantity = module->paramQuantities[M::DRIFT];
            driftSlider->box.size.x = 125;
            menu->addChild(driftSlider);

            auto attenSlider = new rack::ui::Slider;
            attenSlider->quantity = module->paramQuantities[M::FIXED_ATTENUATION];
            attenSlider->box.size.x = 125;
            menu->addChild(attenSlider);

            menu->addChild(rack::createSubmenuItem("Halfband Filter", "",
                                                   [this, m](auto *x) { downsampleMenu(x, m); }));

            menu->addChild(rack::createMenuItem("Apply DC Blocker", CHECKMARK(m->doDCBlock),
                                                [m]() { m->doDCBlock = !m->doDCBlock; }));
            VCOConfig<oscType>::addMenuItems(m, menu);
        }
    }

    void step() override
    {
        if constexpr (VCOConfig<oscType>::requiresWavetables())
        {
            auto vm = static_cast<VCO<oscType> *>(module);
            if (vm && vm->forceRefreshWT)
            {
                vm->forceRefreshWT = false;
                vm->storage->refresh_wtlist();
            }
        }
        widgets::XTModuleWidget::step();
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
        WavetableMenuBuilder<oscType>::sendLoadFor(module, nt);
    }

    void onShowMenu() override
    {
        if (!module)
            return;

        auto menu = rack::createMenu();
        WavetableMenuBuilder<oscType>::buildMenuOnto(menu, module);
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

        for (int i = 0; i < n_osc_params; ++i)
            priorDeform[i] = 0;
    }

    std::set<rack::Widget *> deleteOnNextStep;
    bool lastDownloadContent{false};
    void step() override
    {
        for (auto d : deleteOnNextStep)
        {
            getParent()->removeChild(d);
            delete d;
        }
        if (deleteOnNextStep.size())
        {
            auto p = getParent();
            auto dp = dynamic_cast<VCOWidget<oscType> *>(p);
            if (dp)
                dp->childrenChanged();

            recalcPath();
            bdwPlot->dirty = true;
            bdw->dirty = true;
        }
        deleteOnNextStep.clear();

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

            if (downloadingContent)
            {
                bdw->dirty = true;
                bdwPlot->dirty = true;
            }

            if (downloadingContent != lastDownloadContent)
            {
                bdw->dirty = true;
                bdwPlot->dirty = true;
            }
            lastDownloadContent = downloadingContent;
        }

        if constexpr (VCOConfig<oscType>::supportsCustomEditor())
        {
            auto sce = VCOConfig<oscType>::isCustomEditorActivatable(module);
            if (sce != showCustomEditorOpen)
            {
                bdw->dirty = true;
                bdwPlot->dirty = true;
            }
            showCustomEditorOpen = sce;
        }
        else
        {
            showCustomEditorOpen = false;
        }

        rack::widget::Widget::step();
    }

    virtual void onStyleChanged() override
    {
        bdw->dirty = true;
        bdwPlot->dirty = true;

        auto vcm = dynamic_cast<VCO<oscType> *>(module);
        if (vcm)
            vcm->animateDisplayFromMod = style::XTStyle::getShowModulationAnimationOnDisplay();
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
    int priorDeform[n_osc_params]{};
    int charF{-1};
    bool isOneShot{false};
    bool showCustomEditorOpen{false};

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
                auto *par = &(oscdata->p[i]);
                auto idis = par->param_id_in_scene;
                pdata tval;
                tval.i = par->val.i;
                if (par->valtype == vt_float && module->animateDisplayFromMod)
                {
                    tval.f +=
                        module->modAssist.modvalues[i + 1][0] * (par->val_max.f - par->val_min.f);
                }
                dval = dval || (tp[par->param_id_in_scene].i != tval.i);
                lSumDeact += par->deactivated * (1 << i);
                lSumAbs += par->absolute * (1 << i);

                dval = dval || (priorDeform[i] != par->deform_type);
                priorDeform[i] = par->deform_type;
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

            if (VCOConfig<oscType>::requiresWavetables())
            {
                auto wos = module->isWTOneShot();
                if (wos != isOneShot)
                {
                    dval = true;
                }
                isOneShot = wos;
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
            auto *par = &(oscdata->p[i]);
            auto idis = par->param_id_in_scene;
            tp[idis].i = par->val.i;
            if (par->valtype == vt_float && module->animateDisplayFromMod)
            {
                tp[idis].f +=
                    module->modAssist.modvalues[i + 1][0] * (par->val_max.f - par->val_min.f);
            }
        }

        auto res = spawn_osc(oscdata->type.val.i, storage, oscdata, tp, oscbuffer);
        res->init_ctrltypes();
        return res;
    }

    std::vector<std::pair<float, float>> oscPath;
    void recalcPath()
    {
        oscPath.clear();

        if (VCOConfig<oscType>::requiresWavetables())
        {
            if (module->wavetableCount == 0)
                return;
        }

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
            {
                auto par = &(oscdata->p[0]);
                pos = par->val.f;
                if (module->animateDisplayFromMod)
                {
                    pos +=
                        module->modAssist.modvalues[0 + 1][0] * (par->val_max.f - par->val_min.f);
                }
                off = par->extend_range;
            }
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
    const float xsedit{rack::mm2px(12)};
    const float ysedit{ys3d};

    void drawPlotBackground(NVGcontext *vg)
    {
        if (module && VCOConfig<oscType>::requiresWavetables() && module->draw3DWavetable &&
            module->wavetableCount > 0)
        {
            draw3DBackground(vg);
        }
        else
        {
            draw2DBackground(vg);
        }

        if (showCustomEditorOpen)
        {
            nvgBeginPath(vg);
            nvgRect(vg, box.size.x - xsedit, 0, xsedit, ysedit);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_BG));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_VALUE_FG));
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(vg, box.size.x - xsedit * 0.5, ys3d * 0.5, "EDIT", nullptr);
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

            if (isOneShot)
            {
                nvgBeginPath(vg);
                nvgFontFaceId(vg, style()->fontIdBold(vg));
                nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CONTROL_TEXT));
                nvgFontSize(vg, layout::LayoutConstants::labelSize_pt * 96 / 72);
                nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
                nvgText(vg, box.size.x - rack::mm2px(0.5), ys3d * 0.5, "OneShot", nullptr);
            }
        }
    }

    bool editPressed{false};
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

        if (showCustomEditorOpen && e.pos.x > box.size.x - xsedit && e.pos.y < ys3d)
        {
            // don't capture the release on dismiss to re-arm the edit basically
            if (e.action == GLFW_PRESS)
            {
                editPressed = true;
            }
            if (e.action == GLFW_RELEASE && editPressed)
            {
                auto ce = VCOConfig<oscType>::createCustomEditorAt(box.pos, box.size, module,
                                                                   [this](auto *w) {
                                                                       // getParent()->removeChild(w);
                                                                       deleteOnNextStep.insert(w);
                                                                       show();
                                                                   });
                if (ce)
                {
                    hide();
                    getParent()->addChild(ce);
                    auto p = getParent();
                    auto dp = dynamic_cast<VCOWidget<oscType> *>(p);
                    if (dp)
                        dp->childrenChanged();
                }
                editPressed = false;
                e.consume(this);
                return;
            }
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
        else if (downloadingContent)
        {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, 18);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5 + 2, "Downloading", nullptr);

            auto r = fmt::format("{} pct", (int)(contentProgress * 100));
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5 + 3, r.c_str(), nullptr);
        }
        else if (!oscPath.empty())
        {
            bool is3D{false};
            if (module && VCOConfig<oscType>::requiresWavetables() && module->draw3DWavetable &&
                module->wavetableCount > 0)
            {
                is3D = true;
            }

            nvgSave(vg);
            nvgScissor(vg, 0, 0.5, box.size.x, box.size.y - 1);

            auto col = style()->getColor(style::XTStyle::PLOT_CURVE);
            auto gcp = col;
            gcp.a = 0.5;
            auto gcn = col;
            gcn.a = 0.0;

            bool first{true};

            if (!is3D)
            {
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
                nvgFillPaint(
                    vg, nvgLinearGradient(vg, 0, box.size.y * 0.1, 0, box.size.y * 0.5, gcp, gcn));
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
                nvgFillPaint(
                    vg, nvgLinearGradient(vg, 0, box.size.y * 0.5, 0, box.size.y * 0.9, gcn, gcp));
                nvgFill(vg);
            }

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
        oct->minScale = 1;
        oct->maxScale = 16;
        oct->maxVal = 9;
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

            auto &surgePar = vcm->oscstorage_display->p[VCOConfig<oscType>::rightMenuParamId()];
            if (!(surgePar.valtype == vt_int))
                return;

            auto men = rack::createMenu();
            men->addChild(rack::createMenuLabel(pq->getLabel()));

            int step{1};
            for (int i = surgePar.val_min.i; i <= surgePar.val_max.i; i += step)
            {
                if (!VCOConfig<oscType>::showRightMenuChoice(i))
                    continue;
                char txt[256];
                auto fv = Parameter::intScaledToFloat(i, surgePar.val_max.i, surgePar.val_min.i);
                surgePar.get_display(txt, true, fv);
                std::string nm = surgePar.get_name();
                men->addChild(
                    rack::createMenuItem(txt, CHECKMARK(i == surgePar.val.i), [nm, pq, fv]() {
                        auto *h = new rack::history::ParamChange;
                        h->name = std::string("change ") + nm;
                        h->moduleId = pq->module->id;
                        h->paramId = pq->paramId;
                        h->oldValue = pq->getValue();
                        h->newValue = fv;
                        APP->history->push(h);

                        pq->setValue(fv);
                    }));
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

    engine_t ::createLeftRightInputLabels(this, "V/OCT", "");
    engine_t::createInputOutputPorts(this, M::PITCH_CV, M::RETRIGGER, M::OUTPUT_L, M::OUTPUT_R);

    // Special input 2 label is dynamic for WT. This is a wee bit of a generatlization
    // breakage but not too bad.
    {
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
        auto lab = engine_t::makeLabelAt(bl, 1, "", style::XTStyle::TEXT_LABEL);
        lab->hasDynamicLabel = true;
        lab->module = module;
        lab->dynamicLabel = [](auto m) -> std::string {
            if (VCOConfig<oscType>::requiresWavetables() && m)
            {
                auto vm = dynamic_cast<VCO<oscType> *>(m);
                if (vm && vm->isWTOneShot())
                    return "TRIG";
            }
            return VCOConfig<oscType>::retriggerLabel();
            ;
        };
        addChild(lab);
    }
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
