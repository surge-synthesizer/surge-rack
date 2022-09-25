#ifndef SURGE_RACK_XTMODULEWIDGET
#define SURGE_RACK_XTMODULEWIDGET 1

#include "rack.hpp"
#include "XTStyle.hpp"
#include <array>
#include "XTWidgets.h"

namespace sst::surgext_rack::widgets
{
struct XTModuleWidget : public virtual rack::ModuleWidget, style::StyleParticipant
{
    virtual void onStyleChanged() override { dirtyFB(this); }

    void dirtyFB(rack::Widget *w)
    {
        auto f = dynamic_cast<rack::FramebufferWidget *>(w);
        if (f)
            f->dirty = true;
        for (auto c : w->children)
            dirtyFB(c);
    }

    virtual void appendContextMenu(rack::ui::Menu *menu) override;

  protected:
    void resetStyleCouplingToModule();
    void toggleCoupleToGlobalStyle();
};

struct StandardWidthWithModulationConstants
{
    static constexpr std::array<float, 4> columnCenters_MM{9.48, 23.48, 37.48, 51.48};
    static constexpr std::array<float, 2> modulationRowCenters_MM{85.32, 100.16};
    static constexpr float inputRowCenter_MM = 114.5;
    static constexpr float columnWidth_MM = 14;
    static constexpr float modulationLabelBaseline_MM = 94.864;
    static constexpr float inputLabelBaseline_MM = 109.203;
    static constexpr float verticalPortOffset_MM = 0.5;

    static constexpr int numberOfScrews = 12;

    Label *makeLabelAt(float rowPos, int col, const std::string label,
                       style::XTStyle::Colors clr = style::XTStyle::TEXT_LABEL)
    {
        auto cx = columnCenters_MM[col];
        auto bl = rowPos;

        auto boxx0 = cx - columnWidth_MM * 0.5;
        auto boxy0 = bl - 5;

        auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
        auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));

        auto lab = widgets::Label::createWithBaselineBox(p0, s0, label, 7.3, clr);
        return lab;
    }

    rack::Widget *makeModLabel(int col)
    {
        auto label = "MOD " + std::to_string(col + 1);
        auto bl = modulationLabelBaseline_MM;

        return makeLabelAt(bl, col, label);
    }

    rack::Widget *makeIORowLabel(int col, const std::string &lab, bool isInput)
    {
        auto bl = inputLabelBaseline_MM;
        return makeLabelAt(
            bl, col, lab, isInput ? style::XTStyle::TEXT_LABEL : style::XTStyle::TEXT_LABEL_OUTPUT);
    }
};

struct VCOVCFConstants : StandardWidthWithModulationConstants
{
    static constexpr float plotH_MM = 36;
    static constexpr float plotW_MM = 51;
    static constexpr float plotCX_MM = 30.48;
    static constexpr float plotCY_MM = 27.35;

    static constexpr float plotControlsH_MM = 5;

    static constexpr std::array<float, 5> rowCenters_MM{
        55, 71, modulationRowCenters_MM[0], modulationRowCenters_MM[1], inputRowCenter_MM};
    static constexpr std::array<float, 2> labelBaselines_MM{63.573, 79.573};

    float plotStartX = rack::mm2px(plotCX_MM - plotW_MM * 0.5);
    float plotStartY = rack::mm2px(plotCY_MM - plotH_MM * 0.5);
    float plotW = rack::mm2px(plotW_MM);
    float plotH = rack::mm2px(plotH_MM - plotControlsH_MM);

    float underPlotStartY = plotStartY + plotH;
    float underPlotH = rack::mm2px(plotControlsH_MM);

    Label *makeLabel(int row, int col, const std::string label,
                     style::XTStyle::Colors clr = style::XTStyle::TEXT_LABEL)
    {
        auto bl = labelBaselines_MM[row];
        return makeLabelAt(bl, col, label, clr);
    }
};
} // namespace sst::surgext_rack::widgets

#endif