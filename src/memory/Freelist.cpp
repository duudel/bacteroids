
#include "Freelist.h"
#include "PtrAlign.h"

namespace rob
{

    Freelist::Freelist()
        : m_next(nullptr)
    { }

    void* Freelist::Obtain()
    {
        if (m_next == nullptr)
            return nullptr;

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

    void Freelist::AddElements(void *start, size_t size, size_t elementSize, size_t elementAlign)
    {
        char *it = static_cast<char*>(start);
        const char * const end = it + size;

        it = ptr_align(it, elementAlign);
        while (it + elementSize < end)
        {
            Return(static_cast<void*>(it));
            it += elementSize;
            it = ptr_align(it, elementAlign);
        }
    }

} // rob
