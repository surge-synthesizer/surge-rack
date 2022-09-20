//
// Created by Paul Walker on 9/20/22.
//

#ifndef XTRACK_MODMATRIX_HPP
#define XTRACK_MODMATRIX_HPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>
#include "DebugHelpers.h"
#include "globals.h"

namespace sst::surgext_rack::modmatrix
{
struct ModMatrix : modules::XTModule
{
    std::string getName() override { return "ModMatrix"; }
};
} // namespace sst::surgext_rack::modmatrix
#endif // RACK_HACK_mixer_HPP
