#include "XTModule.hpp"

std::mutex sst::surgext_rack::modules::XTModule::xtSurgeCreateMutex{};
std::atomic<bool> sst::surgext_rack::modules::XTModule::showedPathsOnce{false};
