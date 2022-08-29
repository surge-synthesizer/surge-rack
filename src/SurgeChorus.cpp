#include "Surge.hpp"
#include "SurgeFX.hpp"
#include "SurgeRackGUI.hpp"

struct SurgeChorusWidget : SurgeModuleWidgetCommon
{
    typedef SurgeFX<fxt_chorus4> M;
    SurgeChorusWidget(M *module);

    float boxw = SCREW_WIDTH * 13;

    float labelHeight = 16;
    float lowerCtrlH = (portY + 2 * padMargin + 18);
    float lowerCtrlOff = 18;
    float textHeight = 15;
    float upperCtrlH = (3 * portY + 6 * padMargin + 13);
    float upperTextWidth = 3.8 * SCREW_WIDTH;
    std::vector<float> regionCtrlH = {upperCtrlH, lowerCtrlH, lowerCtrlH};
    std::vector<std::string> regionLabels = {"Modulation", "EQ Cutoff", "Output"};
    std::vector<int> paramId = {1, 2, 3, 0, 4, 5, 6, 7};
    std::vector<std::string> labels = {"Rate", "Depth", "F/B",   "Delay",
                                       "Low",  "Hi",    "Width", "Mix"};

    float regionStartY(int r)
    {
        float ch = 0;
        for (int i = 0; i < r; ++i)
            ch += regionCtrlH[i] + labelHeight;
        ch += SCREW_WIDTH + padFromEdge;
        return ch;
    }

    rack::math::Rect controlBox(int c)
    {
        if (c < 2)
        {
            float yp = regionStartY(0) + labelHeight;

            yp += c * (portY + padMargin);
            float xp = padFromEdge + portX + 3 * padMargin;
            float ys = portY + padMargin;
            float xs = boxw - 2 * padFromEdge - portX - 3 * padMargin;

            return rack::math::Rect(rack::Vec(xp, yp), rack::Vec(xs, ys));
        }
        else if (c == 2 || c == 3)
        {
            float yp = regionStartY(0) + labelHeight + 3 * padMargin + 2 * portY;
            float xp = (c == 2 ? padFromEdge : boxw / 2.0 + padMargin);
            float xs = boxw / 2.0 - padMargin - padFromEdge;
            float ys = portY + padMargin + textHeight;

            return rack::math::Rect(rack::Vec(xp, yp), rack::Vec(xs, ys));
        }
        else
        {
            int col = c % 2;
            int row = c / 2;

            float yp = regionStartY(row == 2 ? 1 : 2) + labelHeight + padMargin;
            float xp = (col == 0 ? padFromEdge : boxw / 2 + padMargin);
            float ys = lowerCtrlH - 2 * padMargin;
            float xs = boxw / 2 - padFromEdge - padMargin;
            return rack::math::Rect(rack::Vec(xp, yp), rack::Vec(xs, ys));
        }
        return rack::math::Rect(rack::Vec(10 + 5 * c, 10 + 9 * c), rack::Vec(20, 20));
    }

    void moduleBackground(NVGcontext *vg)
    {
        for (int i = 0; i < regionLabels.size(); ++i)
        {
            centerRuledLabel(vg, padFromEdge, regionStartY(i), boxw - 2 * padFromEdge,
                             regionLabels[i].c_str());
        }

        for (int i = 0; i < 8; ++i)
        {
            auto cb = controlBox(i);
            if (i < 2)
            {
                nvgBeginPath(vg);
                nvgFontFaceId(vg, fontId(vg));
                nvgFontSize(vg, 12);
                nvgFillColor(vg, panelLabel());
                nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
                nvgText(vg, cb.pos.x, cb.pos.y + cb.size.y / 2, labels[i].c_str(), NULL);

                nvgBeginPath(vg);
                nvgMoveTo(vg, cb.pos.x + 30, cb.pos.y + cb.size.y / 2);
                nvgLineTo(vg, boxw - padFromEdge - upperTextWidth + 4, cb.pos.y + cb.size.y / 2);
                nvgStrokeWidth(vg, 1);
                nvgStrokeColor(vg, panelLabel());
                nvgStroke(vg);

                float rx = boxw - padFromEdge - upperTextWidth;
                float ry = cb.pos.y + (cb.size.y - textHeight) / 2;
                float sx = upperTextWidth;
                float sy = textHeight;

                drawTextBGRect(vg, rx, ry, sx, sy);
            }
            if (i >= 2)
            {
                // bottom half controls
                // OK so rotated text
                nvgBeginPath(vg);
                nvgSave(vg);
                nvgTranslate(vg, cb.pos.x + padMargin + 10, cb.pos.y);
                nvgRotate(vg, M_PI / 2.0);
                nvgFontFaceId(vg, fontId(vg));
                nvgFontSize(vg, 12);
                nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
                nvgFillColor(vg, panelLabel());
                nvgText(vg, 0, 0, labels[i].c_str(), NULL);

                float bounds[4];
                nvgTextBounds(vg, 0, 0, labels[i].c_str(), NULL, bounds);
                nvgRestore(vg);

                float linesx = cb.pos.x + padMargin + 10 - bounds[3] / 2.0;
                float linesy = cb.pos.y + bounds[2] + 2;

                float rx = cb.pos.x + lowerCtrlOff;
                float ry = cb.pos.y + cb.size.y - textHeight;
                float sx = cb.size.x - lowerCtrlOff;
                float sy = textHeight;

                dropRightLine(vg, linesx, linesy, rx + 2, ry + sy / 2);
                drawTextBGRect(vg, rx, ry, sx, sy);
            }
            else
            {
                // Top half controls
            }
        }

        auto cp = controlBox(0);
        clockBackground(vg, padFromEdge, cp.pos.y, true);

        drawStackedInputOutputBackground(vg, box);
    }
};

