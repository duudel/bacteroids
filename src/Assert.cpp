
#include "Assert.h"
#include "Log.h"

//#include <SDL2/SDL_log.h>

namespace rob
{

    void Assert(const char * const x, const char * const file, const int line)
    {
        log::Print("Assert: ", x, " in ", file, ":", line, "\n");
        ((int*)nullptr)[0] = 42;

//        ::SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "%s in %s:%i", x, file, line);
    }

    void Warn(const char * const x, const char * const file, const int line)
    {
        log::Warning(x, " in ", file, ":", line, "\n");
    }

} // rob
