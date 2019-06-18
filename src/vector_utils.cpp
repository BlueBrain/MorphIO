#include <sstream>

#include <math.h>
#include <morphio/types.h>

namespace morphio {
Point operator+(const Point& left, const Point& right)
{
    Point ret;
    for (size_t i = 0; i < 3; ++i)
        ret[i] = left[i] + right[i];
    return ret;
}

Point operator-(const Point& left, const Point& right)
{
    Point ret;
    for (size_t i = 0; i < 3; ++i)
        ret[i] = left[i] - right[i];
    return ret;
}

Point operator+=(Point& left, const Point& right)
{
    for (size_t i = 0; i < 3; ++i)
        left[i] += right[i];
    return left;
}

Point operator-=(Point& left, const Point& right)
{
    for (size_t i = 0; i < 3; ++i)
        left[i] -= right[i];
    return left;
}

Point operator/=(Point& left, float factor)
{
    for (size_t i = 0; i < 3; ++i)
        left[i] /= factor;
    return left;
}

std::vector<Point> operator+(const std::vector<Point>& points,
    const Point& right)
{
    std::vector<Point> result;
    for (auto& p : points)
        result.push_back(p + right);
    return result;
}
std::vector<Point> operator-(const std::vector<Point>& points,
    const Point& right)
{
    std::vector<Point> result;
    for (auto& p : points)
        result.push_back(p - right);
    return result;
}

std::vector<Point> operator+=(std::vector<Point>& points, const Point& right)
{
    for (auto& p : points)
        p += right;
    return points;
}

std::vector<Point> operator-=(std::vector<Point>& points, const Point& right)
{
    for (auto& p : points)
        p -= right;
    return points;
}

/**
   Euclidian distance between two points
**/
float distance(const Point& left, const Point& right)
{
    return std::sqrt((left[0] - right[0]) * (left[0] - right[0]) + (left[1] - right[1]) * (left[1] - right[1]) + (left[2] - right[2]) * (left[2] - right[2]));
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
    for (const auto& point : points)
        str += dumpPoint(point) + "\n";
    return str;
}

template <typename T>
const Point centerOfGravity(const T& points)
{
    float x = 0, y = 0, z = 0;
    float size = float(points.size());
    for (const Point& point : points) {
        x += point[0];
        y += point[1];
        z += point[2];
    }
    return Point({x / size, y / size, z / size});
}
template const Point centerOfGravity(const range<const Point>& points);
template const Point centerOfGravity(const std::vector<Point>& points);

template <typename T>
Point operator*(const Point& from, T factor)
{
    Point ret;
    for (size_t i = 0; i < 3; ++i)
        ret[i] = from[i] * static_cast<float>(factor);
    return ret;
}
template Point operator*<int>(const Point& from, int factor);
template Point operator*<float>(const Point& from, float factor);

template <typename T>
Point operator*(T factor, const Point& from)
{
    return from * factor;
}
template Point operator*<int>(int factor, const Point& from);
template Point operator*<float>(float factor, const Point& from);

template <typename T>
Point operator/(const Point& from, T factor)
{
    return from * (1 / static_cast<float>(factor));
}
template Point operator/(const Point& from, int factor);
template Point operator/(const Point& from, float factor);

std::ostream& operator<<(std::ostream& os,
    const std::vector<morphio::Point>& points)
{
    os << morphio::dumpPoints(points);
    return os;
}

std::ostream& operator<<(std::ostream& os, const morphio::Point& point)
{
    os << morphio::dumpPoint(point);
    return os;
}

// Like std::tolower but accepts char
char my_tolower(char ch)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

} // namespace morphio

std::ostream& operator<<(std::ostream& os,
    const std::vector<morphio::Point>& points)
{
    os << morphio::dumpPoints(points);
    return os;
}
std::ostream& operator<<(std::ostream& os, const morphio::Point& point)
{
    os << morphio::dumpPoint(point);
    return os;
}

namespace std {
template <typename T, size_t N>
string to_string(const array<T, N>& a)
{
    string res;
    for (auto el : a)
        res += to_string(el) + ", ";
    return res;
}

template string to_string<float, 3>(const array<float, 3>&);
} // namespace std
