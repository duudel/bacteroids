
#ifndef H_ROB_LINEAR_ALLOCATOR_H
#define H_ROB_LINEAR_ALLOCATOR_H

#include "../Types.h"

#include <new>

namespace rob
{

    class LinearAllocator
    {
    public:
        LinearAllocator(const LinearAllocator &) = delete;
        LinearAllocator(LinearAllocator &) = delete;

        LinearAllocator();
        LinearAllocator(size_t size);
        LinearAllocator(void *start, size_t size);
        ~LinearAllocator();

        void SetMemory(void *start, size_t size);
        size_t GetAllocatedSize() const;

        void* Allocate(size_t size);
        void* Allocate(size_t size, size_t alignment);

        void Reset();

        template <class T, class... Args>
        T* new_object(Args&& ...args)
        {
            void *ptr = Allocate(sizeof(T), alignof(T));
            return new (ptr) T(args...);
        }

        template <class T>
        void del_object(T *object)
        {
            if (object) object->~T();
        }

    private:
        char *m_start;
        char *m_myMemory;
        char *m_head;
        const char *m_end;
    };

} // rob

#endif // H_ROB_LINEAR_ALLOCATOR_H

