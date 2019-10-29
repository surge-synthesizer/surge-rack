#include "SurgePatchPlayer.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgePatchPlayerWidget : public virtual SurgeModuleWidgetCommon {
    typedef SurgePatchPlayer M;
    SurgePatchPlayerWidget(M *module);

    int ioRegionWidth = 105;

    float pitchY = SCREW_WIDTH + padFromEdge;
    float infoPos0 = pitchY + 25 + padMargin;
    
    float pitchCtrlX = padFromEdge + 2 * padMargin + 2 * portX;
    
    float controlsY = pitchY + 2 * padMargin + surgeRoosterY + portY;
    float controlsHeight = orangeLine - controlsY - padMargin;
    float controlHeightPer = controlsHeight / n_osc_params;

    float colOneEnd = surgeRoosterX + portX + 2 * padMargin + 14 + pitchCtrlX + padFromEdge;
    float patchSelY = pitchY + 113;

    float catitlodsz = ( SCREW_WIDTH * 18 - colOneEnd ) / 3;

    typedef std::tuple<int,int,int,std::string> ctrlPos_t;
    std::vector<ctrlPos_t> ctrlPositions;

    void setupCtrlPositions() {
        auto acp = [this](int r, int c, int p, std::string v )
            {
                this->ctrlPositions.push_back(ctrlPos_t(r,c,p,v));
            };
        acp(1,1,M::PITCH_CV, "note");
        acp(1,2,M::VEL_CV, "vel");
        acp(1,3,M::GATE_CV, "gate");
        
        acp(1,5,M::PITCHBEND_CV, "bend" );
        acp(1,6,M::MODWHEEL_CV, "mod" );
        acp(1,7,M::CHAN_AT_CV, "chan at" );
        
        acp(2,6,M::POLY_AT_CV, "poly at" );
        acp(2,7,M::TIMBRE_CV, "timbre" );
        
        for( int i=0; i<8; ++i )
        {
            char txt[2];
            txt[0] = '0' + i;
            txt[1] = 0;
            acp(i < 4 ? 2 : 3,i % 4 + 1,M::CONTROL_0_CV+i,txt);
        }
    }

    
    void moduleBackground(NVGcontext *vg) {
        // The input triggers and output
        nvgBeginPath(vg);

        drawLeftRightInputOutputBackground(vg, box, "Output", false, true);
        drawLeftRightInputOutputBackground(vg, box, "Input", true, true);

        drawBlueIORect(vg, box.size.x/2 - portX / 2 - padMargin, orangeLine + ioMargin,
                       portX + 2 * padMargin, box.size.y - orangeLine - 2 * ioMargin );
        nvgBeginPath(vg);
        nvgFillColor(vg, ioRegionText() );
        nvgFontFaceId(vg, fontId(vg) );
        nvgFontSize(vg, 11);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
        nvgText(vg, box.size.x/2, box.size.y - ioMargin - 1.5, "clock cv", NULL );

        auto xTPos = padFromEdge + surgeRoosterX + padMargin;
        auto yTPos = orangeLine - ( 5 * ( 14 + padMargin ) + padMargin + 2 + padMargin + 19 );

        auto inputRegionSize = yTPos - ( SCREW_WIDTH + padMargin + 50 ) - padMargin;
        auto inputSingleSize = inputRegionSize / 3;
        for( auto tup : ctrlPositions )
        {
            auto xp = std::get<1>(tup);
            auto yp = std::get<0>(tup);
            auto lb = std::get<3>(tup);

            float xc = padMargin + ( 10 + portX + padMargin ) * (xp-1);
            float yc = SCREW_WIDTH + 50 + 2 * padMargin + (yp-1) * inputSingleSize;
            float w = 10 + portX;
            float h = inputSingleSize - padMargin;

            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 12);
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER );
            nvgFillColor( vg, panelLabel());
            nvgText( vg, xc + w/2, yc, lb.c_str(), NULL );

        }
        
        drawTextBGRect(vg, padFromEdge, SCREW_WIDTH + padMargin, box.size.x - 2 * padFromEdge, 50 );
        
        drawTextBGRect(vg, xTPos, yTPos, box.size.x - padFromEdge - xTPos, 19 );
        yTPos += 19 + padMargin;
        drawTextBGRect(vg, xTPos, yTPos, box.size.x - padFromEdge - xTPos, orangeLine - yTPos - padMargin );
        
        
    }
};

