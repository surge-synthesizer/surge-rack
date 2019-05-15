#include "SurgeLFO.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeLFOWidget : SurgeModuleWidgetCommon {
    typedef SurgeLFO M;
    SurgeLFOWidget(M *module);


    int modeYPos = 35;

    float buttonBankHeight = 25;

    /* OLD */
    float topControl = SCREW_WIDTH + padMargin;
    float controlAreaHeight = orangeLine - padMargin - topControl;
    int   nControls = M::R_PARAM - M::RATE_PARAM + 1;
    float controlHeight = controlAreaHeight / nControls;
    float textAreaWidth = SCREW_WIDTH * 15 - 4 * padMargin - 2 * portX;

    float oldColStart = SCREW_WIDTH * 18;
    
    float inputXPos(int which) {
        float perI = portX + 2 * padMargin;
        float x0 = box.size.x / 2 - 1.5 * perI;
        float posn = x0 + which * perI;
        return posn;
    }

    float inputYPos(int which) {
        return orangeLine + ioMargin + padMargin;
    }

    float topBoxW = SCREW_WIDTH * 8.2;
    float boxLayerY0 = SCREW_WIDTH + padMargin * 2 + buttonBankHeight;
    float boxLayerH = 50;
    float labelH = 16;
    float UIW = SCREW_WIDTH * 18;
    float buttonBankW = SCREW_WIDTH * 15;
    
    void moduleBackground(NVGcontext *vg) {
        float ioyb = orangeLine + ioMargin;
        auto xb0 = inputXPos(0) - padMargin;
        drawBlueIORect(vg, xb0, ioyb, 3 * portX + 6 * padMargin, box.size.y - orangeLine - 2 * ioMargin );
        std::vector<std::string> lab = { "Gate", "Retrig", "Out" };
        for( int i=0; i<3; ++i )
        {
            auto xc = inputXPos(i) + portX / 2;
            auto yc = box.size.y - ioMargin - 1.5;
            nvgBeginPath(vg);
            nvgFontFaceId(vg,fontId(vg));
            nvgFontSize(vg, 12);
            nvgFillColor(vg, surgeWhite());
            nvgTextAlign(vg,NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, xc, yc, lab[i].c_str(), NULL );
        }

        nvgBeginPath( vg );
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 11);
        nvgFillColor(vg, surgeBlue() );
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgText( vg, UIW - SCREW_WIDTH * 1.2, SCREW_WIDTH + padMargin, "uni", NULL );
        nvgText( vg, UIW - SCREW_WIDTH * 1.2, SCREW_WIDTH + padMargin + 11, "bi", NULL );
        
        centerRuledLabel( vg, padFromEdge, boxLayerY0, topBoxW, "Rate" );
        drawTextBGRect(vg, padMargin*2 + padFromEdge + 2 * portX, boxLayerY0 + labelH + 2, topBoxW - 2 * padMargin - 2 * portX, portY - 4 );
        
        centerRuledLabel( vg, UIW - padFromEdge - topBoxW, boxLayerY0, topBoxW, "Magnitude" );
        drawTextBGRect(vg, UIW - padFromEdge - topBoxW + padMargin*2 + 2 * portX,
                       boxLayerY0 + labelH + 2, topBoxW - 2 * padMargin - 2 * portX, portY - 4 );
        
        centerRuledLabel( vg, padFromEdge, boxLayerY0 + boxLayerH, topBoxW, "Phase" );
        drawTextBGRect(vg, padMargin*2 + padFromEdge + 2 * portX,
                       boxLayerY0 + boxLayerH + labelH + 2, topBoxW - 2 * padMargin - 2 * portX, portY - 4 );

        centerRuledLabel( vg, UIW - padFromEdge - topBoxW, boxLayerY0 + boxLayerH, topBoxW, "Deform" );
        drawTextBGRect(vg, UIW - padFromEdge - topBoxW + padMargin*2 + 2 * portX,
                       boxLayerY0 + boxLayerH + labelH + 2, topBoxW - 2 * padMargin - 2 * portX, portY - 4 );
        

        centerRuledLabel( vg, padFromEdge, 6 * controlHeight + topControl, UIW - 2 * padFromEdge, "Envelope (in progress)" );
        for( int i=7; i<nControls; ++i )
        {
            drawTextBGRect(vg, 3*padMargin+2*portX, i*controlHeight + topControl,
                                       textAreaWidth, controlHeight-padMargin);
        }
        
    }
};

