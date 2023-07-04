/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
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

#include "VCF.h"
#include "SurgeXT.h"
#include "XTModuleWidget.h"
#include "XTWidgets.h"
#include "dsp/DSPExternalAdapterUtils.h"
#include "LayoutEngine.h"

#ifndef __EMSCRIPTEN__
#define SURGE_VCF_WITH_THREADED_ANALYSER 1
#endif

namespace sst::surgext_rack::vcf::ui
{
struct VCFWidget : widgets::XTModuleWidget
{
    typedef vcf::VCF M;
    VCFWidget(M *module);

    void moduleBackground(NVGcontext *vg) {}

    std::array<std::array<rack::Widget *, M::n_mod_inputs>, 5> overlays;
    std::array<widgets::ModulatableKnob *, VCF::n_vcf_params> underKnobs;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;

    void selectModulator(int mod) override
    {
        if (toggles[mod])
            toggles[mod]->onToggle(!toggles[mod]->pressedState);
    }
};

struct FilterChangeAction : rack::history::ModuleAction
{
    int priorType{0}, priorSubType{0}, newType{0}, newSubType{0};
    explicit FilterChangeAction(VCF *m)
    {
        moduleId = m->id;

        priorType = (int)std::round(m->paramQuantities[VCF::VCF_TYPE]->getValue());
        priorSubType = (int)std::round(m->paramQuantities[VCF::VCF_SUBTYPE]->getValue());
        newType = priorType;
        newSubType = priorSubType;
    }

    void undo() override
    {
        auto *module = APP->engine->getModule(moduleId);
        if (!module)
            return;
        module->paramQuantities[VCF::VCF_TYPE]->setValue(priorType);
        module->paramQuantities[VCF::VCF_SUBTYPE]->setValue(priorSubType);
    }

    void redo() override
    {
        auto *module = APP->engine->getModule(moduleId);
        if (!module)
            return;
        module->paramQuantities[VCF::VCF_TYPE]->setValue(newType);
        module->paramQuantities[VCF::VCF_SUBTYPE]->setValue(newSubType);
    }
};

struct VCFSelector : widgets::ParamJogSelector
{
    FilterSelectorMapper fsm;
    std::vector<int> fsmOrdering;

    VCFSelector() { fsmOrdering = fsm.totalIndexOrdering(); }
    static VCFSelector *create(const rack::Vec &pos, const rack::Vec &size, VCF *module,
                               int paramId)
    {
        auto res = new VCFSelector();
        res->box.pos = pos;
        res->box.size = size;
        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();
        res->setup();

        return res;
    }

    void onPresetJog(int dir) override
    {
        if (!getParamQuantity())
            return;

        int type = (int)std::round(getParamQuantity()->getValue());
        auto di = fsm.remapStreamedIndexToDisplayIndex(type);
        di += dir;
        if (di >= (int)fsmOrdering.size())
            di = 0;
        if (di < 0)
            di = fsmOrdering.size() - 1;
        setType(fsmOrdering[di]);
    }

    int filterType()
    {
        if (!module)
            return 0;
        int type = (int)std::round(module->params[VCF::VCF_TYPE].getValue());
        return type;
    }

    rack::ui::Menu *menuForGroup(rack::ui::Menu *menu, const std::string &group)
    {
        auto t = filterType();
        for (const auto &[id, gn] : fsm.mapping)
        {
            if (gn == group)
            {
                menu->addChild(rack::createMenuItem(sst::filters::filter_type_names[id],
                                                    CHECKMARK(id == t),
                                                    [this, cid = id]() { setType(cid); }));
            }
        }
        return menu;
    }

    void onShowMenu() override
    {
        if (!module)
            return;
        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel("Filter Types"));

        std::string currentGroup{"-not-a-filter-group-"};

