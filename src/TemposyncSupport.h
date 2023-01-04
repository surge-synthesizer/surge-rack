//
// Created by Paul Walker on 12/16/22.
//

#ifndef SURGEXTRACK_TEMPOSYNCSUPPORT_H
#define SURGEXTRACK_TEMPOSYNCSUPPORT_H

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
