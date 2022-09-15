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
};

struct GriddedPanelConstants
{
    float plotH_MM = 36;
    float plotW_MM = 51;
    float plotCX_MM = 30.48;
    float plotCY_MM = 27.35;

    float plotControlsH_MM = 5;

    std::array<float, 4> columnCenters_MM{9.48, 23.48, 37.48, 51.48};
    std::array<float, 5> rowCenters_MM{55, 71, 85.32, 100.16, 114.5};
    float columnWidth_MM = 14;

    std::array<float, 4> labelBaselines_MM{63.573, 79.573, 94.864, 109.203};

    float verticalPortOffset_MM = 0.5;

    float plotStartX = rack::mm2px(plotCX_MM - plotW_MM * 0.5);
    float plotStartY = rack::mm2px(plotCY_MM - plotH_MM * 0.5);
    float plotW = rack::mm2px(plotW_MM);
    float plotH = rack::mm2px(plotH_MM - plotControlsH_MM);

    float underPlotStartY = plotStartY + plotH;
    float underPlotH = rack::mm2px(plotControlsH_MM);

    int numberOfScrews = 12;

    rack::Widget *makeLabel(int row, int col, const std::string label,
                            style::XTStyle::Colors clr = style::XTStyle::TEXT_LABEL)
    {
        auto cx = columnCenters_MM[col];
        auto bl = labelBaselines_MM[row];

        auto boxx0 = cx - columnWidth_MM * 0.5;
        auto boxy0 = bl - 5;

        auto p0 = rack::mm2px(rack::Vec(boxx0, boxy0));
        auto s0 = rack::mm2px(rack::Vec(columnWidth_MM, 5));

        auto lab = widgets::Label::createWithBaselineBox(p0, s0, label, 7.3, clr);
        return lab;
    }
};
} // namespace sst::surgext_rack::widgets