        auto t = filterType();
        for (const auto &[id, gn] : fsm.mapping)
        {
            if (gn == "")
            {
                menu->addChild(rack::createMenuItem(sst::filters::filter_type_names[id],
                                                    CHECKMARK(id == t),
                                                    [this, cid = id]() { setType(cid); }));
            }
            else if (gn != currentGroup)
            {
                bool check{false};
                // This is a little silly to iterate again to find this but this list is small and
                // the group count is low and it's only at change so it's not problematically
                // quadratic
                for (const auto &[idr, gnr] : fsm.mapping)
                {
                    if (gn == gnr && idr == t)
                    {
                        check = true;
                    }
                }
                menu->addChild(rack::createSubmenuItem(
                    gn, CHECKMARK(check), [this, cgn = gn](auto *x) { menuForGroup(x, cgn); }));
                currentGroup = gn;
            }
        }
    }
    bool forceDirty{false};
    void setType(int id)
    {
        forceDirty = true;
        if (!module)
            return;
        if (!getParamQuantity())
            return;
        auto vcfm = static_cast<VCF *>(module);
        auto h = new FilterChangeAction(vcfm);
        h->newType = id;
        h->newSubType = vcfm->defaultSubtype[id];
        APP->history->push(h);

        getParamQuantity()->setValue(id);
        // FixMe: have a default type per type
        module->params[VCF::VCF_SUBTYPE].setValue(vcfm->defaultSubtype[id]);
    }

    int lastType{-1};
    bool isDirty() override
    {
        if (forceDirty)
        {
            forceDirty = false;
            return true;
        }
        if (getParamQuantity())
        {
            int type = (int)std::round(getParamQuantity()->getValue());
            if (type != lastType)
            {
                lastType = type;
                return true;
            }
        }
        return false;
    }
    std::string getPresetName() override
    {
        if (!getParamQuantity())
            return "FILTER";
        int type = (int)std::round(getParamQuantity()->getValue());
        return sst::filters::filter_type_names[type];
    }
};

struct VCFSubtypeSelector : widgets::ParamJogSelector
{
    FilterSelectorMapper fsm;

    static VCFSubtypeSelector *create(const rack::Vec &pos, const rack::Vec &size, VCF *module,
                                      int paramId)
    {
        auto res = new VCFSubtypeSelector();
        res->box.pos = pos;
        res->box.size = size;
        res->module = module;
        res->paramId = paramId;
        res->initParamQuantity();
        res->setup();

        return res;
    }

    int filterType()
    {
        if (!module)
            return 0;
        int type = (int)std::round(module->params[VCF::VCF_TYPE].getValue());
        return type;
    }

    int filterSubType()
    {
        if (!module)
            return 0;
        int type = (int)std::round(module->params[VCF::VCF_SUBTYPE].getValue());
        return type;
    }
    void onPresetJog(int dir) override
    {
        if (!module)
            return;
        auto type = filterType();
        int i = std::clamp((int)std::round(getParamQuantity()->getValue()), 0,
                           sst::filters::fut_subcount[type]);
        if (sst::filters::fut_subcount[type] == 0)
            return;

        i += dir;
        if (i < 0)
            i = sst::filters::fut_subcount[type] - 1;
        if (i >= sst::filters::fut_subcount[type])
            i = 0;

        setSubType(i);
    }

    bool hasPresets() override
    {
        if (!module)
            return true;
        auto type = filterType();
        return (sst::filters::fut_subcount[type] != 0);
    }
    void onShowMenu() override
    {
        if (!module)
            return;

        int type = filterType();
        int subt = filterSubType();
        int ct = sst::filters::fut_subcount[type];
        if (ct == 0)
            return;

        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel("Filter SubTypes"));
        for (int i = 0; i < ct; ++i)
        {
            menu->addChild(
                rack::createMenuItem(VCF::subtypeLabel(type, i), CHECKMARK(i == subt), [this, i]() {
                    setSubType(i);
                    forceDirty = true;
                }));
        }
    }
    bool forceDirty{false};
    void setSubType(int id)
    {
        forceDirty = true;
        if (!getParamQuantity())
            return;

        auto vcfm = static_cast<VCF *>(module);
        auto h = new FilterChangeAction(vcfm);
        h->newSubType = id;
        APP->history->push(h);

        getParamQuantity()->setValue(id);
    }

    int lastType{-1}, lastSubType{-1};
    bool isDirty() override
    {
        if (forceDirty)
        {
            forceDirty = false;
            return true;
        }
        if (!module)
            return false;

        int type = filterType();
        if (type != lastType)
        {
            lastType = type;
            return true;
        }

        auto st = (int)std::round(getParamQuantity()->getValue());
        if (lastSubType != st)
        {
            lastSubType = st;
            return true;
        }
        return false;
    }
    std::string getPresetName() override
    {
        if (!module)
            return "None";

        int type = filterType();
        int i = std::clamp((int)std::round(getParamQuantity()->getValue()), 0,
                           sst::filters::fut_subcount[type]);

        return VCF::subtypeLabel(type, i);
    }
};

