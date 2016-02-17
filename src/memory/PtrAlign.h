
#ifndef H_ROB_PTR_ALIGN_H
#define H_ROB_PTR_ALIGN_H

#include "../Types.h"

namespace rob
{

    inline uintptr_t align(const uintptr_t p, const size_t_32 alignment)
    {
        const uintptr_t mask = alignment - 1;
        return (p + mask) & ~mask;
    }

    inline char* ptr_align(const char *ptr, const size_t_32 alignment)
    {
        const uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
        return reinterpret_cast<char*>(align(p, alignment));
    }

    template <class T>
    size_t_32 GetArraySize(const size_t_32 count)
    { return count * align(sizeof(T), alignof(T)); }

} // rob

#endif // H_ROB_PTR_ALIGN_H

