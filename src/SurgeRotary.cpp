#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeRotaryWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<fxt_rotaryspeaker> M;
    SurgeRotaryWidget(M *module);

    int ioRegionWidth = 105;

    float textHeight = 16;
    float xText = 50;
    float ctrlHeight = portY + 3 * padMargin + textHeight + 10;
    float divHeight = 14;
    float yStart = padFromTop + 20;
    float y1Offset = 16 + padMargin + 20;

    void moduleBackground(NVGcontext *vg) {

        float y0 = yStart;

        std::vector<std::string> lab = { "Horn Rate", "Doppler", "Amp Mod" };
        for( int i=0; i<3; ++i )
        {
            if( i == 1 )
            {
                y0 += 20; // These should add up to y1Offset;
                fxGroupLabel( vg, y0, "Depth", box );
                y0 += 16 + padMargin;
            }
            
            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 14);
            nvgFillColor(vg, surgeBlue() );
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE );
            nvgText( vg, padFromEdge, y0 + portY / 2, lab[i].c_str(), NULL );

            float a, d, h;
            nvgTextMetrics( vg, &a, &d, &h );

            dropRightLine( vg, padFromEdge + 10, y0 + portY / 2 + h / 2 + padMargin,
                           xText + padFromEdge, y0 + portY + 1.5 * padMargin + textHeight / 2);
            
            drawTextBGRect( vg, xText, y0 + portY + 1.5 * padMargin, box.size.x - xText - padFromEdge, textHeight );
            y0 += ctrlHeight;
        }
        
        drawStackedInputOutputBackground(vg, box);

    }
};

SurgeRotaryWidget::SurgeRotaryWidget(SurgeRotaryWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 8, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "Rotary");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box, true),
                                                 module, M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box, true),
                                                 module, M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box, true), module,
                                               M::INPUT_GAIN
                                               ));

    addOutput(rack::createOutput<rack::PJ301MPort>(
                  ioPortLocation(false, 0, box, true), module, M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box, true),
                                                   module, M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box, true), module,
                                               M::OUTPUT_GAIN

                                               ));

    for( int i=0; i<3; ++i )
    {
        float yp = yStart + i * ctrlHeight;
        if( i >= 1 )
            yp += y1Offset;
        
        float xp = box.size.x - padFromEdge - padMargin - 2 * portX;
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(xp,yp),
                                                   module, M::FX_PARAM_0 + i
                     ));
        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(xp + portX + padMargin, yp ),
                                                     module, M::FX_PARAM_INPUT_0 + i ) );

        addChild(TextDisplayLight::create(
                     rack::Vec(xText + padMargin, yp + portY + 1.5 * padMargin),
                     rack::Vec(box.size.x - xText - 2 * padMargin, textHeight),
                     module ? &(module->pb[i]->valCache) : nullptr,
                     12,
                     NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE,
                     surgeWhite()
                     ));


    }
    
}

auto mrotary = addFX(
		     rack::createModel<SurgeRotaryWidget::M, SurgeRotaryWidget>("SurgeRotary"), fxt_rotaryspeaker );

