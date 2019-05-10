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
#ifndef RACK_V1
#include "widgets.hpp"
#endif

struct SurgeModuleWidgetCommon : public virtual rack::ModuleWidget {
#if RACK_V1
    SurgeModuleWidgetCommon() : rack::ModuleWidget() { }
#else
    SurgeModuleWidgetCommon(rack::Module *m) : rack::ModuleWidget(m) { }

    int stepsSinceRefresh = 0;
    void step() override {
        /*
        ** This basically moves the surgesmallknobs if they are invalidated
        ** which is required in V0.6.2 (but not in V1 where this is done internally
        ** in the knobs). Since I use a dyn cast to find em, only do it every
        ** 10 frames or so
        */
        if( module && stepsSinceRefresh == 0)
        {
            for(auto pw : params )
            {
                SurgeSmallKnob *ssk;
                if( (ssk = dynamic_cast<SurgeSmallKnob *>(pw) ) != nullptr )
                {
                    if( ssk->value != module->params[ssk->paramId].value )
                    {
                        ssk->setValue(module->params[ssk->paramId].value);
                    }
                }
            }
        }
        stepsSinceRefresh ++;
        if( stepsSinceRefresh >= 10 ) stepsSinceRefresh = 0;
        
        ModuleWidget::step();
    }
    
#endif    
};

/*
** FIXME: THis class is dumb and we should remove it
*/

#if RACK_V1
struct SurgeParamLargeWidget : public rack::widget::Widget
#else
struct SurgeParamLargeWidget : public rack::TransparentWidget
#endif
{
#if RACK_V1
    SurgeParamLargeWidget() : rack::widget::Widget() {}
#else
    SurgeParamLargeWidget() : rack::TransparentWidget() {}
#endif
    static const int height = 40;
    static const int width = 14 * SCREW_WIDTH;

    static constexpr float portX = 24.6721;
    static constexpr float portY = 24.6721;
    static const int sknobX = 24;
    static const int sknobY = 24;
    static const int knobX = 34;
    static const int knobY = 34;
    static const int toggleX = 7;
    static const int toggleY = 20;
    static const int itemMargin = 3;

    static SurgeParamLargeWidget *
    create(rack::ModuleWidget *mw, rack::Module *module, rack::Vec pos,
           int paramID, int paramGainID, int cvID, int extendedSwitchID,
           TextDisplayLight::stringGetter_t labelfn,
           TextDisplayLight::stringDirtyGetter_t labelDirtyFn,
           TextDisplayLight::stringGetter_t sublabelfn,
           TextDisplayLight::stringDirtyGetter_t sublabelDirtyFn,
           TextDisplayLight::stringGetter_t valuefn,
           TextDisplayLight::stringDirtyGetter_t valueDirtyFn) {
        SurgeParamLargeWidget *res = new SurgeParamLargeWidget();
        res->box.pos = pos;
        res->box.size = rack::Vec(width, height);

        res->addChild(new BufferedDrawFunctionWidget(
            rack::Vec(0, 0), res->box.size,
            [res](NVGcontext *vg) { res->drawBG(vg); }));
        int text0 = portX + knobX + sknobX + toggleX + 6 * itemMargin;

        res->addChild(TextDisplayLight::create(
            rack::Vec(text0 + 3, 2),
            rack::Vec(res->box.size.x - text0 - 6, res->box.size.y), sublabelfn,
            sublabelDirtyFn, 8, NVG_ALIGN_TOP | NVG_ALIGN_RIGHT));

        TextDisplayLight *lt;
        res->addChild(lt = TextDisplayLight::create(
                          rack::Vec(text0 + 3, 2),
                          rack::Vec(res->box.size.x - text0 - 6, 14), labelfn,
                          labelDirtyFn, 14));
        lt->font = SurgeStyle::fontFaceCondensed();

        res->addChild(
            lt = TextDisplayLight::create(
                rack::Vec(text0 + 3, 2),
                rack::Vec(res->box.size.x - text0 - 4, res->box.size.y - 4),
                valuefn, valueDirtyFn, 18, NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT,
                SurgeStyle::surgeWhite()));
        lt->font = SurgeStyle::fontFaceCondensed();

        mw->addInput(rack::createInput<rack::PJ301MPort>(
            rack::Vec(res->box.pos.x + itemMargin,
                      res->box.pos.y + res->box.size.y / 2 - portX / 2),
            module, cvID));

        mw->addParam(rack::createParam<SurgeSmallKnob>(
            rack::Vec(res->box.pos.x + 2 * itemMargin + portX,
                      res->box.pos.y + res->box.size.y / 2 - sknobY / 2),
            module, paramGainID
#ifndef RACK_V1
            ,
            0, 1, 0.5
#endif
            ));

        mw->addParam(rack::createParam<SurgeSwitch>(
            rack::Vec(res->box.pos.x + sknobX + 4 * itemMargin + portX,
                      res->box.pos.y + res->box.size.y / 2 - toggleY / 2),
            module, extendedSwitchID
#ifndef RACK_V1
            ,
            0, 1, 0
#endif
            ));

        mw->addParam(rack::createParam<SurgeKnobRooster>(
            rack::Vec(res->box.pos.x + sknobX + 5 * itemMargin + portX +
                          toggleX,
                      res->box.pos.y + res->box.size.y / 2 - knobY / 2),
            module, paramID
#ifndef RACK_V1
            ,
            0, 1, 0.5
#endif
            ));

        return res;
    }

    void drawBG(NVGcontext *vg) {
        int text0 = portX + sknobX + knobX + toggleX + 6 * itemMargin;
        SurgeStyle::drawTextBGRect(vg, text0, 0, box.size.x - text0,
                                   box.size.y);
    }
};
