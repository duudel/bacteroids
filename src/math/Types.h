
#ifndef H_ROB_MATH_TYPES_H
#define H_ROB_MATH_TYPES_H

#include "../Types.h"

namespace rob
{

    template <class T, size_t S = 0>
    class Vector4;

    template <class T, size_t S = 0>
    class Matrix4;


    typedef float   float32;
    typedef double  float64;
    typedef float32 real;

    typedef Vector4<float32>    vec4f;
    typedef Vector4<int32_t>    vec4i;

} // rob

#endif // H_ROB_MATH_TYPES_H

