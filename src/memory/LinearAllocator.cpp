
#include "LinearAllocator.h"
#include "../Assert.h"

namespace rob
{

    LinearAllocator::LinearAllocator()
        : m_start(nullptr)
        , m_myMemory(nullptr)
        , m_head(nullptr)
        , m_end(nullptr)
    { }

    LinearAllocator::LinearAllocator(size_t size)
        : m_start(new char[size])
        , m_myMemory(m_start)
        , m_head(m_start)
        , m_end(m_start + size)
    { }

    LinearAllocator::LinearAllocator(void *start, size_t size)
        : m_start(static_cast<char*>(start))
        , m_myMemory(nullptr)
        , m_head(m_start)
        , m_end(m_head + size)
    { }

    LinearAllocator::~LinearAllocator()
    { delete[] m_myMemory; }

    void LinearAllocator::SetMemory(void *start, size_t size)
    {
        ROB_ASSERT(m_head == nullptr);
        m_start = static_cast<char*>(start);
        m_myMemory = nullptr;
        m_head = m_start;
        m_end = m_head + size;
    }

    size_t LinearAllocator::GetAllocatedSize() const
    { return static_cast<size_t>(m_head - m_start); }

    void* LinearAllocator::Allocate(size_t size)
    {
        if (m_head + size > m_end)
            return 0;
        char *ptr = m_head;
        m_head += size;
        return ptr;
    }

    void LinearAllocator::Reset()
    { m_head = m_start; }

} // rob
