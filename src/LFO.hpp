//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_LFO_HPP
#define XTRACK_LFO_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::lfo
{
struct LFO : modules::XTModule
{
    std::string getName() override { return "LFO"; }
};
} // namespace sst::surgext_rack::lfo
#endif // RACK_HACK_LFO_HPP
