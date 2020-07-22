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
    float textAreaWidth = SCREW_WIDTH * 15 - 4 * padMargin - 2 * portX - 20;

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

    float topBoxW = SCREW_WIDTH * 6;
    float boxLayerY0 = SCREW_WIDTH + padMargin * 2 + buttonBankHeight;
    float boxLayerH = 50;
    float labelH = 16;
    float UIW = SCREW_WIDTH * 18;
    float buttonBankW = SCREW_WIDTH * 15;

    float envPos = 6 * controlHeight + topControl + 5;;

    std::vector<std::string> envlabel = { "Delay", "Att", "Hold", "Decay", "Sus", "Rel" };
    std::vector<int> elOff = { 0, 2, 1, 3, 4, 5, 6 };

    rack::Vec envPoint(int i) {
        int col = i % 2;
        int row = i / 2;

        float segHeight = ( orangeLine - envPos - 15 - padMargin ) / 3.0;
        
        float xp = (col == 0 ? padFromEdge : box.size.x / 2.0 + padMargin) + 5;
        float yp = segHeight * row + envPos + 15 + padMargin;
        return rack::Vec(xp,yp);
    }
    
    rack::Vec envSize(int i) {
        return rack::Vec( ( box.size.x - 2 * padFromEdge - 2 * padMargin ) / 2.0 - 10,
                          ( orangeLine - envPos - 15 - padMargin ) / 3.0 - padMargin );
    }

    float envXOff = 20;

    void moduleBackground(NVGcontext *vg) {
        float ioyb = orangeLine + ioMargin;
        auto xb0 = inputXPos(0) - padMargin;
        drawBlueIORect(vg, xb0, ioyb, 3 * portX + 6 * padMargin, box.size.y - orangeLine - 2 * ioMargin );
        std::vector<std::string> lab = { "Gate", "", "Out" };
        for( int i=0; i<3; ++i )
        {
            auto xc = inputXPos(i) + portX / 2;
            auto yc = box.size.y - ioMargin - 1.5;
            nvgBeginPath(vg);
            nvgFontFaceId(vg,fontId(vg));
            nvgFontSize(vg, 12);
            nvgFillColor(vg, ioRegionText());
            nvgTextAlign(vg,NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgText(vg, xc, yc, lab[i].c_str(), NULL );
        }

        nvgBeginPath( vg );
        nvgFontFaceId(vg, fontId(vg));
        nvgFontSize(vg, 11);
        nvgFillColor(vg, panelLabel() );
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgText( vg, UIW - SCREW_WIDTH * 1.2, SCREW_WIDTH + padMargin, "uni", NULL );
        nvgText( vg, UIW - SCREW_WIDTH * 1.2, SCREW_WIDTH + padMargin + 11, "bi", NULL );

        // Rate section spans
        centerRuledLabel( vg, padFromEdge, boxLayerY0, box.size.x - 2 * padFromEdge, "Rate" );
        drawTextBGRect(vg, box.size.x / 2 + padMargin, boxLayerY0 + labelH + 2, box.size.x/2 - 2 * padMargin, portY - 2 );

        clockBackground(vg, padFromEdge, boxLayerY0 + labelH);
        
        
        // Below it we get these guys in threes
        std::vector<std::string> l = { "Magnitude", "Phase", "Deform" };
        for( int i=0; i<3; ++i )
        {
            int xpos = padFromEdge + i * box.size.x / 3;
            int ypos = boxLayerY0 + boxLayerH;
            centerRuledLabel( vg, xpos, ypos, topBoxW - 2 * padFromEdge, l[i].c_str() );
            drawTextBGRect(vg, xpos, ypos + labelH + portY + 2*padMargin, box.size.x/3 - 2 * padFromEdge, 14 );
        }
        

        centerRuledLabel( vg, padFromEdge, envPos, UIW - 2 * padFromEdge, "Envelope" );

        for( int i=0; i<6; ++i )
        {
            rack::Vec p = envPoint(i);
            rack::Vec s = envSize(i);

            // OK so rotated text
            nvgBeginPath(vg);
            nvgSave(vg);
            nvgTranslate(vg, p.x + padMargin + 14, p.y );
            nvgRotate(vg, M_PI / 2.0 );
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 13);
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT );
            nvgFillColor(vg, panelLabel() );
            nvgText( vg, 0, 0, envlabel[i].c_str(), NULL );

            float bounds[4];
            nvgTextBounds( vg, 0, 0, envlabel[i].c_str(), NULL, bounds );
            nvgRestore(vg);

            // so a bit tricky
            float linesx = p.x + padMargin + 14 - bounds[3] / 2.0;
            float linesy  = p.y + bounds[2] + 2;

            float rx = p.x + envXOff;
            float ry = p.y + padMargin + portY;
            float sx = s.x - envXOff; //s.x - rx - padMargin;
            float sy = s.y - padMargin - portY; // s.y - ry - padMargin;

            dropRightLine(vg, linesx, linesy, rx + 2, ry + sy / 2 );
            drawTextBGRect(vg, rx, ry, sx, sy );
        }
        
        
    }
};

