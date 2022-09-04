#include "SurgeWidgets.hpp"

#if MAC
#include <execinfo.h>
#endif

void stackToInfo()
{
#if MAC
    void *callstack[128];
    int i, frames = backtrace(callstack, 128);
    char **strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i)
    {
        INFO("[SurgeRack] StackTrace[%3d]: %s", i, strs[i]);
    }
    free(strs);
#endif
}

#if BUILD_OLD_WIDGETS
void SurgeButtonBank::drawSelectedButton(NVGcontext *vg, float x, float y, float w, float h,
                                         std::string label)
{
    NVGcolor gradientTop = SurgeStyle::buttonBankSelectedGradientTop();
    NVGcolor gradientMid = SurgeStyle::buttonBankSelectedGradientMid();
    NVGcolor gradientBot = SurgeStyle::buttonBankSelectedGradientEnd();

    NVGcolor stroke = SurgeStyle::buttonBankSelectedOutline();
    NVGcolor lightStroke = SurgeStyle::buttonBankSelectedLightOutline();
    NVGcolor lightFillTop = SurgeStyle::buttonBankSelectedLightGradientTop();
    NVGcolor lightFillBot = SurgeStyle::buttonBankSelectedLightGradientEnd();

    NVGcolor dropShadow = SurgeStyle::buttonBankDropShadow();

    int lightWidth = 10;
    int lightHeight = 4;
    int lightRadius = 2;

    // Start with the drop shadow
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 1.8, y + 2.2, w - 2, h - 2, 3);
    NVGpaint bgr = nvgBoxGradient(vg, x + 1.8, y + 1.8, w - 2, h - 2, 3, 3, dropShadow,
                                  SurgeStyle::panelBackground());
    nvgFillPaint(vg, bgr);
    nvgFill(vg);

    nvgSave(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 1.5, y + 1.5, w - 3, h - 3, 3);

    NVGpaint vGradient =
        nvgLinearGradient(vg, x + 1, y + 1, x + 1, y + h / 2, gradientTop, gradientMid);
    nvgScissor(vg, x, y, w, h / 2);
    nvgFillPaint(vg, vGradient);
    nvgFill(vg);
    nvgResetScissor(vg);

    vGradient = nvgLinearGradient(vg, x + 1, y + h / 2, x + 1, y + h, gradientMid, gradientBot);
    nvgScissor(vg, x, y + h / 2 - 1, w, h / 2 + 1);
    nvgFillPaint(vg, vGradient);
    nvgFill(vg);
    nvgRestore(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 1.5, y + 1.5, w - 3, h - 3, 3);
    nvgStrokeColor(vg, stroke);
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    float lx0 = x + w / 2 - lightWidth / 2;
    float ly0 = y + h - 3 - lightHeight;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, lx0, ly0, lightWidth, lightHeight, lightRadius);
    NVGpaint lightG =
        nvgLinearGradient(vg, lx0, ly0, lx0, ly0 + lightHeight, lightFillTop, lightFillBot);
    nvgFillPaint(vg, lightG);
    nvgFill(vg);
    nvgStrokeColor(vg, lightStroke);
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgFillColor(vg, SurgeStyle::buttonBankSelectedLabelGlow());
    nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    nvgFontFaceId(vg, fontId);
    nvgFontBlur(vg, 3);
    nvgFontSize(vg, 11);
    nvgText(vg, x + w / 2, y + 2, label.c_str(), NULL);

    nvgBeginPath(vg);
    nvgFillColor(vg, SurgeStyle::buttonBankSelectedLabelGlow());
    nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 11);
    nvgFontBlur(vg, 1);
    nvgText(vg, x + w / 2, y + 2, label.c_str(), NULL);

    nvgBeginPath(vg);
    nvgFillColor(vg, SurgeStyle::buttonBankSelectedLabel());
    nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 11);
    nvgFontBlur(vg, 0);
    nvgText(vg, x + w / 2, y + 2, label.c_str(), NULL);
}

void SurgeButtonBank::drawUnselectedButton(NVGcontext *vg, float x, float y, float w, float h,
                                           std::string label)
{
    NVGcolor gradientTop = SurgeStyle::buttonBankUnselectedGradientTop();
    NVGcolor gradientMid = SurgeStyle::buttonBankUnselectedGradientMid();
    NVGcolor gradientBot = SurgeStyle::buttonBankUnselectedGradientEnd();

    NVGcolor stroke = SurgeStyle::buttonBankUnselectedOutline();

    NVGcolor lightStroke = SurgeStyle::buttonBankUnselectedLightOutline();
    NVGcolor lightFill = SurgeStyle::buttonBankUnselectedLightFill();

    NVGcolor dropShadow = SurgeStyle::buttonBankDropShadow();

    int lightWidth = 10;
    int lightHeight = 4;
    int lightRadius = 2;

    // Start with the drop shadow
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 2, y + 3, w - 2, h - 2, 3);
    NVGpaint bgr = nvgBoxGradient(vg, x + 2, y + 3, w - 2, h - 2, 3, 3, dropShadow,
                                  SurgeStyle::panelBackground());
    nvgFillPaint(vg, bgr);
    nvgFill(vg);

    nvgSave(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 1.5, y + 1.5, w - 3, h - 3, 3);

    NVGpaint vGradient =
        nvgLinearGradient(vg, x + 1, y + 1, x + 1, y + h / 2, gradientTop, gradientMid);
    nvgScissor(vg, x, y, w, h / 2);
    nvgFillPaint(vg, vGradient);
    nvgFill(vg);
    nvgResetScissor(vg);

    vGradient = nvgLinearGradient(vg, x + 1, y + h / 2, x + 1, y + h, gradientMid, gradientBot);
    nvgScissor(vg, x, y + h / 2 - 1, w, h / 2 + 1);
    nvgFillPaint(vg, vGradient);
    nvgFill(vg);
    nvgRestore(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 1.5, y + 1.5, w - 3, h - 3, 3);
    nvgStrokeColor(vg, stroke);
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    float lx0 = x + w / 2 - lightWidth / 2;
    float ly0 = y + h - 3 - lightHeight;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, lx0, ly0, lightWidth, lightHeight, lightRadius);
    nvgFillColor(vg, lightFill);
    nvgFill(vg);
    nvgStrokeColor(vg, lightStroke);
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgFillColor(vg, SurgeStyle::buttonBankUnselectedLabel());
    nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    nvgFontFaceId(vg, fontId);
    nvgFontSize(vg, 11);
    nvgText(vg, x + w / 2, y + 2, label.c_str(), NULL);
}

void SurgeButtonBank::drawWidget(NVGcontext *vg)
{
    if (fontId < 0)
        fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace());

    auto bw = (box.size.x - 1.0) / rows;
    auto bh = (box.size.y - 1.0) / cols;

    int cell = 0;
    for (int c = 0; c < cols; ++c)
        for (int r = 0; r < rows; ++r)
        {
            bool selected = (getSelectedCell() == cell);

            auto px = r * bw; // remember we are already translated by box.pos.y
            auto py = c * bh;

            std::string lab = "err";
            if (cell < cellLabels.size())
                lab = cellLabels[cell];

            // Draw the button background
            if (selected)
            {
                drawSelectedButton(vg, px, py, bw, bh, lab);
            }
            else
            {
                drawUnselectedButton(vg, px, py, bw, bh, lab);
            }

            cell++;
        }
}
#endif