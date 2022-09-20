//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_TREEMONSTER_HPP
#define XTRACK_TREEMONSTER_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::treemonster
{
struct TreeMonster : modules::XTModule
{
    std::string getName() override { return "TreeMonster"; }
};
} // namespace sst::surgext_rack::treemonster
#endif // RACK_HACK_mixer_HPP
