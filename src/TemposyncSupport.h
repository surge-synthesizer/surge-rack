/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#ifndef SURGE_XT_RACK_SRC_TEMPOSYNCSUPPORT_H
#define SURGE_XT_RACK_SRC_TEMPOSYNCSUPPORT_H

#include <cmath>
#include "Parameter.h"

namespace sst::surgext_rack::temposync_support
{
inline float roundTemposync(float f)
{
    float a, b = modff(f, &a);
    if (b < 0)
    {
        b += 1.f;
        a -= 1.f;
    }
    b = powf(2.0f, b);

    if (b > 1.41f)
    {
        b = log2(1.5f);
    }
    else if (b > 1.167f)
    {
        b = log2(1.3333333333f);
    }
    else
    {
        b = 0.f;
    }
    return a + b;
};

inline std::string temposyncLabel(float f, bool minus = false)
{
    auto ts = temposync_support::roundTemposync(f);
    Parameter p;
    return p.tempoSyncNotationValue((minus ? -1 : 1) * ts);
}

inline std::string abbreviateSurgeTemposyncLabel(std::string s)
{
    std::list<std::pair<std::string, std::string>> replace = {
        {"whole note", "W"}, {"note", ""},     {" dotted", "D"}, {" triplet", "T"},
        {"whole", "W"},      {"double", "2x"}, {"triple", "3x"}};

    for (const auto &[from, to] : replace)
    {
        auto p = s.find(from);
        if (p != std::string::npos)
        {
            auto pre = s.substr(0, p);
            auto post = s.substr(p + from.size());
            s = pre + to + post;
        }
    }
    return s;
}
} // namespace sst::surgext_rack::temposync_support
#endif // SURGEXTRACK_TEMPOSYNCSUPPORT_H
