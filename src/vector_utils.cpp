#include "vector_types.h"

std::array<float, 3> operator*(const std::array<float, 3> &from, float factor)
{
    std::array<float, 3> ret;
    for(int i =0; i<3; ++i)
        ret[i] = from[i] * factor;
    return ret;
}

std::array<float, 3> operator+(const std::array<float, 3> &left, const std::array<float, 3> &right)
{
    std::array<float, 3> ret;
    for(int i =0; i<3; ++i)
        ret[i] = left[i] + right[i];
    return ret;
}

std::array<float, 3> operator-(const std::array<float, 3> &left, const std::array<float, 3> &right)
{
    std::array<float, 3> ret;
    for(int i =0; i<3; ++i)
        ret[i] = left[i] - right[i];
    return ret;
}

std::array<float, 3> operator+=(std::array<float, 3> &left, const std::array<float, 3> &right)
{
    for(int i =0; i<3; ++i)
        left[i] += right[i];
    return left;
}

std::array<float, 3> operator/=(std::array<float, 3> &left, float factor)
{
    for(int i =0; i<3; ++i)
        left[i] /= factor;
    return left;
}