SurgeChorusWidget::SurgeChorusWidget(SurgeChorusWidget::M *module) : SurgeModuleWidgetCommon()
{
    setModule(module);

    box.size = rack::Vec(boxw, RACK_HEIGHT);
    SurgeRackBG *bg = new SurgeRackBG(rack::Vec(0, 0), box.size, "Chorus");
    bg->moduleSpecificDraw = [this](NVGcontext *vg) { this->moduleBackground(vg); };
    addChild(bg);

    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 0, box, true), module,
                                                 M::INPUT_L_OR_MONO));
    addInput(rack::createInput<rack::PJ301MPort>(ioPortLocation(true, 1, box, true), module,
                                                 M::INPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(true, 2, box, true), module,
                                               M::INPUT_GAIN));

    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 0, box, true), module,
                                                   M::OUTPUT_L_OR_MONO));
    addOutput(rack::createOutput<rack::PJ301MPort>(ioPortLocation(false, 1, box, true), module,
                                                   M::OUTPUT_R));
    addParam(rack::createParam<SurgeSmallKnob>(ioPortLocation(false, 2, box, true), module,
                                               M::OUTPUT_GAIN));

    // top half
    for (int i = 0; i < 2; ++i)
    {
        auto cb = controlBox(i);
        float extraOff = 28;
        float extraY = padMargin / 2.0;
        addParam(rack::createParam<SurgeSmallKnob>(
            rack::Vec(cb.pos.x + extraOff, cb.pos.y + extraY), module, M::FX_PARAM_0 + paramId[i]));
        addInput(rack::createInput<rack::PJ301MPort>(
            rack::Vec(cb.pos.x + extraOff + portX + padMargin, cb.pos.y + extraY), module,
            M::FX_PARAM_INPUT_0 + paramId[i]));
        if (i == 0)
        {
            addParam(rack::createParam<SurgeSwitch>(
                rack::Vec(cb.pos.x + extraOff + 2 * portX + 2 * padMargin, cb.pos.y + 2 + extraY),
                module, M::PARAM_TEMPOSYNC_0 + paramId[i]));
            if (module && module->pb[M::FX_PARAM_0 + paramId[i]] != nullptr)
            {
                module->pb[M::FX_PARAM_0 + paramId[i]]->tsbpmLabel = false;
            }
        }
        addChild(TextDisplayLight::create(rack::Vec(boxw - padFromEdge - upperTextWidth + padMargin,
                                                    cb.pos.y + cb.size.y / 2 - textHeight / 2),
                                          rack::Vec(upperTextWidth, textHeight),
                                          module ? &(module->pb[paramId[i]]->valCache) : nullptr,
                                          12, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE,
                                          parameterValueText_KEY()));
    }

    // Bottom half
    for (int i = 2; i < 8; ++i)
    {
        auto cb = controlBox(i);
        float extraOff = 5;
        addParam(rack::createParam<SurgeSmallKnob>(
            rack::Vec(cb.pos.x + lowerCtrlOff + extraOff, cb.pos.y), module,
            M::FX_PARAM_0 + paramId[i]));
        addInput(rack::createInput<rack::PJ301MPort>(
            rack::Vec(cb.pos.x + lowerCtrlOff + extraOff + portX + padMargin, cb.pos.y), module,
            M::FX_PARAM_INPUT_0 + paramId[i]));
        addChild(TextDisplayLight::create(
            rack::Vec(cb.pos.x + lowerCtrlOff + 2, cb.pos.y + cb.size.y - textHeight),
            rack::Vec(cb.size.x - lowerCtrlOff - 4, textHeight),
            module ? &(module->pb[paramId[i]]->valCache) : nullptr, 12,
            NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, parameterValueText_KEY()));
    }

    auto p0 = controlBox(0);
    p0.pos.x = padFromEdge;
    p0.pos.y += clockPad;
    p0.pos.x += padMargin;
    p0.pos.y += padMargin;
    addInput(rack::createInput<rack::PJ301MPort>(p0.pos, module, M::CLOCK_CV_INPUT));
}

auto mchorus =
    addFX(rack::createModel<SurgeChorusWidget::M, SurgeChorusWidget>("SurgeChorus"), fxt_chorus4);
