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
} // namespace sst::surgext_rack::temposync_support
#endif // SURGEXTRACK_TEMPOSYNCSUPPORT_H