SurgeLFOWidget::SurgeLFOWidget(SurgeLFOWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 18, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "LFO");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(
        rack::Vec(inputXPos(0), inputYPos(0)), module, M::GATE_IN));
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
                 ));
    
    // Vector is param #, x, y
    // Add the rate inputs
    float rateTop = boxLayerY0 + labelH + 2;
    float xPos = padFromEdge + padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec(xPos, rateTop), module, M::CLOCK_CV_INPUT ) );
    xPos += portX + padMargin + clockPad + 18;
    addParam(rack::createParam<SurgeSmallKnob>(
                 rack::Vec(xPos, rateTop), module, M::RATE_PARAM ) );
    xPos += portX + padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec(xPos, rateTop), module, M::RATE_CV ) );

    xPos += portX + padMargin;
    addParam(rack::createParam<SurgeSwitch>(rack::Vec(xPos, rateTop + 2), module,
                                            M::RATE_TS));

    addChild(TextDisplayLight::create(rack::Vec( box.size.x / 2 + 2 * padMargin, rateTop ),
                                      rack::Vec( box.size.x / 2 - 3 * padMargin, portY-2 ),
                                      module ? &(module->pb[0]->valCache) : nullptr,
                                      14, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, parameterValueText_KEY() ) );
    
    
    std::vector<float> mpd = { 3, 2, 4 };
    for( int i=0; i<3; ++i )
    {
        int xpos = padFromEdge + i * box.size.x / 3;
        float xc0 = xpos + box.size.x / 6 - padFromEdge - portX - padMargin/2;
        int ypos = boxLayerY0 + boxLayerH;

        ypos += labelH + 2;

        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(xc0, ypos), module, M::RATE_PARAM + mpd[i]));
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xc0 + portX + padMargin, ypos), module, M::RATE_CV + mpd[i] ));

        
        addChild(TextDisplayLight::create(rack::Vec(xpos, ypos - 2 + portY + 2*padMargin ),
                                          rack::Vec(box.size.x/3 - 2 * padFromEdge, 14 ),
                                          module ? &(module->pb[mpd[i]]->valCache) : nullptr,
                                          12, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, parameterValueText_KEY()));
    }

    addChild(rack::createLight<rack::SmallLight<rack::BlueLight>>(rack::Vec(box.size.x/2 + 25, envPos),
                                                                  module,
                                                                  M::ENV_LIGHT));
    
    /* End CLEAN UI */
    for( int i=7; i<nControls; ++i )
    {
        int idx = 7 + elOff[i-7];
        int pa = M::RATE_PARAM + idx;
        int cv = M::RATE_CV + idx;
        int ts = M::DEL_TS + elOff[i-7] - (pa >= M::S_PARAM ? 1 : 0 );

        rack::Vec ep = envPoint(i-7);
        float xstart = ep.x + envXOff;
        
        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(xstart + 2* padMargin + portX, ep.y), module, cv ));
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(xstart + padMargin, ep.y), module,
                                                   pa
                                                   ));
        if( pa != M::S_PARAM )
        {
            addParam(rack::createParam<SurgeSwitch>(rack::Vec(xstart + 2 * portX + 3 * padMargin, ep.y), module,
                                                    ts));
        }

        addChild(TextDisplayLight::create(rack::Vec(xstart + padMargin, ep.y + portY + padMargin  ),
                                          rack::Vec(textAreaWidth - 2 * padMargin, controlHeight - padMargin),
                                          module ? &(module->pb[idx]->valCache) : nullptr,
                                          14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, parameterValueText_KEY()));
    }
}

rack::Model *modelSurgeLFO =
    rack::createModel<SurgeLFOWidget::M, SurgeLFOWidget>("SurgeLFO");
