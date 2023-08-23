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

#ifndef SURGE_XT_RACK_SRC_LAYOUTENGINE_H
#define SURGE_XT_RACK_SRC_LAYOUTENGINE_H

#include <unordered_map>
#include "XTWidgets.h"
#include "XTModuleWidget.h"
#include "LayoutConstants.h"

namespace sst::surgext_rack::layout
{
struct LayoutItem
{
    // order matters a bit on this enum. knobs contiguous pls
    enum Type
    {
        KNOB9,
        KNOB12,
        KNOB14,
        KNOB16,
        VSLIDER,
        VSLIDER_25,
        PORT,
        OUT_PORT,
        MOMENTARY_PARAM,
        TOGGLE_PARAM,
        GROUP_LABEL,
        KNOB_SPAN_LABEL,
        LCD_BG,
        LCD_MENU_ITEM_SURGE_PARAM,
        POWER_LIGHT,
        EXTEND_LIGHT,
        ERROR
    } type{ERROR};
    std::string label{"ERR"};
    int parId{-1};
    float xcmm{-1}, ycmm{-1};
    float spanmm{0}; // for group label only
    float overrideColWidth_MM{layout::LayoutConstants::columnWidth_MM};

    bool skipModulation{false};
    bool dynamicLabel{false};
    std::function<std::string(modules::XTModule *m)> dynLabelFn{nullptr};

    static LayoutItem createLCDArea(float ht)
    {
        auto res = LayoutItem();
        res.type = LCD_BG;
        res.ycmm = ht;
        return res;
    }

    static LayoutItem createPresetLCDArea() { return createLCDArea(14.9); }

    static LayoutItem createPresetPlusOneArea()
    {
        // This 19 should match the 19 below
        return createLCDArea(19).withExtra("CENTER_RULE", true);
    }

    static LayoutItem createPresetPlusTwoArea()
    {
        // This 19 should match the 19 below
        return createLCDArea(19).withExtra("CENTER_RULE", true).withExtra("SPLIT_LOWER", true);
    }

    static LayoutItem createSingleMenuItem(const std::string &lab, int param)
    {
        // This 19 should match the 19 above
        return {LayoutItem::LCD_MENU_ITEM_SURGE_PARAM, lab, param, 0, 19};
    }

    static LayoutItem createLeftMenuItem(const std::string &lab, int param)
    {
        // This 19 should match the 19 above
        LayoutItem res{LayoutItem::LCD_MENU_ITEM_SURGE_PARAM, lab, param, 0, 19};
        return res.withExtra("SIDE", 1);
    }

    static LayoutItem createRightMenuItem(const std::string &lab, int param)
    {
        // This 19 should match the 19 above
        LayoutItem res{LayoutItem::LCD_MENU_ITEM_SURGE_PARAM, lab, param, 0, 19};
        return res.withExtra("SIDE", -1);
    }

    static LayoutItem
    createGrouplabel(const std::string &label, float xcmm, float ycmm, float span,
                     float spanColumnW_MM = layout::LayoutConstants::columnWidth_MM)
    {
        auto res = LayoutItem();
        res.label = label;
        res.type = GROUP_LABEL;
        res.xcmm = xcmm;
        res.ycmm = ycmm;
        res.spanmm = span;
        res.overrideColWidth_MM = spanColumnW_MM;
        return res;
    }

    static LayoutItem createKnobSpanLabel(const std::string &label, float xcmm, float ycmm,
                                          float span)
    {
        auto res = LayoutItem();
        res.label = label;
        res.type = KNOB_SPAN_LABEL;
        res.xcmm = xcmm;
        res.ycmm = ycmm;
        res.spanmm = span;
        return res;
    }

    static LayoutItem createVCOItem(Type t, int param, const std::string &label, int row, int col)
    {
        auto res = LayoutItem();
        res.type = t;
        res.label = label;
        res.parId = param;
        res.xcmm = LayoutConstants::firstColumnCenter_MM + LayoutConstants::columnWidth_MM * col;
        res.ycmm = LayoutConstants::vcoRowCenters_MM[row];
        return res;
    }

    static LayoutItem createVCOLight(Type t, int param, int row, int col)
    {
        auto res = createVCOItem(t, param, "", row, col);
        res.spanmm = 1;
        return res;
    }

