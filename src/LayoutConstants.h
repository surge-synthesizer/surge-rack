//
// Created by Paul Walker on 10/8/22.
//

#ifndef RACK_HACK_LAYOUTCONSTANTS_H
#define RACK_HACK_LAYOUTCONSTANTS_H

namespace sst::surgext_rack::layout
{
struct LayoutConstants
{
    // Standard module constants
    static constexpr int numberOfScrews = 12;

    static constexpr std::array<float, 4> columnCenters_MM{9.48, 23.48, 37.48, 51.48};
    static constexpr std::array<float, 2> modulationRowCenters_MM{85.32, 100.16};
    static constexpr std::array<float, 2> vcoRowCenters_MM{55, 71};
    static constexpr float inputRowCenter_MM = 114.5;
    static constexpr float columnWidth_MM = 14;
    static constexpr float modulationLabelBaseline_MM = 94.864;
    static constexpr float inputLabelBaseline_MM = 109.203;
    static constexpr float verticalPortOffset_MM = 0.5;

    // FX Constants
    static constexpr float rowStart_MM = 71;
    static constexpr float unlabeledGap_MM = 16;
    static constexpr float labeledGap_MM = 20;
    static constexpr float bigCol0 = columnCenters_MM[1] - 7;
    static constexpr float bigCol1 = columnCenters_MM[2] + 7;
    static constexpr float knobGap16_MM = unlabeledGap_MM - 9 + 18;

    // VCO VCF Waveshaper
    static constexpr float VCOplotH_MM = 36;
    static constexpr float VCOplotW_MM = 51;
    static constexpr float VCOplotCX_MM = 30.48;
    static constexpr float VCOplotCY_MM = 27.35;
    static constexpr float VCOplotControlsH_MM = 5;

    static constexpr float labelSize_pt = 7.2;

    // background
    static constexpr float mainLabelBaseline_MM = 6.295, mainLabelSize_PT = 10.5;

};
} // namespace sst::surgext_rack::layout

#endif // RACK_HACK_LAYOUTCONSTANTS_H
