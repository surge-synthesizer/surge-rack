/*
** Widgets to ease double buffering with lambdas, to create input panel areas,
** to have a common background widget and more
*/
#include "Surge.hpp"
#include "SurgeStyle.hpp"
#include "SurgeWidgets.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <functional>
#include <map>

struct SurgeModuleWidgetCommon : public virtual rack::ModuleWidget, SurgeStyle, SurgeStyle::StyleListener {
    SurgeModuleWidgetCommon() : rack::ModuleWidget() {
        loadDefaultStyle();
        addStyleListener(this);
    }
    ~SurgeModuleWidgetCommon() {
        removeStyleListener(this);
    }
    virtual void styleHasChanged() override {
        dirtyFB(this);
    }
    
    void dirtyFB(rack::Widget *w) {
        auto f = dynamic_cast<rack::FramebufferWidget *>(w);
        if(f)
            f->dirty = true;
        for( auto c : w->children )
            dirtyFB(c);
    }

    virtual void appendContextMenu(rack::ui::Menu* menu ) override;
};

