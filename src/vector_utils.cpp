#include <sstream>

#include <morphio/vector_types.h>

namespace morphio
{

Point operator*(const Point &from, float factor)
{
    Point ret;
    for (int i = 0; i < 3; ++i)
        ret[i] = from[i] * factor;
    return ret;
}

Point operator+(const Point &left,
                               const Point &right)
{
    Point ret;
    for (int i = 0; i < 3; ++i)
        ret[i] = left[i] + right[i];
    return ret;
}

Point operator-(const Point &left,
                               const Point &right)
{
    Point ret;
    for (int i = 0; i < 3; ++i)
        ret[i] = left[i] - right[i];
    return ret;
}

Point operator+=(Point &left,
                                const Point &right)
{
    for (int i = 0; i < 3; ++i)
        left[i] += right[i];
    return left;
}

Point operator/=(Point &left, float factor)
{
    for (int i = 0; i < 3; ++i)
        left[i] /= factor;
    return left;
}

std::string dumpPoint(const Point& point)
{
    std::stringstream ss;
    ss << point[0] << " " << point[1] << " " << point[2];
    return ss.str();
}

std::string dumpPoints(const std::vector<Point>& points)
{
    std::string str;
    for(const auto& point: points)
        str += dumpPoint(point) + '\n';
}

} // namespace morphio
