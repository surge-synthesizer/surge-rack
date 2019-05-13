#include "UserInteractions.h"
#include "rack.hpp"
#include <sstream>

using namespace rack;

namespace Surge {
namespace UserInteractions {

void promptError(const std::string &message, const std::string &title,
                 SurgeGUIEditor *guiEditor) {
    std::ostringstream oss;
    oss << "Surge Error\n" << title << "\n" << message << "\n";
#if RACK_V1
    // FIXME: What's the appropriate rack error mechanism?
    WARN(oss.str().c_str());
#else
    warn(oss.str().c_str());
#endif
}

void promptError(const Surge::Error &error, SurgeGUIEditor *guiEditor) {
    promptError(error.getMessage(), error.getTitle());
}

MessageResult promptOKCancel(const std::string &message,
                             const std::string &title,
                             SurgeGUIEditor *guiEditor) {
    std::ostringstream oss;
    oss << "Surge OkCancel\n"
        << title << "\n"
        << message << "\n"
        << "Returning CANCEL";
#if RACK_V1
    // FIXME: What's the appropriate rack error mechanism?
    INFO(oss.str().c_str());
#else
    info(oss.str().c_str());
#endif
    return UserInteractions::CANCEL;
}

void openURL(const std::string &url) {}

void promptFileOpenDialog(const std::string &initialDirectory,
                          const std::string &filterSuffix,
                          std::function<void(std::string)> callbackOnOpen,
                          SurgeGUIEditor *guiEditor) {
    UserInteractions::promptError(
        "OpenFileDialog is unimplemented in this version of Surge. Sorry!",
        "Unimplemented Function", guiEditor);
}
}; // namespace UserInteractions

}; // namespace Surge
