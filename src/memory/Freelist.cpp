
#include "Freelist.h"

namespace rob
{

    Freelist::Freelist()
        : m_next(nullptr)
    { }

    void* Freelist::Obtain()
    {
        if (m_next == nullptr)
            return 0;

        Freelist *head = m_next;
        m_next = head->m_next;
        return static_cast<void*>(head);
    }

    void Freelist::Return(void *ptr)
    {
        Freelist *head = static_cast<Freelist*>(ptr);
        head->m_next = m_next;
        m_next = head;
    }

    void Freelist::AddElements(void *start, size_t size, size_t elementSize)
    {
        char *it = static_cast<char*>(start);
        const char * const end = it + size;

        for (; it + elementSize <= end; it += elementSize)
        {
            Return(static_cast<void*>(it));
        }
    }

} // rob
