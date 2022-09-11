//
// Created by Paul Walker on 8/29/22.
//

#include "VCF.hpp"
#include "SurgeXT.hpp"
#include "XTModuleWidget.hpp"
#include "XTWidgets.h"

namespace sst::surgext_rack::vcf::ui
{
struct VCFWidget : widgets::XTModuleWidget, widgets::VCOVCFConstants
{
    typedef vcf::VCF M;
    VCFWidget(M *module);

    void moduleBackground(NVGcontext *vg) {}

    std::array<std::array<widgets::ModRingKnob *, M::n_mod_inputs>, 5> overlays;
    std::array<widgets::ModToggleButton *, M::n_mod_inputs> toggles;
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
        if (di >= fsmOrdering.size())
            di = 0;
        if (di < 0)
            di = fsmOrdering.size() - 1;
        setType(fsmOrdering[di]);
    }

    rack::ui::Menu *menuForGroup(rack::ui::Menu *menu, const std::string &group)
    {
        for (const auto &[id, gn] : fsm.mapping)
        {
            if (gn == group)
            {
                menu->addChild(rack::createMenuItem(sst::filters::filter_type_names[id], "",
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

        for (const auto &[id, gn] : fsm.mapping)
        {
            if (gn == "")
            {
                menu->addChild(rack::createMenuItem(sst::filters::filter_type_names[id], "",
                                                    [this, cid = id]() { setType(cid); }));
            }
            else if (gn != currentGroup)
            {
                menu->addChild(rack::createSubmenuItem(
                    gn, "", [this, cgn = gn](auto *x) { menuForGroup(x, cgn); }));
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
        getParamQuantity()->setValue(id);
        // FixMe: have a default type per type
        auto vcfm = static_cast<VCF *>(module);
        module->params[VCF::VCF_SUBTYPE].setValue(vcfm->defaultSubtype[id]);
    }

    bool isDirty() override
    {
        if (forceDirty)
        {
            forceDirty = false;
            return true;
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
        auto vcfm = static_cast<VCF *>(module);
        int type = (int)std::round(module->params[VCF::VCF_TYPE].getValue());
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
            i = sst::filters::fut_subcount[type];
        if (i >= sst::filters::fut_subcount[type])
            i = 0;

        getParamQuantity()->setValue(i);
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
        int ct = sst::filters::fut_subcount[type];
        if (ct == 0)
            return;

        auto menu = rack::createMenu();
        menu->addChild(rack::createMenuLabel("Filter SubTypes"));
        for (int i = 0; i < ct; ++i)
        {
            menu->addChild(rack::createMenuItem(VCF::subtypeLabel(type, i), "", [this, i]() {
                this->getParamQuantity()->setValue(i);
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

#if 0
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
        auto fp = sst::filters::FilterPlotter(15);
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

                auto par = sst::filters::FilterPlotParameters();
                par.inputAmplitude *= cgn;
                auto data = fp.plotFilterMagnitudeResponse(
                    (sst::filters::FilterType)cty, (sst::filters::FilterSubType)csu, ccu, cre, par);

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
    // std::unique_ptr<FilterAnalzer> analyzer;
    VCF *module{nullptr};
    widgets::BufferedDrawFunctionWidget *bdw{nullptr};

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
        // analyzer = std::make_unique<FilterAnalzer>();
        bdw = new widgets::BufferedDrawFunctionWidget(rack::Vec(0, 0), box.size,
                                                      [this](auto vg) { this->drawUnder(vg); });
        addChild(bdw);
    }
    void drawUnder(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgFillColor(vg, style()->getColor(style::XTStyle::PLOT_CURVE));

        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, style()->fontIdBold(vg));
        nvgFontSize(vg, 14 * 96 / 72);
        nvgText(vg, this->box.size.x * 0.5, this->box.size.y * 0.5, "Plot Soon", nullptr);
    }

    void onStyleChanged() override { bdw->dirty = true; }
};

VCFWidget::VCFWidget(VCFWidget::M *module) : XTModuleWidget()
{
    setModule(module);
    box.size = rack::Vec(rack::app::RACK_GRID_WIDTH * numberOfScrews, rack::app::RACK_GRID_HEIGHT);

    auto bg = new widgets::Background(box.size, "FILTER", "vco", "BlankVCO");
    addChild(bg);

    int idx = 0;

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

    auto subType = VCFSubtypeSelector::create(rack::Vec(plotStartX, underPlotStartY),
                                              rack::Vec(plotW, underPlotH), module, M::VCF_SUBTYPE);
    addChild(subType);
    for (const auto &[row, col, pid, label] :
         {std::make_tuple(0, 0, M::FREQUENCY, "FREQUENCY"),
          std::make_tuple(0, 2, M::RESONANCE, "RESO"), std::make_tuple(0, 3, M::MIX, "MIX"),
          std::make_tuple(1, 2, M::IN_GAIN, "GAIN"), std::make_tuple(1, 3, M::OUT_GAIN, "GAIN")})
    {
        auto uxp = columnCenters_MM[col];
        auto uyp = rowCenters_MM[row];

        widgets::KnobN *baseKnob{nullptr};
        if (row == 0 && col == 0)
        {
            uxp = (columnCenters_MM[0] + columnCenters_MM[1]) * 0.5f;
            uyp = (rowCenters_MM[0] + rowCenters_MM[1]) * 0.5f;

            auto boxx0 = uxp - columnWidth_MM;
            auto boxy0 = uyp + 8;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(columnWidth_MM * 2, 5));

            auto lab = widgets::Label::createWithBaselineBox(p0, s0, label);
            addChild(lab);
            baseKnob = rack::createParamCentered<widgets::Knob16>(rack::mm2px(rack::Vec(uxp, uyp)),
                                                                  module, pid);
        }
        else
        {
            addChild(makeLabel(row, col, label));
            baseKnob = rack::createParamCentered<widgets::Knob9>(rack::mm2px(rack::Vec(uxp, uyp)),
                                                                 module, pid);
        }
        addParam(baseKnob);
        for (int m = 0; m < M::n_mod_inputs; ++m)
        {
            auto radius = rack::mm2px(baseKnob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
            int id = M::modulatorIndexFor(pid, m);
            auto *k = widgets::ModRingKnob::createCentered(rack::mm2px(rack::Vec(uxp, uyp)), radius,
                                                           module, id);
            overlays[idx][m] = k;
            k->setVisible(false);
            k->underlyerParamWidget = baseKnob;
            baseKnob->modRings.insert(k);
            addChild(k);
        }

        idx++;
    }

    int row{0}, col{0};
    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        addChild(makeLabel(2, i, std::string("MOD ") + std::to_string(i + 1)));
    }

    col = 0;
    row = 3;

    for (int i = 0; i < M::n_mod_inputs; ++i)
    {
        auto uxp = columnCenters_MM[i];
        auto uyp = rowCenters_MM[2];

        auto *k =
            rack::createWidgetCentered<widgets::ModToggleButton>(rack::mm2px(rack::Vec(uxp, uyp)));
        toggles[i] = k;
        k->onToggle = [this, toggleIdx = i](bool isOn) {
            for (const auto &t : toggles)
                if (t)
                    t->setState(false);
            for (const auto &ob : overlays)
                for (const auto &o : ob)
                    if (o)
                        o->setVisible(false);
            if (isOn)
            {
                toggles[toggleIdx]->setState(true);
                for (const auto &ob : overlays)
                    if (ob[toggleIdx])
                    {
                        ob[toggleIdx]->setVisible(true);
                        ob[toggleIdx]->bdw->dirty = true;
                    }
            }
        };

        addChild(k);

        uyp = rowCenters_MM[3];

        addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)), module,
                                                          M::VCF_MOD_INPUT + i));
    }

    col = 0;
    for (auto p : {M::INPUT_L, M::INPUT_R})
    {
        auto yp = rowCenters_MM[4];
        auto xp = columnCenters_MM[col];
        addInput(
            rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    for (auto p : {M::OUTPUT_L, M::OUTPUT_R})
    {
        auto yp = rowCenters_MM[4];
        auto xp = columnCenters_MM[col];
        addOutput(
            rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)), module, p));
        col++;
    }

    col = 0;
    for (const std::string &s : {"LEFT", "RIGHT", "LEFT", "RIGHT"})
    {
        addChild(makeLabel(
            3, col, s, (col < 2 ? style::XTStyle::TEXT_LABEL : style::XTStyle::TEXT_LABEL_OUTPUT)));
        col++;
    }
}
} // namespace sst::surgext_rack::vcf::ui

// namespace sst::surgext_rack::vcf::ui

rack::Model *modelSurgeVCF = rack::createModel<sst::surgext_rack::vcf::ui::VCFWidget::M,
                                               sst::surgext_rack::vcf::ui::VCFWidget>("SurgeXTVCF");
