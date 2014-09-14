
#ifndef H_ROB_RESOURCE_ID_H
#define H_ROB_RESOURCE_ID_H

#include "../Types.h"

namespace rob
{

    template <size_t N, size_t I>
    struct FnvHash
    {
        static uint32_t Hash(const char (&str)[N])
        {
            return (FnvHash<N, I-1>::Hash(str) ^ str[I-1]) * 16777619u;
        }
    };

    template <size_t N>
    struct FnvHash<N, 1>
    {
        static uint32_t Hash(const char (&str)[N])
        {
            return (2166136261u ^ str[0]) * 16777619u;
        }
    };

    inline uint32_t CalculateFnv(const char *str)
    {
        uint32_t hash = 2166136261u;
        while (*str)
        {
            hash ^= *str++;
            hash *= 16777619u;
        }
        hash ^= *str;
        hash *= 16777619u;
        return hash;
    }

    class ResourceID
    {
        uint32_t m_id;
    public:
        struct ConstCharPtr
        {
            ConstCharPtr(const char *s)
                : m_str(s) { }
            const char * const m_str;
        };

        ResourceID()
            : ResourceID("")
        { }

        ResourceID(uint32_t id)
            : m_id(id)
        { }

        template <size_t N>
        ResourceID(const char (&str)[N])
            : m_id(FnvHash<N, N>::Hash(str))
        { }

        ResourceID(const ConstCharPtr &str)
            : m_id(CalculateFnv(str.m_str))
        { }

        uint32_t GetHash() const { return m_id; }

        operator uint32_t() const { return m_id; }
    };

} // rob

#endif // H_ROB_RESOURCE_ID_H

