#include "SurgeADSR.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeADSRWidget : rack::ModuleWidget {
    typedef SurgeADSR M;
    SurgeADSRWidget(M *module);

    int sideMargin = 5;
    int topLayer = 20;
    int padMargin = 3;
    int fontId = -1, condensedFontId = -1;
    int adsrHeight = 37;
    int adsrSpace = 10;
    
    float inputXPos(int which) {
        float xSize = box.size.x - 2 * sideMargin;
        float perI = xSize / 3.0;
        float posn = sideMargin + (which + 0.5) * perI - SurgeLayout::portX/2;
        return posn;
    }

    float inputYPos(int which) {
        return topLayer + padMargin + 12;
    }

    float ADSRYPos(int which) {
        int endOfIn = topLayer + SurgeLayout::portX + 17 + 2 * padMargin;
        return endOfIn + (adsrHeight + adsrSpace) * which + 20;
    }
    
    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        SurgeStyle::drawBlueIORect(vg, sideMargin, topLayer,
                                   box.size.x - 2 * sideMargin,
                                   SurgeLayout::portX + 17 + padMargin);

        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());
        
        nvgBeginPath(vg);
        nvgFillColor(vg, SurgeStyle::surgeWhite() );
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 12);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgText(vg, inputXPos(0) + SurgeLayout::portX/2, topLayer + 1.5, "gate", NULL );

        nvgBeginPath(vg);
        nvgText(vg, inputXPos(1) + SurgeLayout::portX/2, topLayer + 1.5, "retrig", NULL );

        nvgBeginPath(vg);
        nvgText(vg, inputXPos(2) + SurgeLayout::portX/2, topLayer + 1.5, "out", NULL );

        std::vector<std::string> lab = { "A", "D", "S", "R" };
        for( int i=0; i<4; ++i )
        {
            nvgBeginPath(vg);
            nvgRoundedRect(vg, sideMargin, ADSRYPos(i), box.size.x - 2 * sideMargin, adsrHeight, 5);
            nvgStrokeColor(vg, SurgeStyle::surgeOrange2());
            nvgStrokeWidth(vg, 2);
            nvgStroke(vg);

            NVGpaint adsrGradient = nvgLinearGradient(vg,
                                                      sideMargin,
                                                      ADSRYPos(i),
                                                      box.size.x - 2 * sideMargin,
                                                      ADSRYPos(i),
                                                      SurgeStyle::backgroundLightGray(),
                                                      SurgeStyle::backgroundLightOrange());
            nvgFillPaint(vg, adsrGradient);
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId);
            nvgFontSize(vg, 20);
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

            nvgFillColor(vg, SurgeStyle::surgeWhite() );
            nvgText(vg, sideMargin * 2 + 1, ADSRYPos(i) + adsrHeight/2 + 1, lab[i].c_str(), NULL );

            nvgFillColor(vg, SurgeStyle::surgeOrange2() );
            nvgText(vg, sideMargin * 2, ADSRYPos(i) + adsrHeight/2, lab[i].c_str(), NULL );
        }

        nvgBeginPath(vg);
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 15);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
        nvgFillColor(vg, SurgeStyle::surgeOrange2() );
        nvgText(vg, sideMargin * 2, 295, "Analog/Digital", NULL );

    }
};

SurgeADSRWidget::SurgeADSRWidget(SurgeADSRWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 8, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "ADSR");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(inputXPos(0),inputYPos(0)),
                                                 module, M::GATE_IN));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(inputXPos(1),inputYPos(1)),
                                                     module, M::RETRIG_IN));
    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(inputXPos(2),inputYPos(2)),
                                                   module, M::OUTPUT_ENV));

    int envStart = topLayer + 24.6 * 2 + 15 + topLayer + 30;
    int envHeight = 40;
    int x0 = 30;

    for( int i=M::A_PARAM; i<= M::R_PARAM; ++i )
    {
        int ipos = i - M::A_PARAM;
        addParam( rack::createParam<SurgeSmallKnob>( rack::Vec(x0+30, ADSRYPos(ipos) + (adsrHeight-SurgeLayout::surgeKnobY)/2 ), module, i
#if !RACK_V1
                                                     ,0,1,0.5
#endif                                                     

                      ) );
    }

    for( int i=M::A_CV; i<= M::R_CV; ++i )
    {
        int ipos = i - M::A_CV;
        addInput( rack::createInput<rack::PJ301MPort>( rack::Vec(x0, ADSRYPos(ipos) + (adsrHeight-SurgeLayout::portY)/2 ), module, i ) );
    }

    for( int i=M::A_S_PARAM; i<=M::R_S_PARAM; ++i)
    {
        int ipos = i - M::A_S_PARAM;
        if( i == M::R_S_PARAM)
            ipos++;
        
        addParam(rack::createParam<rack::CKSSThree>( rack::Vec( x0+60, ADSRYPos(ipos) + (adsrHeight-28)/2.0 ) , module, i
#if !RACK_V1
                                                     ,0,2,0
#endif                                                     

                     ));
    }

    addParam(rack::createParam<SurgeSwitchFull>(rack::Vec(box.size.x - sideMargin - 20, 290), module, M::MODE_PARAM
#if !RACK_V1
                                                     ,0,1,0
#endif                                                     
                 ));
    
}

#if RACK_V1
rack::Model *modelSurgeADSR =
    rack::createModel<SurgeADSRWidget::M, SurgeADSRWidget>("SurgeADSR");
#else
rack::Model *modelSurgeADSR = rack::createModel<SurgeADSRWidget::M, SurgeADSRWidget>(
    "Surge Team", "SurgeADSR", "SurgeADSR", rack::ENVELOPE_GENERATOR_TAG);
#endif