struct FilterPlotParameters
{
    float sampleRate = 96000.0f;
    float startFreqHz = 20.0f;
    float endFreqHz = 20000.0f;
    float inputAmplitude = 1.0f / 1.414213562373095; // root 2
    float freqSmoothOctaves = 1.0f / 12.0f;
};

#if SURGE_VCF_WITH_THREADED_ANALYSER
struct FilterAnalzer
{
    FilterAnalzer() { analysisThread = std::make_unique<std::thread>(callRunThread, this); }
    ~FilterAnalzer()
    {
        {
            auto lock = std::unique_lock<std::mutex>(dataLock);
            continueWaiting = false;
        }
        cv.notify_one();
        analysisThread->join();
    }

    static void callRunThread(FilterAnalzer *that) { that->runThread(); }
    void runThread()
    {
        uint64_t lastIB = 0;
        while (continueWaiting)
        {
            if (lastIB == inboundUpdates)
            {
                auto lock = std::unique_lock<std::mutex>(dataLock);
                cv.wait(lock);
            }

            if (lastIB != inboundUpdates)
            {
                int cty, csu;
                float ccu, cre, cgn;
                {
                    auto lock = std::unique_lock<std::mutex>(dataLock);
                    cty = type;
                    csu = subtype;
                    ccu = cutoff;
                    cre = resonance;
                    cgn = gain;
                    lastIB = inboundUpdates;
                }

                auto data = surge::calculateFilterResponseCurve(
                    (sst::filters::FilterType)cty, (sst::filters::FilterSubType)csu, ccu, cre, cgn);

                {
                    auto lock = std::unique_lock<std::mutex>(dataLock);
                    outboundUpdates++;
                    dataCopy = data;
                }
            }
        }
    }

    void request(int t, int s, float c, float r, float g)
    {
        {
            auto lock = std::unique_lock<std::mutex>(dataLock);

            type = t;
            subtype = s;
            cutoff = c;
            resonance = r;
            gain = powf(2.f, g / 18.f);
            inboundUpdates++;
        }
        cv.notify_one();
    }

    std::pair<std::vector<float>, std::vector<float>> dataCopy;
    std::atomic<uint64_t> inboundUpdates{1}, outboundUpdates{1};
    int type{0}, subtype{0};
    float cutoff{60}, resonance{0}, gain{1.f};
    std::mutex dataLock;
    std::condition_variable cv;
    std::unique_ptr<std::thread> analysisThread;
    bool hasWork{false}, continueWaiting{true};
};
#endif

struct FilterPlotWidget : rack::widget::TransparentWidget, style::StyleParticipant
{
#if SURGE_VCF_WITH_THREADED_ANALYSER
    std::unique_ptr<FilterAnalzer> analyzer;
#endif
    VCF *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};
    widgets::BufferedDrawFunctionWidget *bdwPlot{nullptr};

    FilterPlotWidget() {}

    static FilterPlotWidget *create(const rack::Vec &pos, const rack::Vec &size, VCF *module)
    {
        auto res = new FilterPlotWidget();
        res->box.pos = pos;
        res->box.size = size;
        res->module = module;
        res->setup();

        return res;
    }

    float fr, re;

    void setup()
    {
#if SURGE_VCF_WITH_THREADED_ANALYSER
        if (module)
            analyzer = std::make_unique<FilterAnalzer>();
#endif
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { this->drawUnder(vg); });
        bdwPlot = new widgets::BufferedDrawFunctionWidgetOnLayer(
            rack::Vec(0, 0), box.size, [this](auto vg) { this->drawPlot(vg); });
        addChild(bdw);
        addChild(bdwPlot);
    }

    uint64_t lastOutbound{1};
    float lastFreq{-1}, lastReso{-1}, lastTy{-1}, lastSub{-1}, lastGn{-1};

    std::pair<std::vector<float>, std::vector<float>> responseCurve;
