#pragma once

#include <array>
#include <iostream>
#include <vector>

namespace minimorph
{
typedef std::array<int, 2> Vector2i;
typedef std::array<double, 3> Vector3d;
typedef std::array<float, 3> Vector3f;
typedef std::array<double, 4> Vector4d;
typedef std::array<float, 4> Vector4f;
typedef std::array<std::array<float, 4>, 4> Matrix4f;

typedef std::array<float, 3> Point;
typedef std::vector<Point> Points;
}

std::array<float, 3> operator*(const std::array<float, 3> &from, float factor);
std::array<float, 3> operator+(const std::array<float, 3> &left,
                               const std::array<float, 3> &right);
std::array<float, 3> operator-(const std::array<float, 3> &left,
                               const std::array<float, 3> &right);
std::array<float, 3> operator+=(std::array<float, 3> &left,
                                const std::array<float, 3> &right);
std::array<float, 3> operator/=(std::array<float, 3> &left, float factor);
