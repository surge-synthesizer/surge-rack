#include "SurgeVCF.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeVCFWidget : SurgeModuleWidgetCommon {
    typedef SurgeVCF M;
    SurgeVCFWidget(M *module);

    void moduleBackground(NVGcontext *vg) {
        drawLeftRightInputOutputBackground(vg, box);

        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGBA(255,0,0,255));
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 20 );
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER );
        nvgText( vg, box.size.x/2, 200, "DO NOT USE", NULL );
        nvgText( vg, box.size.x/2, 230, "STILL IN DEV", NULL );
    }
};

SurgeVCFWidget::SurgeVCFWidget(SurgeVCFWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 20, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "VCF");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    // FIXME - size obviously wrong
    SurgeButtonBank *typeBank = SurgeButtonBank::create(rack::Vec( padFromEdge, SCREW_WIDTH + padFromEdge ),
                                                        rack::Vec( 7 * SCREW_WIDTH - padFromEdge - padMargin, 5 * SCREW_WIDTH ),
                                                        module, M::FILTER_TYPE, 3, 3 );
    typeBank->addLabel( "lp12" );
    typeBank->addLabel( "lp24" );
    typeBank->addLabel( "lplad" );
    typeBank->addLabel( "hp12" );
    typeBank->addLabel( "hp24" );
    typeBank->addLabel( "bp" );
    typeBank->addLabel( "notch" );
    typeBank->addLabel( "comb" );
    typeBank->addLabel( "s&h" );
    addParam( typeBank );

    SurgeButtonBank *subTypeBank = SurgeButtonBank::create(rack::Vec( padFromEdge + 8 * SCREW_WIDTH, SCREW_WIDTH + padFromEdge ),
                                                        rack::Vec( 7 * SCREW_WIDTH / 3.0, 5 * SCREW_WIDTH ),
                                                        module, M::FILTER_SUBTYPE, 1, 3 );
    subTypeBank->addLabel("SVF");
    subTypeBank->addLabel("Rough");
    subTypeBank->addLabel("Smooth");
    addParam(subTypeBank);


    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box), module,
                                               M::INPUT_GAIN
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box), module,
                                               M::OUTPUT_GAIN

                                               ));

}

rack::Model *modelSurgeVCF =
    rack::createModel<SurgeVCFWidget::M, SurgeVCFWidget>("SurgeVCF");