#if SURGE_VCF_WITH_THREADED_ANALYSER
    void step() override
    {
        if (!module)
            return;
        if (!analyzer)
            return;

        if (analyzer->outboundUpdates != lastOutbound)
        {
            {
                std::lock_guard<std::mutex> g(analyzer->dataLock);
                responseCurve = analyzer->dataCopy;
                lastOutbound = analyzer->outboundUpdates;
            }
            bdwPlot->dirty = true;
        }

        // This is how we do the 'unanimated' version
        // auto fr = module->params[VCF::FREQUENCY].getValue();
        // auto re = module->params[VCF::RESONANCE].getValue();
        float fr{0.f}, re{0.f}, gn{0.f};

        if (style::XTStyle::getShowModulationAnimationOnDisplay())
        {
            fr = module->modulationAssistant.values[VCF::FREQUENCY][0];
            re = module->modulationAssistant.values[VCF::RESONANCE][0];
            gn = module->modulationAssistant.values[VCF::IN_GAIN][0];
        }
        else
        {
            fr = module->modulationAssistant.basevalues[VCF::FREQUENCY];
            re = module->modulationAssistant.basevalues[VCF::RESONANCE];
            gn = module->modulationAssistant.basevalues[VCF::IN_GAIN];
        }

        auto ty = (int)std::round(module->params[VCF::VCF_TYPE].getValue());
        auto sty = (int)std::round(module->params[VCF::VCF_SUBTYPE].getValue());

        if (fr != lastFreq || re != lastReso || ty != lastTy || sty != lastSub || gn != lastGn)
        {
            lastFreq = fr;
            lastReso = re;
            lastSub = sty;
            lastTy = ty;
            lastGn = gn;
            // Remeber our filters all focus on midi note 69
            analyzer->request(ty, sty, fr * 12 - 9, re, gn);
        }
    }
#endif
    static constexpr float lowFreq = 10.f;
    static constexpr float highFreq = 18000.f;
    static constexpr float dbMin = -42.f;
    static constexpr float dbMax = 12.f;
    static constexpr float dbRange = dbMax - dbMin;

    float freqToX(float freq, int width)
    {
        auto xNorm = std::log(freq / lowFreq) / std::log(highFreq / lowFreq);
        return xNorm * (float)width;
    };

    float dbToY(float db, int height) { return (float)height * (dbMax - db) / dbRange; };

    void drawUnder(NVGcontext *vg)
    {

        for (const auto freq : {100, 1000, 10000})
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgMoveTo(vg, freqToX(freq, box.size.x), 0);
            nvgLineTo(vg, freqToX(freq, box.size.x), box.size.y);
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }

        for (const auto db : {-36, -24, -12, 0, 12})
        {
            nvgBeginPath(vg);
            nvgStrokeColor(vg, style()->getColor(style::XTStyle::PLOT_MARKS));
            nvgMoveTo(vg, 0, dbToY(db, box.size.y));
            nvgLineTo(vg, box.size.x, dbToY(db, box.size.y));
            nvgStrokeWidth(vg, 0.75);
            nvgStroke(vg);
        }
    }

    void drawPlot(NVGcontext *vg)
    {
        if (!module)
        {
            // Draw the module name here for preview goodness
            nvgBeginPath(vg);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, style()->fontIdBold(vg));
            nvgFontSize(vg, 30);
            nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));
            nvgText(vg, box.size.x * 0.5, box.size.y * 0.5, "Filter", nullptr);
            return;
        }
        const auto &freq = responseCurve.first;
        const auto &resp = responseCurve.second;
        auto sz = std::min(freq.size(), resp.size());
        nvgSave(vg);
        nvgScissor(vg, 0, 0.5, box.size.x, box.size.y - 1);

        auto makePath = [&]() -> std::pair<float, float> {
            uint64_t curr = 0;
            nvgBeginPath(vg);
            bool first{true};
            float firstf = 0, lastf = 0;
            while (curr < sz)
            {
                auto f = freq[curr];
                auto r = resp[curr];

                if (f >= lowFreq && f <= highFreq)
                {
                    float fscale = freqToX(f, box.size.x);
                    float yscale = dbToY(r, box.size.y);

                    if (first)
                    {
                        nvgMoveTo(vg, fscale, yscale);
                        firstf = fscale;
                    }
                    else
                    {
                        lastf = fscale;
                        nvgLineTo(vg, fscale, yscale);
                    }
                    first = false;
                }

                ++curr;
            }
            return {firstf, lastf};
        };

        auto col = style()->getColor(style::XTStyle::PLOT_CURVE);

        auto ff = makePath();
        nvgLineTo(vg, ff.second, box.size.y);
        nvgLineTo(vg, ff.first, box.size.y);
        auto gsp = dbToY(6, box.size.y);
        auto gsn = box.size.y;
        auto gcp = col;
        gcp.a = 0.5;
        auto gcn = col;
        gcn.a = 0.0;
        auto gr = nvgLinearGradient(vg, 0, gsp, 0, gsn, gcp, gcn);
        // nvgFillColor(vg, nvgRGBA(255, 0, 0, 80));
        nvgFillPaint(vg, gr);
        nvgFill(vg);

        makePath();
        nvgStrokeColor(vg, col);
        nvgStrokeWidth(vg, 1.25);
        nvgStroke(vg);

        col.a = 0.1;
        nvgStrokeColor(vg, col);
        nvgStrokeWidth(vg, 3);
        nvgStroke(vg);
        nvgRestore(vg);
    }

    void onStyleChanged() override { bdw->dirty = true; }
};

