#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeEQWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<fxt_eq> M;
    SurgeEQWidget(M *module);

    int SCREW_COUNT=16;
    float bandRegion = SCREW_WIDTH * SCREW_COUNT / 3.0;
    int nBands = 3;
    int bandY0 = SCREW_WIDTH + padFromEdge + 15;
    float labelHeight = 13;
    float textHeight = 18;
    float bandCHeight = portY + 3 * padMargin + textHeight + labelHeight + 5;
    float bandLOffset = 18;

    float masterGainY = bandCHeight * 3 + bandY0 + padMargin * 2 + bandLOffset;
    
    void moduleBackground(NVGcontext *vg) {
        drawLeftRightInputOutputBackground(vg, box);

        std::vector<std::string> lab = { "Gain", "Freq", "BW" };
        for( int i=0; i<nBands; ++i )
        {
            char txt[32];
            snprintf(txt, 32, "Band %d", i + 1 );
            centerRuledLabel(vg, bandRegion * i + padFromEdge, bandY0, bandRegion - 2 * padFromEdge, txt );

            if( i != nBands )
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, bandRegion * (i+1), bandY0 + bandLOffset);
                nvgLineTo(vg, bandRegion * (i+1), bandY0 + 3 * bandCHeight + bandLOffset - 10 );
                nvgStrokeColor(vg, backgroundDarkGray());
                nvgStrokeWidth(vg, 1 );
                nvgStroke(vg);
            }

            for( int j=0; j<3; ++j )
            {
                float yPos = bandY0 + bandLOffset + j * bandCHeight;
                nvgBeginPath(vg);
                nvgFontFaceId(vg, fontId(vg));
                nvgFontSize(vg,12);
                nvgStrokeColor(vg,surgeBlue());
                nvgText(vg, bandRegion * ( i + 0.5 ), yPos, lab[j].c_str(), NULL );

                yPos += labelHeight + padMargin + portY + padMargin;
                drawTextBGRect(vg, i * bandRegion + padFromEdge, yPos, bandRegion - 2 * padFromEdge, textHeight );
            }
        }
        
        centerRuledLabel(vg, box.size.x / 4, masterGainY, box.size.x/2, "Master Gain" );
        drawTextBGRect( vg, box.size.x/4 + 2 * portX + 2 * padMargin, masterGainY + labelHeight + padMargin + ( portY-textHeight)/2,
                        box.size.x/2 - 2 * portX - 2 * padMargin, textHeight );

    }
};

SurgeEQWidget::SurgeEQWidget(SurgeEQWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * SCREW_COUNT, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "EQ");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box), module,
                                               M::INPUT_GAIN));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box), module,
                                               M::OUTPUT_GAIN));

    for( int i=0; i<nBands; ++i )
    {
        for( int j=0; j<3; ++j )
        {
            int parImd = i * 3 + j;
            int pa = M::FX_PARAM_0 + parImd;
            int ca = M::FX_PARAM_INPUT_0 + parImd;

            float xPosCtr = bandRegion * ( i + 0.5 );
            float yPos = bandY0 + bandLOffset + j * bandCHeight + labelHeight + padMargin;

            addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(xPosCtr - portY - padMargin / 2, yPos),
                                                       module, pa ));
            addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xPosCtr + padMargin / 2, yPos),
                                                         module, pa ));

            yPos += portY + padMargin;
            addChild(TextDisplayLight::create(rack::Vec( i * bandRegion + padFromEdge, yPos ),
                                              rack::Vec( bandRegion - 2 * padFromEdge, textHeight ),
                                              module ? &(module->pb[parImd]->valCache) : nullptr,
                                              12, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, surgeWhite() ) );

        }
    }

    addParam(rack::createParam<SurgeSmallKnob>(rack::Vec( box.size.x / 4, masterGainY + labelHeight + padMargin),
                                               module, M::FX_PARAM_0 + 9 ));
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec( box.size.x / 4 + portX + padMargin, masterGainY + labelHeight + padMargin),
                                                 module, M::FX_PARAM_INPUT_0 + 9 ));
    addChild(TextDisplayLight::create( rack::Vec( box.size.x/4 + 2 * portX + 3 * padMargin,
                                                  masterGainY + labelHeight + padMargin + ( portY-textHeight)/2),
                                       rack::Vec(box.size.x/2 - 2 * portX - 3 * padMargin, textHeight ),
                                       module ? &(module->pb[9]->valCache) : nullptr,
                                       12, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT, surgeWhite() ) );

    
}

auto meq = addFX(
    rack::createModel<SurgeEQWidget::M, SurgeEQWidget>("SurgeEQ"), fxt_eq );
