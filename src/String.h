
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




    inline uint32_t DecodeUtf8(const char *&str, const char * const end)
    {
        const uint32_t c = uint8_t(*str++);
        if (c > 0x80 && c < 0xC0) // In the middle of a character
        {
            while (str != end)
            {
                const uint32_t c = uint8_t(*str++);
                if (c <= 0x80 || c >= 0xC0)
                    break;
            }
            return '?';
        }

        if (c < 0x80)
            return c;

        if (str == end) return '?';
        const uint32_t c2 = uint8_t(*str++);
        if (c < 0xE0)
            return (c2 & 0x3F) | ((c & 0x1F) << 6);

        if (str == end) return '?';
        const uint32_t c3 = uint8_t(*str++);
        if (c < 0xF0)
            return (c3 & 0x3F) | ((c2 & 0x3F) << 6) | ((c & 0x1F) << 12);

        if (str == end) return '?';
        const uint32_t c4 = uint8_t(*str++);
        if (c < 0xF8)
            return (c4 & 0x3F) | ((c3 & 0x3F) << 6) | ((c2 & 0x3F) << 12) | ((c & 0x7) << 18);

        if (str == end) return '?';
        const uint32_t c5 = uint8_t(*str++);
        if (c < 0xFC)
            return (c5 & 0x3F) | ((c4 & 0x3F) << 6) | ((c3 & 0x3F) << 12) | ((c2 & 0x3F) << 18) | ((c & 0x3) << 24);

        if (str == end) return '?';
        const uint32_t c6 = uint8_t(*str++);
        return (c6 & 0x3F) | ((c5 & 0x3F) << 6) | ((c4 & 0x3F) << 12) | ((c3 & 0x3F) << 18) | ((c2 & 0x3F) << 24) | ((c & 0x1) << 30);
    }

    inline uint32_t SkipUtf8Right(const char *&str, const char * const end)
    { return DecodeUtf8(str, end); }

    inline bool IsUtf8Trail(uint8_t c) { return (c >> 6) == 0x2; }

    inline uint32_t SkipUtf8Left(const char *&str, const char * const start)
    {
        if (str == start) return 0;
        str--;
        while (IsUtf8Trail(*str)) str--;
        const char *s = str;
        return DecodeUtf8(s, 0);
    }

    struct AsciiTraits
    {
        static uint32_t Decode(const char *&str, const char * const end)
        { return (str != end) ? *str++ : 0; }
        static uint32_t SkipRight(const char *&str, const char * const end)
        { return Decode(str, end); }
        static uint32_t SkipLeft(const char *&str, const char * const start)
        { return (str != start) ? *--str : 0; }
    };

    struct Utf8Traits
    {
        static uint32_t Decode(const char *&str, const char * const end)
        { return DecodeUtf8(str, end); }
        static uint32_t SkipRight(const char *&str, const char * const end)
        { return Decode(str, end); }
        static uint32_t SkipLeft(const char *&str, const char * const start)
        { return SkipUtf8Left(str, start); }
    };

} // rob

#endif // H_ROB_STRING_H