VCFWidget::VCFWidget(VCFWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    typedef layout::LayoutEngine<VCFWidget, M::FREQUENCY> engine_t;
    engine_t::initializeModulationToBlank(this);

    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews,
                         rack::app::RACK_GRID_HEIGHT);
    auto bg = new widgets::Background(box.size, "FILTER", "vco", "BlankVCO");
    addChild(bg);

    int idx = 0;

    float plotStartX = rack::mm2px(layout::LayoutConstants::VCOplotCX_MM -
                                   layout::LayoutConstants::VCOplotW_MM * 0.5);
    float plotStartY = rack::mm2px(layout::LayoutConstants::VCOplotCY_MM -
                                   layout::LayoutConstants::VCOplotH_MM * 0.5);
    float plotW = rack::mm2px(layout::LayoutConstants::VCOplotW_MM);
    float plotH = rack::mm2px(layout::LayoutConstants::VCOplotH_MM -
                              layout::LayoutConstants::VCOplotControlsH_MM);
    float underPlotStartY = plotStartY + plotH;
    float underPlotH = rack::mm2px(layout::LayoutConstants::VCOplotControlsH_MM);

    auto fivemm = rack::mm2px(5);
    auto halfmm = rack::mm2px(0.5);
    auto wts = VCFSelector::create(rack::Vec(plotStartX, plotStartY),
                                   rack::Vec(plotW, fivemm - halfmm), module, VCF::VCF_TYPE);
    addChild(wts);
    plotStartY += fivemm;
    plotH -= fivemm;

    auto fpw = FilterPlotWidget::create(rack::Vec(plotStartX, plotStartY), rack::Vec(plotW, plotH),
                                        module);
    addChild(fpw);

    auto subType =
        VCFSubtypeSelector::create(rack::Vec(plotStartX, underPlotStartY),
                                   rack::Vec(plotW, fivemm - halfmm), module, M::VCF_SUBTYPE);
    addChild(subType);

    typedef layout::LayoutItem lay_t;
    for (const auto &lay : {lay_t::createVCFWSBigKnob(M::FREQUENCY, "FREQUENCY"),
                            lay_t::createVCOKnob(M::RESONANCE, "RES", 0, 2),
                            lay_t::createVCOKnob(M::MIX, "MIX", 0, 3),
                            lay_t::createVCOKnob(M::IN_GAIN, "DRIVE", 1, 2),
                            lay_t::createVCOKnob(M::OUT_GAIN, "GAIN", 1, 3)})
    {
        engine_t::layoutItem(this, lay, "VCF");
    }

    engine_t::addModulationSection(this, M::n_mod_inputs, M::VCF_MOD_INPUT);

    engine_t::createLeftRightInputLabels(this);
    engine_t::createInputOutputPorts(this, M::INPUT_L, M::INPUT_R, M::OUTPUT_L, M::OUTPUT_R);

    resetStyleCouplingToModule();
}
} // namespace sst::surgext_rack::vcf::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeVCF = rack::createModel<sst::surgext_rack::vcf::ui::VCFWidget::M,
                                               sst::surgext_rack::vcf::ui::VCFWidget>("SurgeXTVCF");
