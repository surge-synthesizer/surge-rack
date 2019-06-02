#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

template <int effectType>
struct SurgeVOCWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<effectType> M;
    SurgeVOCWidget(M *module);

    int nControls = 4;
    
    float controlAreaHeight = orangeLine - padMargin - SCREW_WIDTH;
    float controlHeight = controlAreaHeight / 12 + 23;
    float controlOffset = controlHeight - padMargin - portY;
    float textAreaHeight = portY;

    float modY0 = orangeLine - 2 * ioMargin - 2 * ( RACK_HEIGHT - orangeLine - 2 * ioMargin );


    void moduleBackground(NVGcontext *vg) {
        int textAreaWidth = box.size.x - 4 * padMargin - 2 * portX;

        std::vector<std::string> lab = { "Gain / Level", "Gate / Level", "Rate / Filter", "Q / Filter" };
        for( int i=0; i<nControls; ++i )
        {
            fxGroupLabel(vg, i * controlHeight + SCREW_WIDTH + 2 * padMargin, lab[i].c_str(), box );
            SurgeStyle::drawTextBGRect(vg, 3*padMargin+2*portX , i*controlHeight + SCREW_WIDTH + padMargin + controlOffset,
                                       textAreaWidth, textAreaHeight );
        }

        drawStackedInputOutputBackground(vg, box, "Carrier");

        float x0 = box.size.x/2 - ioRegionWidth/2;
        float boxHt = box.size.y-orangeLine-2*ioMargin;
        drawBlueIORect(vg, box.size.x/2-ioRegionWidth/2, modY0,
                       ioRegionWidth, boxHt );

        nvgFillColor(vg, surgeWhite());
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 12);
        nvgSave(vg);
        nvgTranslate(vg, x0 + ioRegionWidth - 2, modY0 + ioMargin * 0.5 );
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgRotate(vg, M_PI / 2);
        nvgText(vg, 0, 0, "Mod", NULL);
        nvgRestore(vg);
        rack::Vec ll;
        ll = ioPortLocation(0, 0, box, true);
        ll.y = orangeLine - 0.5* ioMargin - boxHt - 2 * ioMargin;
        ll.x += 24.6721 / 2;
        nvgFontSize(vg, 11);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "L/Mon", NULL);
        
        ll = ioPortLocation(0, 1, box, true);
        ll.y = orangeLine - 0.5* ioMargin - boxHt - 2 * ioMargin;
        ll.x += 24.6721 / 2;
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "R", NULL);
        
        ll = ioPortLocation(0, 2, box, true);
        ll.y = orangeLine - 0.5* ioMargin - boxHt - 2 * ioMargin;
        ll.x += 24.6721 / 2;
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, ll.x, ll.y, "Gain", NULL);
        
        

    }

};

template <int effectType>
SurgeVOCWidget<effectType>::SurgeVOCWidget(SurgeVOCWidget<effectType>::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 8, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, SurgeFXName<effectType>::getName());
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };

    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box,true),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box,true),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box,true), module,
                                               M::INPUT_GAIN
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box,true), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box,true),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box,true), module,
                                               M::OUTPUT_GAIN

                                               ));

    float boxHt = box.size.y-orangeLine-2*ioMargin;
    rack::Vec v = ioPortLocation(true,0,box,true);
    v.y -= boxHt + ioMargin;
    addInput(rack::createInput<rack::PJ301MPort>(v,
                                                 module, M::MODULATOR_L_OR_MONO));
    v = ioPortLocation(true,1,box,true);
    v.y -= boxHt + ioMargin;
    addInput(rack::createInput<rack::PJ301MPort>(v, 
                                                 module, M::MODULATOR_R));

    v = ioPortLocation(true,2,box,true);
    v.y -= boxHt + ioMargin;
    addParam(rack::createParam<SurgeSmallKnob>(v,
                                               module, M::MODULATOR_GAIN));


    int textAreaWidth = box.size.x - 4 * padMargin - 2 * portX;
    for( int i=0; i<nControls; ++i )
    {
        float yPos = i * controlHeight + SCREW_WIDTH + padMargin + controlOffset;
        int pa = M::FX_PARAM_0 + i;
        int cv = M::FX_PARAM_INPUT_0 + i;
        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(2 * padMargin + portX, yPos), module, cv ));
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(padMargin, yPos), module,
                                                   pa
                                                   ));

        
        int tx = 3 * padMargin + 2 * portX + 2;
        addChild(TextDisplayLight::create(rack::Vec(tx , yPos),
                                          rack::Vec(textAreaWidth - 2 * padMargin, textAreaHeight ),
                                          module ? &(module->pb[pa]->valCache) : nullptr,
                                          14, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, SurgeStyle::surgeWhite()));
        
    }
}


#define CREATE_FX( type, name ) auto v ## type = addFX( \
     rack::createModel<SurgeVOCWidget< type >::M, SurgeVOCWidget< type >>(name), type);

// FIXME: Eventually each of these get their own panel and this list drops to 0
CREATE_FX( fxt_vocoder, "SurgeVOC" );
