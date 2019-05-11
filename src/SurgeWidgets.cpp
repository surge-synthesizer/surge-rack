#include "SurgeWidgets.hpp"

#if MAC
#include <execinfo.h>
#endif

void stackToInfo()
{
#if MAC
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        INFO( "STACK[%d]: %s", i, strs[i] );
    }
    free(strs);
#endif
}

void SurgeButtonBank::drawWidget(NVGcontext *vg)
{
    if( fontId < 0 )
        fontId = InternalFontMgr::get(vg, SurgeStyle::fontFace() );
    
    auto bw = (box.size.x-1.0) / rows;
    auto bh = (box.size.y-1.0) / cols;

    NVGcolor unselT = nvgRGB(0xdd, 0xda, 0xc0);
    NVGcolor unselM = nvgRGB(0xff, 0xfd, 0xd0);
    NVGcolor unselB = nvgRGB(0xcc, 0xc7, 0xa0);

    NVGcolor selT = SurgeStyle::surgeOrangeMedium();
    NVGcolor selM = SurgeStyle::surgeOrange();
    NVGcolor selB = SurgeStyle::surgeOrangeMedium();
    
    int cell = 0;
    for(int c=0;c<cols;++c)
        for( int r=0;r<rows;++r)
        {
            bool selected = false;
            selected = (int)getPValue() == cell;
            
            auto px = r * bw; // remember we are already translated by box.pos.y
            auto py = c * bh;

            // Draw the button background
            if( selected )
            {
                nvgSave(vg);
                nvgBeginPath(vg);
                nvgRect(vg,px,py,bw,bh * 0.3);
                NVGpaint tg = nvgLinearGradient(vg, px, py, px, py + bh * 0.3, selT, selM );
                nvgFillPaint(vg, tg);
                nvgFill(vg);
                nvgRestore(vg);

                nvgSave(vg);
                nvgBeginPath(vg);
                nvgRect(vg,px,py + bh*0.3,bw,bh * 0.7);
                tg = nvgLinearGradient(vg, px, py+bh * 0.3, px, py + bh, selM, selB);
                nvgFillPaint(vg, tg);
                nvgFill(vg);
                nvgRestore(vg);

            }
            else
            {
                nvgBeginPath(vg);
                nvgRect(vg,px,py,bw,bh * 0.7);
                NVGpaint tg = nvgLinearGradient(vg, px, py, px, py + bh * 0.7, unselT, unselM );
                nvgFillPaint(vg, tg);
                nvgFill(vg);
                
                nvgBeginPath(vg);
                nvgRect(vg,px,py + bh*0.7,bw, bh * 0.3);
                tg = nvgLinearGradient(vg, px, py + bh * 0.7, px, py + bh, unselM, unselB );
                nvgFillPaint(vg, tg);
                nvgFill(vg);
            }

            
            if( cell < cellLabels.size() )
            {
                if( selected )
                {
                    nvgBeginPath(vg);
                    nvgFontFaceId(vg,fontId);
                    nvgFontSize(vg,fontSize);
                    nvgFontBlur(vg, 4 );

                    nvgFillColor(vg, SurgeStyle::surgeWhite() );
                    nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER );
                    nvgText(vg, px + bw/2, py + bh/2, cellLabels[cell].c_str(), NULL );
                    nvgFontBlur(vg, 2 );
                    nvgFillColor(vg, SurgeStyle::surgeWhite() );
                    nvgText(vg, px + bw/2, py + bh/2, cellLabels[cell].c_str(), NULL );
                    nvgFontBlur(vg, 0);
                }
                
                nvgBeginPath(vg);
                nvgFontFaceId(vg,fontId);
                nvgFontSize(vg,fontSize);
                
                nvgFillColor(vg, SurgeStyle::surgeBlue() );
                nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER );
                nvgText(vg, px + bw/2, py + bh/2, cellLabels[cell].c_str(), NULL );
            }

            if( r != rows - 1 )
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, px + bw, py + 3 );
                nvgLineTo(vg, px + bw, py + bh - 3 );
                nvgStrokeColor(vg,SurgeStyle::buttonBoxContainerStroke());
                nvgStrokeWidth(vg, 1);
                nvgStroke(vg);
            }
            
            cell++;
        }

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x - 1, box.size.y - 1);
    nvgStrokeColor(vg, SurgeStyle::surgeOrange() );
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
}

