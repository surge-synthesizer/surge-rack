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

#ifndef SURGE_RACK_XTMODULEWIDGET
#define SURGE_RACK_XTMODULEWIDGET 1

#include "rack.hpp"
#include "XTStyle.h"
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

    int snapNamesEvery{0};
    void step() override
    {
        if (snapNamesEvery == 0)
        {
            snapNamesEvery =
                2 * APP->window->getMonitorRefreshRate() / rack::settings::frameSwapInterval;
            if (module)
            {
                for (auto *pq : module->paramQuantities)
                {
                    if (auto *s = dynamic_cast<modules::CalculatedName *>(pq))
                    {
                        pq->name = s->getCalculatedName();
                    }
                }
            }
        }
        snapNamesEvery--;
        ModuleWidget::step();
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