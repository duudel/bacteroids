
#ifndef H_ROB_MATH_FUNCTIONS_H
#define H_ROB_MATH_FUNCTIONS_H

#include <cmath>

namespace rob
{

    template <class T>
    inline T Abs(T a)
    {
        return a < T(0) ? -a : a;
    }

    template <class T>
    inline T Max(T a, T b)
    {
        return a > b ? a : b;
    }

    template <class T>
    inline T Min(T a, T b)
    {
        return a < b ? a : b;
    }

    template <class T>
    inline T Sqrt(T x)
    {
        return std::sqrt(x);
    }

    template <class T>
    inline T Sin(T x)
    {
        return std::sin(x);
    }

    template <class T>
    inline T Cos(T x)
    {
        return std::cos(x);
    }

    template <class T>
    inline void SinCos(T x, T &s, T &c)
    {
        s = std::sin(x);
        c = std::cos(x);
    }

} // de

#endif // H_ROB_MATH_FUNCTIONS_H

