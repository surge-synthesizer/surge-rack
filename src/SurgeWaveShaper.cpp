#include "SurgeWaveShaper.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeWaveShaperWidget : rack::ModuleWidget {
    typedef SurgeWaveShaper M;
    SurgeWaveShaperWidget(M *module);

    int roostery0 = 53;
    int textArea = 20;
    int labelHeight = 13;
    int padMargin = 3;
    int textMargin = 6;

    int fontId = -1;
    
    void addLabel(NVGcontext *vg, int yp, const char* label, NVGcolor col = SurgeStyle::surgeBlue()) {
        if( fontId < 0 )
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());
        
        nvgBeginPath(vg);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
        nvgFontFaceId(vg, fontId);
        nvgFontSize(vg, 12);
        nvgFillColor(vg, col);
        nvgText(vg, box.size.x / 2, yp, label, NULL);
    }
    void moduleBackground(NVGcontext *vg) {
        int yPos = roostery0 - labelHeight;
        addLabel(vg, yPos, "Type" );
        yPos += labelHeight + SurgeLayout::surgeRoosterY + padMargin;
        SurgeStyle::drawTextBGRect(vg, textMargin, yPos, box.size.x - 2 * textMargin, textArea);
        yPos += textArea + padMargin;
        addLabel(vg, yPos, "Drive" );

        yPos += labelHeight + SurgeLayout::surgeRoosterY + padMargin + SurgeLayout::portY + 4*padMargin;
        SurgeStyle::drawTextBGRect(vg, textMargin, yPos, box.size.x - 2 * textMargin, textArea);
        yPos += padMargin;

        yPos += textArea + padMargin;
        int sz = SurgeLayout::portX / 2 + 2 * padMargin;
        
        SurgeStyle::drawBlueIORect(vg, box.size.x / 2 - sz, yPos, sz * 2, 2 * SurgeLayout::portY + 2 * labelHeight + 4 * padMargin );
        yPos += padMargin;
        addLabel(vg, yPos, "Input", SurgeStyle::surgeWhite() );
        yPos += labelHeight + SurgeLayout::portY + padMargin;
        addLabel(vg, yPos, "Output", SurgeStyle::surgeWhite() );
    }
};

SurgeWaveShaperWidget::SurgeWaveShaperWidget(SurgeWaveShaperWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 5, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "WS");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) {
        this->moduleBackground(vg);
    };
    addChild(bg);

    int yPos = roostery0;
    
    addParam(rack::createParam<SurgeKnobRooster>(rack::Vec(box.size.x / 2 - SurgeLayout::surgeRoosterX / 2, yPos),
                                                 module, M::MODE_PARAM
#if! RACK_V1
                                                 ,0,n_ws_type-1,0
#endif
                 ));

    yPos += SurgeLayout::surgeRoosterY +  padMargin;

    addChild(TextDisplayLight::create(
                 rack::Vec(textMargin, yPos),
                 rack::Vec(box.size.x - 2 * textMargin, textArea),
                 module ? module->wsNameCache.getValue : []() { return std::string("null"); },
                 module ? module->wsNameCache.getDirty : []() { return false; },
                 16, NVG_ALIGN_TOP | NVG_ALIGN_CENTER ) );

    
    yPos += textArea + padMargin;

    yPos += labelHeight + padMargin;
    addParam(rack::createParam<SurgeKnobRooster>(rack::Vec(box.size.x / 2 - SurgeLayout::surgeRoosterX / 2,
                                                           yPos),
                                                 module, M::DRIVE_PARAM
#if! RACK_V1
                                                 ,-24.0, 0, 24.0
#endif
                 ));
    yPos += SurgeLayout::surgeRoosterY + 2 * padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(box.size.x / 2 - SurgeLayout::portX / 2,
                                                           yPos),
                                                 module, M::DRIVE_CV ) );

    yPos += SurgeLayout::portY + padMargin;

    yPos += textArea + padMargin;
    yPos += padMargin;

    yPos += labelHeight + 2 * padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(box.size.x / 2 - SurgeLayout::portX / 2, yPos ),
                                                module, M::SIGNAL_IN ));
    yPos += SurgeLayout::portY + labelHeight + 2 * padMargin;
    addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(box.size.x / 2 - SurgeLayout::portX / 2, yPos ),
                                                  module, M::SIGNAL_OUT ));
    
}

#if RACK_V1
rack::Model *modelSurgeWaveShaper =
    rack::createModel<SurgeWaveShaperWidget::M, SurgeWaveShaperWidget>("SurgeWaveShaper");
#else
rack::Model *modelSurgeWaveShaper = rack::createModel<SurgeWaveShaperWidget::M, SurgeWaveShaperWidget>(
    "Surge Team", "SurgeWaveShaper", "SurgeWaveShaper", rack::ENVELOPE_GENERATOR_TAG);
#endif
