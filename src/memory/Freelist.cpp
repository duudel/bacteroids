
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

    char* Freelist::AddElements(void *start, size_t_32 size, size_t_32 elementSize, size_t_32 elementAlign)
    {
        elementSize = elementSize < sizeof(void*) ? sizeof(void*) : elementSize;

        char *it = static_cast<char*>(start);
        const char * const end = it + size;

        const size_t_32 alignedSize = align(elementSize, elementAlign);

        it = ptr_align(it, elementAlign);
        char *s = it;
        while (it + elementSize <= end)
        {
            Return(static_cast<void*>(it));
            it += alignedSize;
        }
        return s;
    }

} // rob
