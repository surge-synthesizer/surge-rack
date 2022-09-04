/*
** Widgets to ease double buffering with lambdas, to create input panel areas,
** to have a common background widget and more
*/
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "SurgeStyle.hpp"
#include "SurgeWidgets.hpp"
#include "rack.hpp"
#include <functional>
#include <map>

namespace sst::surgext_rack::widgets
{
struct SurgeModuleWidgetCommon : public virtual rack::ModuleWidget,
                                 style::SurgeStyle,
                                 style::StyleListener
{
    SurgeModuleWidgetCommon() : rack::ModuleWidget()
    {
        loadStyle();
        addStyleListener(this);
    }
    ~SurgeModuleWidgetCommon() { removeStyleListener(this); }
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
} // namespace sst::surgext_rack::widgets