#pragma once

#include <iostream>
#include <vector>

struct Point
{
    double x, y, z;
};

typedef std::vector<Point> Points;

inline std::ostream& operator<<(std::ostream& s, const Point& p)
{
    s << "point<" << p.x << ", " << p.y << ", " << p.z << '>';
    return s;
}
