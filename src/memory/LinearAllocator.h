
#ifndef H_ROB_LINEAR_ALLOCATOR_H
#define H_ROB_LINEAR_ALLOCATOR_H

#include "../Types.h"
#include "PtrAlign.h"

#include <new>
#include <functional>

namespace rob
{

    class LinearAllocator
    {
    public:
        LinearAllocator(const LinearAllocator &) = delete;
        LinearAllocator(LinearAllocator &) = delete;
        LinearAllocator& operator = (LinearAllocator &) = delete;

        LinearAllocator();
        LinearAllocator(size_t_32 size);
        LinearAllocator(void *start, size_t_32 size);
        ~LinearAllocator();

        void SetMemory(void *start, size_t_32 size);
        size_t_32 GetAllocatedSize() const;
        size_t_32 GetTotalSize() const;

        void* Allocate(size_t_32 size);
        void* Allocate(size_t_32 size, size_t_32 alignment);

        template <class T>
        T* AllocateArray(size_t_32 count)
        {
            const size_t_32 size = GetArraySize<T>(count);
            return static_cast<T*>(Allocate(size, alignof(T)));
        }

        void Reset();

        template <class T, class... Args>
        T* new_object(Args&& ...args)
        {
            void *ptr = Allocate(sizeof(T), alignof(T));
            return new (ptr) T(std::forward<Args&&>(args)...);
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

