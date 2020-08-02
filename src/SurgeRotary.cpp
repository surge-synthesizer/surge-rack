#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeRotaryWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<fxt_rotaryspeaker> M;
    SurgeRotaryWidget(M *module);

    int ioRegionWidth = 105;

    float textHeight = 16;
    float xText = 22;
    float ctrlHeight = portY + 3 * padMargin + textHeight + 18;
    float divHeight = 14;
    float yStart = padFromTop + 17;
    float y1Offset = 16 + padMargin + 10;
    float clockY = orangeLine - portX  - 3 * padMargin;

    void moduleBackground(NVGcontext *vg) {

        float y0 = yStart;

        clockBackground(vg, box.size.x / 2 - clockW / 2, clockY);
        
        std::vector<std::string> lab = { "Horn", "Rotor",
                                         "Gain", "Model",
                                         "Doppler", "Tremolo",
                                         "Width", "Mix" };
        std::vector<std::string> sect = { "Rate", "Drive", "Depth", "Output" };

        for( int i=0; i<8; ++i )
        {
            float labelY = y0 + portY / 2;
            int xp = padFromEdge;
            if( i % 2 )
                xp += box.size.x / 2 + padFromEdge;
            int w = box.size.x / 2 - xText - padFromEdge;

            if( i % 2 == 0 )
            {
                fxGroupLabel( vg, labelY - 28, sect[i/2].c_str(), box );
            }
            
            nvgBeginPath(vg);
            nvgSave(vg);
            nvgTranslate(vg, xp, labelY );
            nvgRotate( vg, M_PI/2.0 );
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 14);
            nvgFillColor(vg, panelLabel() );
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM );
            nvgText( vg, 0, 0, lab[i].c_str(), NULL );
            nvgRestore(vg);
            
            float a, d, h;
            //nvgTextMetrics( vg, &a, &d, &h );

            //dropRightLine( vg, xp + 10, labelY + h / 2 + padMargin,
            //xText + xp, y0 + portY + 1.5 * padMargin + textHeight / 2);

            if( i % 2 ) xp -= padMargin;
            drawTextBGRect( vg, xp + xText - padFromEdge, y0 + portY + 1.5 * padMargin, w, textHeight );
            if( i % 2 )
                y0 += ctrlHeight;
        }
        
        drawLeftRightInputOutputBackground(vg, box);

    }
};

SurgeRotaryWidget::SurgeRotaryWidget(SurgeRotaryWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 16, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "Rotary");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    bool stacked = false;
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box, stacked),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box, stacked),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box, stacked), module,
                                               M::INPUT_GAIN
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box, stacked), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box, stacked),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box, stacked), module,
                                               M::OUTPUT_GAIN

                                               ));

    for( int i=0; i<8; ++i )
    {
        float yp = yStart + ((int)(i / 2)) * ctrlHeight;

        float xp;
        if( i % 2 == 0 )
            xp = box.size.x / 2 - padFromEdge - padMargin - 2 * portX;
        else
            xp = box.size.x  - padFromEdge - padMargin - 2 * portX;

        xp -= 12;
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(xp,yp),
                                                   module, M::FX_PARAM_0 + i
                     ));
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xp + portX + padMargin, yp ),
                                                     module, M::FX_PARAM_INPUT_0 + i ) );

        if( i == 0 )
        {
            addParam(rack::createParam<SurgeSwitch>(rack::Vec( xp + portX * 2 + padMargin * 2, yp + 3),
                                                    module, M::PARAM_TEMPOSYNC_0 ) );
            if( module && module->pb[M::FX_PARAM_0] != nullptr )
            {
                module->pb[M::FX_PARAM_0]->tsbpmLabel = false;
            }
        }

        float tp = xText + padMargin;
        if( i % 2 ) tp += box.size.x/2;
        addChild(TextDisplayLight::create(
                     rack::Vec(tp, yp + portY + 1.5 * padMargin),
                     rack::Vec(box.size.x/2 - xText - 2 * padMargin, textHeight),
                     module ? &(module->pb[i]->valCache) : nullptr,
                     12,
                     NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE,
                     parameterValueText_KEY()
                     ));


    }

    addInput(rack::createInput<rack::PJ301MPort>(
                 rack::Vec(box.size.x/2 - clockW/2  + padMargin, clockY + padMargin),
                 module, M::CLOCK_CV_INPUT));

}

auto mrotary = addFX(
		     rack::createModel<SurgeRotaryWidget::M, SurgeRotaryWidget>("SurgeRotary"), fxt_rotaryspeaker );

