#pragma once

#include <array>
#include <iostream>
#include <vector>

namespace morphio
{
typedef std::array<float, 3> Point;
typedef std::vector<Point> Points;


Point operator+(const Point &left, const Point &right);
Point operator-(const Point &left, const Point &right);
Point operator+=(Point &left, const Point &right);
Point operator/=(Point &left, float factor);

template <typename T> const Point centerOfGravity(const T& points);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const std::vector<Point>& point);


/**
   Euclidian distance between two points
**/
float distance(const Point &left, const Point &right);

template <typename T>
Point operator*(const Point &from, T factor)
{
    Point ret;
    for (int i = 0; i < 3; ++i)
        ret[i] = from[i] * factor;
    return ret;
}

template <typename T>
Point operator*(T factor, const Point &from)
{
    return from * factor;
}

template <typename T>
Point operator/(const Point &from, T factor)
{
    return from * (1/(float)factor);
}


std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const std::vector<morphio::Point>& points);

}
std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const std::vector<morphio::Point>& points);
