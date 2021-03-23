#include <algorithm>  // std::max
#include <cmath>      // std::sqrt
#include <numeric>    // std::accumulate
#include <string>     // std::string
#include <cctype>     // std::tolower

#include <morphio/types.h>

namespace morphio {
Point operator+(const Point& left, const Point& right) {
    Point ret;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] = left[i] + right[i];
    return ret;
}

Point operator-(const Point& left, const Point& right) {
    Point ret;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] = left[i] - right[i];
    return ret;
}

Point operator+=(Point& left, const Point& right) {
    for (size_t i = 0; i < left.size(); ++i)
        left[i] += right[i];
    return left;
}

Point operator-=(Point& left, const Point& right) {
    for (size_t i = 0; i < left.size(); ++i)
        left[i] -= right[i];
    return left;
}

Point operator/=(Point& left, floatType factor) {
    for (size_t i = 0; i < left.size(); ++i)
        left[i] /= factor;
    return left;
}

Points operator+(const Points& points, const Point& right) {
    Points result;
    for (auto& p : points)
        result.push_back(p + right);
    return result;
}
Points operator-(const Points& points, const Point& right) {
    Points result;
    for (auto& p : points)
        result.push_back(p - right);
    return result;
}

Points operator+=(Points& points, const Point& right) {
    for (auto& p : points)
        p += right;
    return points;
}

Points operator-=(Points& points, const Point& right) {
    for (auto& p : points)
        p -= right;
    return points;
}

/**
   Euclidian distance between two points
**/
floatType distance(const Point& left, const Point& right) {
    return std::sqrt((left[0] - right[0]) * (left[0] - right[0]) +
                     (left[1] - right[1]) * (left[1] - right[1]) +
                     (left[2] - right[2]) * (left[2] - right[2]));
}

std::string dumpPoint(const Point& point) {
    std::ostringstream oss;
    oss << point[0] << " " << point[1] << " " << point[2];
    return oss.str();
}

std::string dumpPoints(const Points& points) {
    std::ostringstream oss;
    for (const auto& point : points) {
        oss << dumpPoint(point) << '\n';
    }
    return oss.str();
}

template <typename T>
Point centerOfGravity(const T& points) {
    Point::value_type x = 0, y = 0, z = 0;
    const auto size = static_cast<Point::value_type>(points.size());
    for (const auto& point : points) {
        x += point[0];
        y += point[1];
        z += point[2];
    }
    return Point({x / size, y / size, z / size});
}
template Point centerOfGravity(const range<const Point>& points);
template Point centerOfGravity(const Points& points);

template <typename T>
floatType maxDistanceToCenterOfGravity(const T& points) {
    const auto c = centerOfGravity(points);
    return std::accumulate(std::begin(points),
                           std::end(points),
                           floatType{0},
                           [&](morphio::floatType a, const Point& b) {
                               return std::max(a, distance(c, b));
                           });
}
template floatType maxDistanceToCenterOfGravity(const Points& points);

template <typename T>
Point operator*(const Point& from, T factor) {
    Point ret;
    for (size_t i = 0; i < ret.size(); ++i) {
        ret[i] = from[i] * static_cast<Point::value_type>(factor);
    }
    return ret;
}
template Point operator*<int>(const Point& from, int factor);
template Point operator*<morphio::floatType>(const Point& from, floatType factor);

template <typename T>
Point operator*(T factor, const Point& from) {
    return from * factor;
}
template Point operator*<int>(int factor, const Point& from);
template Point operator*<morphio::floatType>(morphio::floatType factor, const Point& from);

template <typename T>
Point operator/(const Point& from, T factor) {
    return from * (1 / static_cast<morphio::floatType>(factor));
}
template Point operator/(const Point& from, int factor);
template Point operator/(const Point& from, floatType factor);

std::ostream& operator<<(std::ostream& os, const Points& points) {
    return os << morphio::dumpPoints(points);
}

std::ostream& operator<<(std::ostream& os, const morphio::Point& point) {
    return os << morphio::dumpPoint(point);
}

// Like std::tolower but accepts char
char my_tolower(char ch) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Points& points) {
    return os << morphio::dumpPoints(points);
}
std::ostream& operator<<(std::ostream& os, const morphio::Point& point) {
    return os << morphio::dumpPoint(point);
}