SurgePatchPlayerWidget::SurgePatchPlayerWidget(SurgePatchPlayerWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);
    setupCtrlPositions();
    
    box.size = rack::Vec(SCREW_WIDTH * 18, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "PatchPlayer");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true,0,box), module,
                                                   M::INPUT_L));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true,1,box), module,
                                                   M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true,2,box), module,
                                               M::INPUT_GAIN
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false,0,box), module,
                                                   M::OUTPUT_L));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false,1,box), module,
                                                   M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false,2,box), module,
                                               M::OUTPUT_GAIN
                                               ));

    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec(box.size.x/2 - portX/2, orangeLine + 1.5 * ioMargin),
                 module, M::CLOCK_CV));


    addChild(TextDisplayLight::create(
                 rack::Vec(padMargin, SCREW_WIDTH + padMargin ),
                 rack::Vec(box.size.x - 2 * padMargin, 15 ),
                 module ? &(module->patchInfoCache[0]) : nullptr,
                 13, NVG_ALIGN_TOP| NVG_ALIGN_CENTER, parameterValueText_KEY() ));

    addChild(TextDisplayLight::create(
                 rack::Vec(padMargin, SCREW_WIDTH + padMargin  + 13 ),
                 rack::Vec(box.size.x - 2 * padMargin, 25 ),
                 module ? &(module->patchInfoCache[1]) : nullptr,
                 20, NVG_ALIGN_TOP| NVG_ALIGN_CENTER, parameterValueText_KEY() ));

    addChild(TextDisplayLight::create(
                 rack::Vec(padMargin, SCREW_WIDTH + padMargin  + 13 + 2 + 20 ),
                 rack::Vec(box.size.x - 2 * padMargin, 15 ),
                 module ? &(module->patchInfoCache[2]) : nullptr,
                 11, NVG_ALIGN_TOP| NVG_ALIGN_CENTER, parameterValueText_KEY() ));

    
    auto xpf = [this](int i) { return this->colOneEnd + this->catitlodsz * ( i + 0.5 ) - this->surgeRoosterX / 2.0; };
    

    auto xTPos = padFromEdge + surgeRoosterX + padMargin;
    auto yTPos = orangeLine - ( 5 * ( 14 + padMargin ) + padMargin + 2 + padMargin + 19 );

    auto inputRegionSize = yTPos - ( SCREW_WIDTH + padMargin + 50 ) - padMargin;
    auto inputSingleSize = inputRegionSize / 3;
    for( auto tup : ctrlPositions )
    {
        auto xp = std::get<1>(tup);
        auto yp = std::get<0>(tup);
        auto ct = std::get<2>(tup);
        
        float xc = padMargin + ( 10 + portX + padMargin ) * (xp-1);
        float yc = SCREW_WIDTH + 50 + 2 * padMargin + (yp-1) * inputSingleSize;
        float w = 10 + portX;
        float h = inputSingleSize - padMargin;
        
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xc + w/2 - portX/2, yc + 14), module, ct ));
    }


    addChild(TextDisplayLight::create(
                 rack::Vec(xTPos + 4, yTPos),
                 rack::Vec(box.size.x - xTPos - 2 * padMargin, 19 ),
                 module ? &(module->patchCategoryName) : nullptr,
                 15, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, parameterValueText_KEY()));

    for( int i=0; i<3; ++i )
    {
        float xlp = padFromEdge;
        float ylp = yTPos + i * (2 * padMargin + surgeRoosterY ) - 3;
        switch( i ) {
        case 0:
            addParam(rack::createParam<SurgeKnobRooster>(
                         rack::Vec(xlp, ylp), module, M::CATEGORY_IDX
                         ));
            break;
        case 1:
            addParam(rack::createParam<SurgeKnobRooster>(
                         rack::Vec(xlp, ylp),
                         module, M::PATCH_IN_CATEGORY_IDX
                         ));
            break;
        case 2:
            addParam(rack::createParam<rack::CKD6>(
                         rack::Vec(xlp, ylp ),
                         module, M::LOAD_PATCH
                         ));
            addChild(rack::createLight<rack::SmallLight<rack::RedLight>>(
                         rack::Vec(xlp + 27, ylp + 1 ),
                         module, M::NEEDS_LOAD
                         ));

            break;
        }
    }

    for( int i=1; i<6; ++i )
    {
        if( i == 1 ) yTPos += 22 + padMargin;
        else if( i == 4 ) yTPos += 16 + padMargin;
        else yTPos += 14 + padMargin;

        auto sc = SurgeStyle::parameterScrollCenter();
        auto se = SurgeStyle::parameterScrollEnd();

        float distance = 1.0;
        switch(i) {
        case 0:
        case 6:
            distance = 0;
        case 1:
        case 5:
            distance = 0.25;
        case 2:
        case 4:
            distance = 0.5;
        }

        float fs = 15 - ( i - 3 ) * ( i - 3 ) / 3.0;
        float colR = 255 * (distance * sc.r + ( 1-distance)*se.r);
        float colG = 255 * (distance * sc.g + ( 1-distance)*se.g);
        float colB = 255 * (distance * sc.b + ( 1-distance)*se.b);
        NVGcolor col = nvgRGB(colR, colG, colB );

        auto c = TextDisplayLight::create(
                     rack::Vec(xTPos + 4, yTPos),
                     rack::Vec(box.size.x - xTPos - 2 * padMargin, (i == 3 ? 16 : 14) ),
                     module ? &(module->patchItemName[i]) : nullptr,
                     fs, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
        c->colorKey = "";
        c->color = col;
        addChild(c);
    }
 
}

rack::Model *modelSurgePatchPlayer =
    rack::createModel<SurgePatchPlayerWidget::M, SurgePatchPlayerWidget>("SurgePatchPlayer");
