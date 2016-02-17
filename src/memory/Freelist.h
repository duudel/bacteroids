
#ifndef H_ROB_FREELIST_H
#define H_ROB_FREELIST_H

#include "../Types.h"

namespace rob
{

    class Freelist
    {
    public:
        Freelist();

        void* Obtain();
        void Return(void *ptr);

        char* AddElements(void *start, size_t_32 size, size_t_32 elementSize, size_t_32 elementAlign);

    private:
        Freelist *m_next;
    };

} // rob

#endif // H_ROB_FREELIST_H

