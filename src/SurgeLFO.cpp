#include "SurgeLFO.hpp"
#include "Surge.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeLFOWidget : rack::ModuleWidget {
    typedef SurgeLFO M;
    SurgeLFOWidget(M *module);


    int sideMargin = 5;
    int topLayer = 20;
    int padMargin = 3;
    int ioMargin = 7;
    int fontId = -1, condensedFontId = -1;

    int modeYPos = 35;

    float topControl = SCREW_WIDTH + padMargin;
    float controlAreaHeight = SurgeLayout::orangeLine - padMargin - topControl;
    int   nControls = M::R_PARAM - M::RATE_PARAM + 1;
    float controlHeight = controlAreaHeight / nControls;
    float textAreaWidth = SCREW_WIDTH * 15 - 4 * padMargin - 2 * SurgeLayout::portX;
    
    float inputXPos(int which) {
        float xSize = box.size.x - 2 * sideMargin;
        float perI = xSize / 3.0;
        float posn = sideMargin + (which + 0.5) * perI - SurgeLayout::portX / 2;
        return posn;
    }

    float inputYPos(int which) {
        return SurgeLayout::orangeLine + ioMargin + padMargin;
    }

    void moduleBackground(NVGcontext *vg) {
        if (fontId < 0)
            fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

        for( int i=0; i<nControls; ++i )
        {
            SurgeStyle::drawTextBGRect(vg, 3*padMargin+2*SurgeLayout::portX, i*controlHeight + topControl,
                                       textAreaWidth, controlHeight-padMargin);
        }
        
    }
};

SurgeLFOWidget::SurgeLFOWidget(SurgeLFOWidget::M *module)
    : rack::ModuleWidget(
#ifndef RACK_V1
          module
#endif
      ) {
#ifdef RACK_V1
    setModule(module);
#endif

    box.size = rack::Vec(SCREW_WIDTH * 15, RACK_HEIGHT);
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

    for( int i=0; i<nControls; ++i )
    {
        float yPos = i * controlHeight + topControl;
        int pa = M::RATE_PARAM + i;
        int cv = M::RATE_CV + i;
        addInput(rack::createInput<rack::PJ301MPort>(
                     rack::Vec(padMargin, yPos), module, cv ));
        addParam(rack::createParam<SurgeSmallKnob>(rack::Vec(2 * padMargin + SurgeLayout::portX, yPos), module,
                                                   pa
#if !RACK_V1
                                                   ,
                                                   0, 1, 0.5
#endif
                                                   ));
        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * SurgeLayout::portX + 2, yPos),
                                          rack::Vec(textAreaWidth, controlHeight - padMargin),
                                          module ? &(module->pb[i]->nameCache) : nullptr,
                                          14, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, SurgeStyle::surgeOrange()));

        addChild(TextDisplayLight::create(rack::Vec(3 * padMargin + 2 * SurgeLayout::portX + 2, yPos),
                                          rack::Vec(textAreaWidth - 2 * padMargin, controlHeight - padMargin),
                                          module ? &(module->pb[i]->valCache) : nullptr,
                                          14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE, SurgeStyle::surgeWhite()));
        
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
