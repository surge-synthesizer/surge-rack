#include "UserInteractions.h"
#include "rack.hpp"
#include <sstream>

namespace Surge
{
namespace UserInteractions
{

void promptError(const std::string &message, const std::string &title,
                 SurgeGUIEditor *guiEditor)
{
    std::ostringstream oss;
    oss << "Surge Error\n"
        << title << "\n"
        << message << "\n";
    // FIXME: What's the appropriate rack error mechanism?
    WARN(oss.str().c_str());
}

void promptError(const Surge::Error &error, SurgeGUIEditor *guiEditor)
{
    promptError(error.getMessage(), error.getTitle());
}

MessageResult promptOKCancel(const std::string &message, const std::string &title,
                             SurgeGUIEditor *guiEditor)
{
    std::ostringstream oss;
    oss << "Surge OkCancel\n"
        << title << "\n"
        << message << "\n" 
        << "Returning CANCEL";
    INFO(oss.str().c_str());
    return UserInteractions::CANCEL;
}

void openURL(const std::string &url)
{
}

void promptFileOpenDialog(const std::string& initialDirectory,
                          const std::string& filterSuffix,
                          std::function<void(std::string)> callbackOnOpen,
                          SurgeGUIEditor* guiEditor)
{
   UserInteractions::promptError("OpenFileDialog is unimplemented in this version of Surge. Sorry!",
                                 "Unimplemented Function", guiEditor);
}
};

};
