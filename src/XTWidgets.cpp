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

#include "XTWidgets.h"

#include "sst/rackhelpers/module_connector.h"

namespace sst::surgext_rack::widgets
{
namespace mcon = sst::rackhelpers::module_connector;

void Port::appendContextMenu(rack::Menu *menu)
{
    if (connectOutputToNeighbor)
    {
        mcon::connectOutputToNeighorInput(menu, module, false);
    }

    if (connectAsOutputToMixmaster)
    {
        auto mixM = mcon::findMixMasters();
        auto auxM = mcon::findAuxSpanders();

        auto lid = portId;
        auto rid = mixMasterStereoCompanion;
        if (lid > rid)
            std::swap(lid, rid);

        if (!mixM.empty() || !auxM.empty())
        {
            menu->addChild(new rack::MenuSeparator());
        }
        for (auto m : mixM)
        {
            menu->addChild(
                rack::createSubmenuItem(m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                    mcon::outputsToMixMasterSubMenu(x, m, this->module, lid, rid);
                }));
        }

        for (auto m : auxM)
        {
            menu->addChild(
                rack::createSubmenuItem(m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                    mcon::outputsToAuxSpanderSubMenu(x, m, this->module, lid, rid);
                }));
        }
    }

    if (connectAsInputFromMixmaster)
    {
        auto auxM = mcon::findAuxSpanders();

        auto lid = portId;
        auto rid = mixMasterStereoCompanion;
        if (lid > rid)
            std::swap(lid, rid);

        if (module->inputs[lid].isConnected() || module->inputs[rid].isConnected())
        {
            // Don't show the menu
        }
        else
        {
            for (auto m : auxM)
            {
                menu->addChild(
                    rack::createSubmenuItem(m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                        mcon::inputsFromAuxSpanderSubMenu(x, m, this->module, lid, rid,
                                                          nvgRGB(38, 99, 190));
                    }));
            }
        }
    }
}
} // namespace sst::surgext_rack::widgets