
#ifndef H_ROB_ALIGNED_STORAGE_H
#define H_ROB_ALIGNED_STORAGE_H

#include "../Types.h"

namespace rob
{

    #define ROB_ALIGNED_FIELD(t, n, a)  t n __attribute__((aligned(a)))

    template <size_t_32 S, size_t_32 Align>
    struct AlignedStorage
    {
        ROB_ALIGNED_FIELD(char, m_value[S], Align);
    };

} // rob

#endif // H_ROB_ALIGNED_STORAGE_H

