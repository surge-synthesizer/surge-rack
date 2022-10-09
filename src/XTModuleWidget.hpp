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

    void onHoverKey(const HoverKeyEvent &e) override
    {
        if (e.action == GLFW_PRESS && (e.mods & GLFW_MOD_ALT))
        {
            if (e.key >= '1' && e.key <= '4')
            {
                auto whichMod = e.key - '1';
                selectModulator(whichMod);
                e.consume(this);
                return;
            }
        }
        ModuleWidget::onHoverKey(e);
    }

    virtual void selectModulator(int whichMod) {}

    virtual void appendModuleSpecificMenu(rack::ui::Menu *menu) {}
    virtual void appendContextMenu(rack::ui::Menu *menu) override;

  protected:
    void resetStyleCouplingToModule();
    void toggleCoupleToGlobalStyle();
};
} // namespace sst::surgext_rack::widgets

#endif