    static LayoutItem createVCOKnob(int param, const std::string &label, int row, int col)
    {
        return createVCOItem(KNOB9, param, label, row, col);
    }

    static LayoutItem createVCFWSBigKnob(int param, const std::string &label)
    {
        auto res = LayoutItem();
        res.type = KNOB16;
        res.label = label;
        res.parId = param;
        res.xcmm = LayoutConstants::firstColumnCenter_MM + LayoutConstants::columnWidth_MM * 0.5;
        res.ycmm =
            (LayoutConstants::vcoRowCenters_MM[0] + LayoutConstants::vcoRowCenters_MM[1]) * 0.5f;
        return res;
    }

    static LayoutItem createVCOKnob(int param,
                                    std::function<std::string(modules::XTModule *)> labelFunc,
                                    int row, int col)
    {
        auto res = createVCOItem(KNOB9, param, "DYN", row, col);
        res.dynamicLabel = true;
        res.dynLabelFn = std::move(labelFunc);
        return res;
    }

    static LayoutItem createVCOPort(int param, const std::string &label, int row, int col)
    {
        return createVCOItem(PORT, param, label, row, col);
    }

    static LayoutItem createVCOSpanLabel(const std::string &label, int row, int col, int span)
    {
        return createKnobSpanLabel(
            label, LayoutConstants::firstColumnCenter_MM + LayoutConstants::columnWidth_MM * col,
            LayoutConstants::vcoRowCenters_MM[row], span);
    }

    static LayoutItem createVCOSpanDynamicLabel(std::function<std::string(modules::XTModule *m)> df,
                                                int row, int col, int span)
    {
        auto res = createVCOSpanLabel("DYN", row, col, span);
        res.dynamicLabel = true;
        res.dynLabelFn = df;
        return res;
    }

