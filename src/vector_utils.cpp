#include <sstream>

#include <morphio/vector_types.h>
#include <gsl/span>


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
    return str;
}

template <typename T>
const Point centerOfGravity(const T& points)
{
    float x = 0, y = 0, z = 0;
    float size = float(points.size());
    for (const Point& point : points)
    {
        x += point[0];
        y += point[1];
        z += point[2];
    }
    return Point({x / size, y / size, z / size});
}
template const Point centerOfGravity(const gsl::span<const Point>& points);
template const Point centerOfGravity(const std::vector<Point>& points);

} // namespace morphio
