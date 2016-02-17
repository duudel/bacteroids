
#ifndef H_ROB_POOL_H
#define H_ROB_POOL_H

#include "Freelist.h"
#include "../Assert.h"

#include <new>

namespace rob
{

    template <class T>
    class Pool
    {
    public:
        Pool()
            : m_objects()
            , m_start(nullptr)
            , m_end(nullptr)
            , m_allocations(0)
        { }

        Pool(const Pool&) = delete;
        Pool& operator = (const Pool&) = delete;

        ~Pool()
        { ROB_ASSERT(m_allocations == 0); }

        size_t_32 GetAllocationCount() const
        { return m_allocations; }

        T* Obtain()
        {
            void *ptr = m_objects.Obtain();
            ROB_ASSERT(ptr != 0 && "Pool out of memory");
            m_allocations++;
            return new (ptr) T();
        }

        T* Get(size_t_32 index)
        { return m_start + index; }

        void Return(T *object)
        {
            ROB_ASSERT(m_start <= object && object < m_end);
            object->~T();
            m_objects.Return(object);
            ROB_ASSERT(m_allocations > 0);
            m_allocations--;
        }

        size_t_32 IndexOf(const T *object) const
        { return static_cast<size_t_32>(object - m_start); }

        void SetMemory(void *start, size_t_32 size)
        {
            ROB_ASSERT(m_start == nullptr); // Not memory set previously

            char *s = m_objects.AddElements(start, size, sizeof(T), alignof(T));
            m_start = reinterpret_cast<T*>(s);
            m_end = reinterpret_cast<T*>(s + size);
        }

    private:
        Freelist m_objects;
        T *m_start;
        T *m_end;
        size_t_32 m_allocations;
    };

} // rob

#endif // H_ROB_POOL_H

