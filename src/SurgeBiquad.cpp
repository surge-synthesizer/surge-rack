#include "SurgeBiquad.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeBiquadWidget : SurgeModuleWidgetCommon {
    typedef SurgeBiquad M;
    SurgeBiquadWidget(M *module);

    float ctrlYStart = 3 * 30 + SCREW_WIDTH + 2 * padMargin;
    float ctrlYHeight = 53;
    float ctrlKnobOff = 45;
    float textOff = 30;
    float textHeight = 17;


    std::vector<std::string> labels = { "Freq", "Reso", "Extra" };
    
    rack::math::Rect ctrlRegion(int i) {
        int x0 = padFromEdge + 10;
        int y0 = ctrlYHeight * i + ctrlYStart + padMargin;
        int xs = box.size.x - 2 * padFromEdge - 20;
        int ys = ctrlYHeight - 3 * padMargin;
        return rack::math::Rect(rack::Vec(x0, y0), rack::Vec(xs, ys));
    }
    
    void moduleBackground(NVGcontext *vg) {
        for( int i=0; i<3; ++i )
        {
            auto b = ctrlRegion(i);

            nvgBeginPath(vg);
            nvgFontFaceId(vg, fontId(vg));
            nvgFontSize(vg, 14);
            nvgStrokeColor(vg, panelLabel());
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT );
            nvgText( vg, b.pos.x, b.pos.y, labels[i].c_str(), NULL );

            int tx0 = b.pos.x + textOff;
            int ty0 = b.pos.y + b.size.y - textHeight;

            dropRightLine(vg, b.pos.x + padFromEdge, b.pos.y + 16, tx0 + 2, ty0 + textHeight / 2.0);
            drawTextBGRect(vg, tx0, ty0, b.size.x - textOff, textHeight );
        }
        drawStackedInputOutputBackground(vg, box);

    }
};

SurgeBiquadWidget::SurgeBiquadWidget(SurgeBiquadWidget::M *module)
    : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(SCREW_WIDTH * 9, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "Biquad");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    float bankW = 3 * 40;
    float bankH = 3 * 30;
    
    SurgeButtonBank *bank = SurgeButtonBank::create( rack::Vec((box.size.x - bankW) / 2.0, SCREW_WIDTH + padMargin),
                                                     rack::Vec(bankW, bankH),
                                                     module, M::FILTER_TYPE,
                                                     3, 3, 13 );
    bank->addLabel( "LP" );
    bank->addLabel( "LP2B" );
    bank->addLabel( "HP" );
    bank->addLabel( "BP" );
    
    bank->addLabel( "BP2A" );
    bank->addLabel( "PKA" );

    bank->addLabel( "NOTCH" );
    bank->addLabel( "peakEQ" );
    bank->addLabel( "APF" );

    addParam(bank);

    for( int i=0; i<3; ++i )
    {
        auto b = ctrlRegion(i);
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(b.pos.x + ctrlKnobOff, b.pos.y),
                                                   module, M::FREQ_KNOB + i));

        addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(b.pos.x + ctrlKnobOff + padMargin + portX, b.pos.y),
                                                     module, M::FREQ_CV + i));

        addChild(TextDisplayLight::create(
                     rack::Vec(b.pos.x + textOff + 2,
                               b.pos.y + b.size.y - textHeight ),
                     rack::Vec(b.size.x - textOff - 4, textHeight ),
                     module ? &(module->pStrings[i] ) : nullptr,
                     12,
                     NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE,
                     parameterValueText()));


    }


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

}

rack::Model *modelSurgeBiquad =
    rack::createModel<SurgeBiquadWidget::M, SurgeBiquadWidget>("SurgeBiquad");
