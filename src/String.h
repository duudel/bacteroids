
#ifndef H_ROB_STRING_H
#define H_ROB_STRING_H

#include "Types.h"
#include <cstring>
#include <cstdio>

namespace rob
{

    inline size_t StringLength(const char *str)
    { return std::strlen(str); }

    inline void CopyString(char *dest, const char *src, size_t num)
    {
        size_t i = 0;
        for (; i < num && *src; i++, src++)
            dest[i] = *src;
        for (; i < num; i++)
            dest[i] = 0;
    }

    template <size_t N>
    void CopyString(char (&dest)[N], const char *src)
    { CopyString(dest, src, N); }

    template <class ...Args>
    size_t StringPrintF(char *buf, size_t bufSize, const char * const fmt, Args ...args)
    { return snprintf(buf, bufSize, fmt, args...); }

    template <size_t N, class ...Args>
    size_t StringPrintF(char (&buf)[N], const char * const fmt, Args ...args)
    { return snprintf(buf, N, fmt, args...); }

} // rob

#endif // H_ROB_STRING_H