SurgeLFOWidget::SurgeLFOWidget(SurgeLFOWidget::M *module)
#ifndef RACK_V1
    : SurgeModuleWidgetCommon( module ), rack::ModuleWidget( module )
#else
    : SurgeModuleWidgetCommon()
#endif      
{
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 18, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "LFO");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(inputXPos(0), inputYPos(0)), module, M::GATE_IN));
    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(inputXPos(1), inputYPos(1)), module, M::RETRIG_IN));
    addOutput(rack::createOutput<rack::PJ301MPort>(
        rack::Vec(inputXPos(2), inputYPos(2)), module, M::OUTPUT_ENV));


    /* CLEAN UI */
    SurgeButtonBank *bank = SurgeButtonBank::create( rack::Vec(padFromEdge, SCREW_WIDTH + padMargin),
                                                     rack::Vec( buttonBankW, buttonBankHeight),
                                                     module, M::SHAPE_PARAM,
                                                     7, 1, 13 );
    bank->addLabel( "Sin" );
    bank->addLabel( "Tri" );
    bank->addLabel( "Sqr" );
    bank->addLabel( "Ramp" );
    bank->addLabel( "Noise" );
    bank->addLabel( "S&H" );
    bank->addLabel( "Env" );

    bank->normalizeTo = 7; // == 8-1 step seq which we don't show
    
    addParam(bank);

    addParam(rack::createParam<SurgeSwitchFull>( rack::Vec( padFromEdge + padMargin + buttonBankW,
                                                            SCREW_WIDTH + padMargin + (buttonBankHeight-21)/2.0),
                                                 module, M::RATE_PARAM + 6
#ifndef RACK_V1
                                                 ,0,1,0
#endif
                 ));
    
    // Vector is param #, x, y
    std::vector<std::vector<float>> rmpd = { { 0, padFromEdge, boxLayerY0 },
                                             { 3, UIW - padFromEdge - topBoxW, boxLayerY0 },
                                             { 2, padFromEdge, boxLayerY0 + boxLayerH },
                                             { 4, UIW - padFromEdge - topBoxW, boxLayerY0 + boxLayerH } };
    for( auto pxy : rmpd )
    {
        int pn = (int)pxy[ 0 ];
        float x0 = pxy[1];
        float y0 = pxy[2];

        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(x0 + portX + padMargin, y0 + labelH), module, M::RATE_CV + pn ));
        
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(x0, y0+labelH), module, M::RATE_PARAM + pn
#if !RACK_V1
                                                   ,
                                                   0, 1, 0.5
#endif
                                                   ));

        
        addChild(TextDisplayLight::create(rack::Vec(x0 + 2 * portX + 3 * padMargin, y0 + labelH + 2),
                                          rack::Vec(topBoxW - 2 * portX, portY - 4 ),
                                          module ? &(module->pb[pn]->valCache) : nullptr,
                                          12, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, surgeWhite()));
    }
    
    
    /* End CLEAN UI */
    
    
    int XX = SCREW_WIDTH * 15;
    for( int i=7; i<nControls; ++i )
    {
        float yPos = i * controlHeight + topControl;
        int pa = M::RATE_PARAM + i;
        int cv = M::RATE_CV + i;
        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(2 * padMargin + portX , yPos), module, cv ));
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(padMargin , yPos), module,
                                                   pa
#if !RACK_V1
                                                   ,
                                                   0, 1, 0.5
#endif
                                                   ));
        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * portX + 2 , yPos),
                                          rack::Vec(textAreaWidth, controlHeight - padMargin),
                                          module ? &(module->pb[i]->nameCache) : nullptr,
                                          14, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, surgeOrange()));

        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * portX + 2 , yPos),
                                          rack::Vec(textAreaWidth - 2 * padMargin, controlHeight - padMargin),
                                          module ? &(module->pb[i]->valCache) : nullptr,
                                          14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, surgeWhite()));
        
    }

}

#if RACK_V1
rack::Model *modelSurgeLFO =
    rack::createModel<SurgeLFOWidget::M, SurgeLFOWidget>("SurgeLFO");
#else
rack::Model *modelSurgeLFO =
    rack::createModel<SurgeLFOWidget::M, SurgeLFOWidget>(
        "Surge Team", "SurgeLFO", "SurgeLFO", rack::ENVELOPE_GENERATOR_TAG);
#endif
