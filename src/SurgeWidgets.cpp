#include "SurgeWidgets.hpp"

#if MAC
#include <execinfo.h>
#endif

void stackToInfo()
{
#if MAC
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        INFO( "STACK[%d]: %s", i, strs[i] );
    }
    free(strs);
#endif
}
