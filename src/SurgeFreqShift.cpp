#include "SurgeFX.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeFreqShiftWidget : SurgeModuleWidgetCommon {
    typedef SurgeFX<fxt_freqshift> M;
    SurgeFreqShiftWidget(M *module);

    int ioRegionWidth = 105;

    float textHeight = 16;
    float xText = 50;
    float ctrlHeight = portY + 3 * padMargin + textHeight;
    float divHeight = 14;
    float yStart = padFromTop + 5;

    void moduleBackground(NVGcontext *vg) {

        float y0 = yStart;

        std::vector<std::string> lab = { "Left Shift", "Right Shift", "Delay", "Feedback", "Mix" };
        for( int i=0; i<5; ++i )
        {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 14);
            nvgFillColor(vg, panelLabel() );
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE );
            nvgText( vg, padFromEdge, y0 + portY / 2, lab[i].c_str(), NULL );

            float a, d, h;
            nvgTextMetrics( vg, &a, &d, &h );

            dropRightLine(vg, padFromEdge + 10, y0 + portY / 2 + h/2 + padMargin,
                          xText + padFromEdge, y0 + portY + 1.5 * padMargin + textHeight / 2 );

            drawTextBGRect( vg, xText, y0 + portY + 1.5 * padMargin, box.size.x - xText - padFromEdge, textHeight );
            y0 += ctrlHeight;
        }
        
        drawStackedInputOutputBackground(vg, box);
    }
};

SurgeFreqShiftWidget::SurgeFreqShiftWidget(SurgeFreqShiftWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 8, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "fShift");
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

    for( int i=0; i<5; ++i )
    {
        float yp = yStart + i * ctrlHeight;
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
                     parameterValueText_KEY()
                     ));


    }
    
}

auto mfreq = addFX(
		   rack::createModel<SurgeFreqShiftWidget::M, SurgeFreqShiftWidget>("SurgeFreqShift"), fxt_freqshift );
