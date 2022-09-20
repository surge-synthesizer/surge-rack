//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_mixer_HPP
#define XTRACK_mixer_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::mixer
{
struct Mixer : modules::XTModule
{
    std::string getName() override { return "Mixer"; }
};
} // namespace sst::surgext_rack::mixer
#endif // RACK_HACK_mixer_HPP
