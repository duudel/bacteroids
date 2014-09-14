
#ifndef H_ROB_PTR_ALIGN_H
#define H_ROB_PTR_ALIGN_H

#include "../Types.h"

namespace rob
{

    inline char* ptr_align(char *ptr, size_t alignment)
    {
        const uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
        const uintptr_t mask = alignment - 1;
        return reinterpret_cast<char*>((p + mask) & ~mask);
    }

} // rob

#endif // H_ROB_PTR_ALIGN_H