    std::unordered_map<std::string, float> extras{};
    LayoutItem &withExtra(const std::string &s, float f)
    {
        extras[s] = f;
        return *this;
    }
};

template <typename W, int param0, int clockId = -1> struct LayoutEngine
{
    static widgets::Label *
    makeSpanLabelAt(float rowPos, int col, const std::string label, int span,
                    style::XTStyle::Colors clr = style::XTStyle::TEXT_LABEL,
                    float colWidth_MM = layout::LayoutConstants::columnWidth_MM)
    {
        auto cx = LayoutConstants::firstColumnCenter_MM + colWidth_MM * col;
        auto bl = rowPos;

        auto boxx0 = cx - colWidth_MM * 0.5;
        auto boxy0 = bl - 5;

        auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
        auto s0 = rack::mm2px(rack::Vec(colWidth_MM * span, 5));

        auto lab = widgets::Label::createWithBaselineBox(p0, s0, label,
                                                         LayoutConstants::labelSize_pt, clr);
        return lab;
    }

    static widgets::Label *makeLabelAt(float rowPos, int col, const std::string label,
                                       style::XTStyle::Colors clr = style::XTStyle::TEXT_LABEL,
                                       float colWidth_MM = layout::LayoutConstants::columnWidth_MM)
    {
        return makeSpanLabelAt(rowPos, col, label, 1, clr, colWidth_MM);
    }

    static void layoutItem(W *w, const LayoutItem &lay, const std::string &panelName)
    {
        typedef LayoutConstants lc;

        auto module = static_cast<modules::XTModule *>(w->module);
        switch (lay.type)
        {
        case LayoutItem::KNOB9:
        case LayoutItem::KNOB12:
        case LayoutItem::KNOB14:
        case LayoutItem::KNOB16:
        case LayoutItem::VSLIDER:
        case LayoutItem::VSLIDER_25:
        {
            widgets::ModulatableKnob *knob{nullptr};
            int diff = lay.type - LayoutItem::KNOB9;
            auto pos = rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm));
            float halfSize{0}; // diff in radius from 4.5
            auto par = param0 + lay.parId;
            if (diff == 0)
            {
                knob = rack::createParamCentered<widgets::Knob9>(pos, module, par);
                halfSize = 0;
            }
            if (diff == 1)
            {
                knob = rack::createParamCentered<widgets::Knob12>(pos, module, par);
                halfSize = (12 - 9) * 0.5 + 1; // just give a smidge more space for
                // the word "SATURATION" to not clip
            }
            if (diff == 2)
            {
                knob = rack::createParamCentered<widgets::Knob14>(pos, module, par);
                halfSize = (14 - 9) * 0.5;
            }
            if (diff == 3)
            {
                knob = rack::createParamCentered<widgets::Knob16>(pos, module, par);
                halfSize = (16 - 9) * 0.5;
            }
            if (diff == 4)
            {
                knob = widgets::VerticalSlider::createCentered(pos, rack::mm2px(lay.spanmm), module,
                                                               par);
                halfSize = (19 - 9) * 0.5;
            }
            if (diff == 5)
            {
                knob = widgets::VerticalSlider::createCentered(pos, rack::mm2px(lay.spanmm), module,
                                                               par, "fader_bg_25.svg");
                halfSize = (25 - 9) * 0.5;
            }
            if (knob)
            {
                w->addChild(knob->asWidget());
                auto boxx0 = lay.xcmm - lc::columnWidth_MM * 0.5 - halfSize;
                auto boxy0 = lay.ycmm + 8.573 + halfSize - 5;

                auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
                auto s0 = rack::mm2px(rack::Vec(lc::columnWidth_MM + halfSize * 2, 5));
                auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);

                if (lay.dynamicLabel && module)
                {
                    lab->hasDynamicLabel = true;
                    lab->module = static_cast<modules::XTModule *>(module);
                    lab->dynamicLabel = lay.dynLabelFn;
                }

                w->addChild(lab);
            }
            if (knob && !lay.skipModulation)
            {
                w->underKnobs[lay.parId] = knob;

                if (diff < 4)
                {
                    auto rknob = static_cast<widgets::KnobN *>(knob->asWidget());
                    for (int m = 0; m < W::M::n_mod_inputs; ++m)
                    {
                        auto radius =
                            rack::mm2px(rknob->knobSize_MM + 2 * widgets::KnobN::ringWidth_MM);
                        int id = W::M::modulatorIndexFor(lay.parId + param0, m);
                        auto *k = widgets::ModRingKnob::createCentered(pos, radius, module, id);
                        w->overlays[lay.parId][m] = k;
                        k->setVisible(false);
                        k->underlyerParamWidget = rknob;
                        rknob->modRings.insert(k);
                        w->addChild(k);
                    }
                }
                else
                {
                    auto rknob = static_cast<widgets::VerticalSlider *>(knob->asWidget());
                    for (int m = 0; m < W::M::n_mod_inputs; ++m)
                    {
                        int id = W::M::modulatorIndexFor(lay.parId + param0, m);
                        auto *k = widgets::VerticalSliderModulator::createCentered(
                            pos, (diff == 4 ? 19 : 25), module, id);
                        w->overlays[lay.parId][m] = k;
                        k->setVisible(false);
                        k->underlyerParamWidget = rknob;
                        rknob->modSliders.insert(k);
                        w->addChild(k);
                    }
                }
            }
        }
        break;
        case LayoutItem::KNOB_SPAN_LABEL:
        {
            auto boxx0 = lay.xcmm - lc::columnWidth_MM * 0.5;
            auto boxy0 = lay.ycmm + 8.573 - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(lc::columnWidth_MM * lay.spanmm, 5));
            auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);

            if (lay.dynamicLabel && module)
            {
                lab->hasDynamicLabel = true;
                lab->module = static_cast<modules::XTModule *>(module);
                lab->dynamicLabel = lay.dynLabelFn;
            }

            w->addChild(lab);
        }
        break;

        case LayoutItem::MOMENTARY_PARAM:
        {
            auto butt = rack::createParamCentered<widgets::MomentaryParamButton>(
                rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm + lc::verticalPortOffset_MM)), module,
                lay.parId);
            w->addChild(butt);

            auto boxx0 = lay.xcmm - lc::columnWidth_MM * 0.5;
            auto boxy0 = lay.ycmm + 8.573 - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(lc::columnWidth_MM, 5));
            auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);
            w->addChild(lab);
        }
        break;
        case LayoutItem::TOGGLE_PARAM:
        {
            auto butt = rack::createParamCentered<widgets::ToggleParamButton>(
                rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm + lc::verticalPortOffset_MM)), module,
                lay.parId);
            w->addChild(butt);

            auto boxx0 = lay.xcmm - lc::columnWidth_MM * 0.5;
            auto boxy0 = lay.ycmm + 8.573 - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(lc::columnWidth_MM, 5));
            auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);
            w->addChild(lab);
        }
        break;
        case LayoutItem::OUT_PORT:
        {
            auto bg = new widgets::OutputDecoration;
            auto pd_MM = 0.5;
            bg->box.pos = rack::mm2px(rack::Vec(lay.xcmm - lc::columnWidth_MM * 0.35 - pd_MM,
                                                lay.ycmm - lc::columnWidth_MM * 0.3 - pd_MM));
            bg->box.size = rack::mm2px(rack::Vec(lc::columnWidth_MM * 0.7 + pd_MM * 2,
                                                 lc::columnWidth_MM * 0.6 + 5 + pd_MM * 2));
            bg->setup();
            w->addChild(bg);

            auto port = rack::createOutputCentered<widgets::Port>(
                rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm + lc::verticalPortOffset_MM)), module,
                lay.parId);
            w->addChild(port);

            auto boxx0 = lay.xcmm - lc::columnWidth_MM * 0.5;
            auto boxy0 = lay.ycmm + 8.573 - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(lc::columnWidth_MM, 5));
            auto lab = widgets::Label::createWithBaselineBox(
                p0, s0, lay.label, lc::labelSize_pt, style::XTStyle::Colors::TEXT_LABEL_OUTPUT);
            w->addChild(lab);
            break;
        }
        case LayoutItem::PORT:
        {
            auto port = rack::createInputCentered<widgets::Port>(
                rack::mm2px(rack::Vec(lay.xcmm, lay.ycmm + lc::verticalPortOffset_MM)), module,
                lay.parId);
            w->addChild(port);

            auto boxx0 = lay.xcmm - lc::columnWidth_MM * 0.5;
            auto boxy0 = lay.ycmm + 8.573 - 5;

            auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
            auto s0 = rack::mm2px(rack::Vec(lc::columnWidth_MM, 5));
            auto lab = widgets::Label::createWithBaselineBox(p0, s0, lay.label);

            if constexpr (clockId >= 0)
            {
                if (module && lay.parId == clockId)
                {
                    lab->hasDynamicLabel = true;
                    lab->module = module;
                    lab->dynamicLabel = [](modules::XTModule *m) -> std::string {
                        if (!m)
                            return "CLOCK";
                        auto fxm = static_cast<typename W::M *>(m);

                        if (fxm->clockProc.clockStyle == W::M::clockProcessor_t::QUARTER_NOTE)
                        {
                            return "CLOCK";
                        }
                        else
                        {
                            return "BPM";
                        }
                    };
                }
            }

            if (lay.dynamicLabel && lay.parId != clockId)
            {
                lab->hasDynamicLabel = true;
                lab->module = module;
                lab->dynamicLabel = lay.dynLabelFn;
            }

            w->addChild(lab);
        }
        break;
        case LayoutItem::POWER_LIGHT:
        case LayoutItem::EXTEND_LIGHT:
        {
            auto dir = lay.spanmm < 0 ? -1 : 1;
            auto as = abs(lay.spanmm);
            auto x = rack::mm2px(lay.xcmm + dir * (4.5 + as));
            auto y = rack::mm2px(lay.ycmm - (4.5 + as));

            auto light = rack::createParamCentered<widgets::ActivateKnobSwitch>(rack::Vec(x, y),
                                                                                module, lay.parId);

            if (lay.type == LayoutItem::EXTEND_LIGHT)
            {
                light->type = widgets::ActivateKnobSwitch::EXTENDED;
            }
            w->addChild(light);
        }
        break;
        case LayoutItem::GROUP_LABEL:
        {
            auto gl = widgets::GroupLabel::createAboveCenterWithColSpan(
                lay.label, rack::Vec(lay.xcmm, lay.ycmm), lay.spanmm, lay.overrideColWidth_MM);
            if (lay.extras.find("SHORTLEFT") != lay.extras.end())
                gl->shortLeft = true;
            if (lay.extras.find("SHORTRIGHT") != lay.extras.end())
                gl->shortRight = true;
            w->addChild(gl);
        }
        break;
        case LayoutItem::LCD_BG:
        {
            auto bg = widgets::LCDBackground::createWithHeight(lay.ycmm);
            if (lay.extras.find("CENTER_RULE") != lay.extras.end())
                bg->centerRule = true;
            if (lay.extras.find("SPLIT_LOWER") != lay.extras.end())
                bg->splitLower = true;

            if (!module)
                bg->noModuleText = panelName;
            w->addChild(bg);
        }
        break;
        case LayoutItem::LCD_MENU_ITEM_SURGE_PARAM:
        {
            auto xpos = rack::mm2px(widgets::LCDBackground::contentPosX_MM);

            int sd = 0;
            if (lay.extras.find("SIDE") != lay.extras.end())
            {
                sd = lay.extras.find("SIDE")->second;
            }

            auto width = w->box.size.x - 2 * rack::mm2px(widgets::LCDBackground::contentPosX_MM);
            if (sd == 1)
            {
                width = width * 0.5;
            }
            if (sd == -1)
            {
                width = width * 0.5;
                xpos += width;
            }
            auto height = rack::mm2px(5);
            auto ypos = rack::mm2px(lay.ycmm - widgets::LCDBackground::padY_MM) - height;
            auto wid = widgets::PlotAreaMenuItem::create(
                rack::Vec(xpos, ypos), rack::Vec(width, height), module, lay.parId);
            wid->upcaseDisplay = false;
            wid->centerDisplay = true;

            if (sd == 0)
            {
                wid->transformLabel = [n = lay.label](auto s) {
                    if (n.empty())
                        return s;
                    return n + ": " + s;
                };
            }
            else
            {
                wid->transformLabel = [](auto s) { return s; };
            }

            wid->onShowMenu = [w, wid, lay]() {
                if (!w->module)
                    return;

                auto *fxm = static_cast<typename W::M *>(w->module);

                auto pq = wid->getParamQuantity();
                if (!pq)
                    return;

                auto *surgePar = fxm->surgeDisplayParameterForParamId(
                    lay.parId - param0); // fxm->fxstorage->p[lay.parId - param0];
                if (!(surgePar->valtype == vt_int))
                    return;

                auto men = rack::createMenu();
                men->addChild(rack::createMenuLabel(pq->getLabel()));

                // yeah a bit gross
                int step{1};
                if (surgePar->ctrltype == ct_vocoder_bandcount)
                    step = 4;

                for (int i = surgePar->val_min.i; i <= surgePar->val_max.i; i += step)
                {
                    char txt[256];
                    auto fv =
                        Parameter::intScaledToFloat(i, surgePar->val_max.i, surgePar->val_min.i);
                    surgePar->get_display(txt, true, fv);
                    men->addChild(rack::createMenuItem(
                        txt, CHECKMARK(i == surgePar->val.i), [surgePar, pq, fv]() {
                            auto *h = new rack::history::ParamChange;
                            h->name = std::string("change ") + surgePar->get_name();
                            h->moduleId = pq->module->id;
                            h->paramId = pq->paramId;
                            h->oldValue = pq->getValue();
                            h->newValue = fv;
                            APP->history->push(h);

                            pq->setValue(fv);
                        }));
                }
            };
            w->addParam(wid);
        }
        break;
        default:
            break;
        }
    }

    static void initializeModulationToBlank(W *w)
    {
        for (auto &ok : w->overlays)
            for (auto &k : ok)
                k = nullptr;
        for (auto &t : w->toggles)
            t = nullptr;
        for (auto &k : w->underKnobs)
            k = nullptr;
    }

    static void addModulationSection(W *w, int n_mod_inputs, int input0,
                                     float modulationRowOffset_MM = 0,
                                     float colW_MM = LayoutConstants::columnWidth_MM)
    {
        for (int i = 0; i < n_mod_inputs; ++i)
        {
            auto modL = makeLabelAt(
                LayoutConstants::modulationLabelBaseline_MM + modulationRowOffset_MM, i,
                std::string("MOD ") + std::to_string(i + 1), style::XTStyle::TEXT_LABEL, colW_MM);
            w->addChild(modL);
        }

        for (int i = 0; i < n_mod_inputs; ++i)
        {
            auto uxp = LayoutConstants::firstColumnCenter_MM + colW_MM * i;
            auto uyp = LayoutConstants::modulationRowCenters_MM[0] + modulationRowOffset_MM;

            auto *k = rack::createWidgetCentered<widgets::ModToggleButton>(
                rack::mm2px(rack::Vec(uxp, uyp)));
            w->toggles[i] = k;
            k->onToggle = [w, toggleIdx = i](bool isOn) {
                for (const auto &t : w->toggles)
                    if (t)
                        t->setState(false);
                for (const auto &ob : w->overlays)
                    for (const auto &o : ob)
                        if (o)
                            o->setVisible(false);
                if (isOn)
                {
                    w->toggles[toggleIdx]->setState(true);
                    for (const auto &ob : w->overlays)
                        if (ob[toggleIdx])
                        {
                            ob[toggleIdx]->setVisible(true);
                            auto w = dynamic_cast<widgets::HasBDW *>(ob[toggleIdx]);
                            if (w)
                            {
                                w->bdw->dirty = true;
                            }
                        }
                    for (const auto &uk : w->underKnobs)
                        if (uk)
                            uk->setIsModEditing(true);
                }
                else
                {
                    for (const auto &uk : w->underKnobs)
                        if (uk)
                            uk->setIsModEditing(false);
                }
            };

            w->addChild(k);
            uyp = LayoutConstants::modulationRowCenters_MM[1] + modulationRowOffset_MM;
            w->addInput(rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(uxp, uyp)),
                                                                 w->module, input0 + i));
        }
    }

    static void createInputOutputPorts(W *w, int in0, int in1, int out0, int out1)
    {
        int col = 0;
        for (auto p : {in0, in1})
        {
            if (p >= 0)
            {
                auto yp = layout::LayoutConstants::inputRowCenter_MM;
                auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                          layout::LayoutConstants::columnWidth_MM * col;
                auto pt = rack::createInputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                   w->module, p);
                pt->connectAsInputFromMixmaster = true;
                if (in0 >= 0 && in1 >= 0)
                    pt->mixMasterStereoCompanion = (p == in0 ? in1 : in0);
                w->addInput(pt);
            }
            col++;
        }

        for (auto p : {out0, out1})
        {
            auto yp = layout::LayoutConstants::inputRowCenter_MM;
            auto xp = layout::LayoutConstants::firstColumnCenter_MM +
                      layout::LayoutConstants::columnWidth_MM * col;
            auto pt = rack::createOutputCentered<widgets::Port>(rack::mm2px(rack::Vec(xp, yp)),
                                                                w->module, p);
            pt->connectAsOutputToMixmaster = true;
            pt->connectOutputToNeighbor = true;
            pt->mixMasterStereoCompanion = (p == out0 ? out1 : out0);
            w->addOutput(pt);
            col++;
        }
    }

    static void addSingleOutputStripBackground(W *w, int c0 = 2, int nc = 2)
    {
        auto od = new widgets::OutputDecoration;
        auto bl = layout::LayoutConstants::inputLabelBaseline_MM;

        auto pd_MM = 0.5;

        od->box.size = rack::Vec(
            rack::mm2px((nc - 0.2) * layout::LayoutConstants::columnWidth_MM + 2 * pd_MM), 42);
        od->box.pos =
            rack::Vec(rack::mm2px(layout::LayoutConstants::firstColumnCenter_MM +
                                  (c0 - 0.4) * layout::LayoutConstants::columnWidth_MM - pd_MM),
                      rack::mm2px(bl - pd_MM) - 7.2 * 96 / 72);
        od->setup();
        w->addChild(od);
    }
    static void createLeftRightInputLabels(W *w, const std::string &in0 = "LEFT",
                                           const std::string &in1 = "RIGHT")
    {
        int col = 0;

        addSingleOutputStripBackground(w);
        for (const std::string &s : {in0, in1, std::string("LEFT"), std::string("RIGHT")})
        {
            auto bl = layout::LayoutConstants::inputLabelBaseline_MM;
            auto lab = makeLabelAt(bl, col, s,
                                   col < 2 ? style::XTStyle::TEXT_LABEL
                                           : style::XTStyle::TEXT_LABEL_OUTPUT);
            w->addChild(lab);
            col++;
        }
    }
};
} // namespace sst::surgext_rack::layout

#endif
