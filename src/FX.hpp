#ifndef SURGE_XT_RACK_FXHPP
#define SURGE_XT_RACK_FXHPP

#include "SurgeXT.hpp"
#include "XTModule.hpp"
#include "rack.hpp"
#include <cstring>

namespace sst::surgext_rack::fx
{
template <int fxType> struct FXConfig
{
    static constexpr int extraInputs() { return 0; }
    static constexpr int specificParamCount() { return 0; }
    static constexpr int panelWidthInScrews() { return 12; }
};

template <int fxType> struct FX : modules::XTModule
{
    static constexpr int n_mod_inputs{4};
    static constexpr int n_arbitrary_switches{4};

    enum ParamIds
    {
        FX_PARAM_0 = 0,
        FX_MOD_PARAM_0 = FX_PARAM_0 + n_fx_params,
        FX_SPECIFIC_PARAM_0 = FX_MOD_PARAM_0 + n_fx_params * n_mod_inputs,
        NUM_PARAMS = FX_SPECIFIC_PARAM_0 + FXConfig<fxType>::specificParamCount()
    };

    enum InputIds
    {
        INPUT_L,
        INPUT_R,
        INPUT_CLOCK,
        MOD_INPUT_0,
        INPUT_SPECIFIC_0 = MOD_INPUT_0 + n_mod_inputs,
        NUM_INPUTS = INPUT_SPECIFIC_0 + FXConfig<fxType>::extraInputs()
    };

    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    FX() : XTModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(INPUT_L, "Left");
        configInput(INPUT_R, "Right");
        configInput(INPUT_CLOCK, "Clock/Tempo CV");
        for (int m = 0; m < n_mod_inputs; ++m)
        {
            auto s = std::string("Modulation Signal ") + std::to_string(m + 1);
            configInput(MOD_INPUT_0 + m, s);
        }
        configOutput(OUTPUT_L, "Left (or Mono merged)");
        configOutput(OUTPUT_R, "Right");
    }

    std::string getName() override { return std::string("FX<") + fx_type_names[fxType] + ">"; }
    void process(const typename rack::Module::ProcessArgs &args) override {}
};
} // namespace sst::surgext_rack::fx
#endif
